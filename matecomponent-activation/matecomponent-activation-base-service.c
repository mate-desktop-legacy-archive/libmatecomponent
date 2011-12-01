/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  matecomponent-activation: A library for accessing matecomponent-activation-server.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000, 2001 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

/* This is part of the per-app CORBA bootstrapping - we use this to get 
   hold of a running metaserver and such */

#include <config.h>

#include <glib/gi18n-lib.h>

#include <gio/gio.h>

#include <matecomponent-activation/matecomponent-activation-init.h>
#include <matecomponent-activation/matecomponent-activation-base-service.h>
#include <matecomponent-activation/matecomponent-activation-private.h>
#include <matecomponent-activation/MateComponent_ActivationContext.h>
#include <matecomponent-activation/matecomponent-activation-client.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <stdlib.h>
#include <fcntl.h>

#ifdef G_OS_WIN32
#include <share.h>
#endif

#include <glib/gstdio.h>

/* If you have a strange unix, you get odd hard coded limits */
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

static GSList *registries = NULL;

typedef struct {
	int priority;
	const MateComponentActivationBaseServiceRegistry *registry;
	gpointer user_data;
} RegistryInfo;

typedef struct {
	int priority;
	MateComponentActivationBaseServiceActivator activator;
} ActivatorInfo;

static gint
ri_compare (gconstpointer a, gconstpointer b)
{
        RegistryInfo *ra = (RegistryInfo *) a;
        RegistryInfo *rb = (RegistryInfo *) b;

	return (rb->priority - ra->priority);
}

void
matecomponent_activation_base_service_registry_add
                      (const MateComponentActivationBaseServiceRegistry *registry,
                       int                                        priority,
                       gpointer                                   user_data)
{
	RegistryInfo *new_ri;

	g_return_if_fail (registry);

	new_ri = g_new (RegistryInfo, 1);
	new_ri->priority = priority;
	new_ri->registry = registry;
	new_ri->user_data = user_data;

	registries = g_slist_insert_sorted (registries, new_ri, ri_compare);
}

CORBA_Object
matecomponent_activation_base_service_check (const MateComponentActivationBaseService *base_service,
                                      CORBA_Environment                 *ev)
{
	GSList *link;
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int dist = INT_MAX;
	char *ior = NULL;

	for (link = registries; link; link = link->next) {
		RegistryInfo *ri;
		char *new_ior;
		int new_dist = dist;

		ri = link->data;

		if (!ri->registry->check)
			continue;

		new_ior = ri->registry->check (ri->registry, base_service, 
                                             &new_dist, ri->user_data);
		if (new_ior && (new_dist < dist)) {
			g_free (ior);
			ior = new_ior;
		} else if (new_ior) {
			g_free (new_ior);
		}
	}

	if (ior) {
		retval = CORBA_ORB_string_to_object (matecomponent_activation_orb_get (), ior, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			retval = CORBA_OBJECT_NIL;

		g_free (ior);
	}

	return retval;
}

/*dumb marshalling hack */
static void
matecomponent_activation_registration_iterate (const MateComponentActivationBaseService *base_service,
			  CORBA_Object obj, CORBA_Environment *ev,
			  gulong offset, int nargs)
{
	GSList *link;
	char *ior = NULL;

	if (nargs == 4)
		ior = CORBA_ORB_object_to_string (matecomponent_activation_orb_get (), obj, ev);

	for (link = registries; link; link = link->next) {
		RegistryInfo *ri;
		void (*func_ptr) ();

		ri = link->data;

		func_ptr = *(gpointer *) ((guchar *) ri->registry + offset);

		if (!func_ptr)
			continue;

		switch (nargs) {
		case 4:
			func_ptr (ri->registry, ior, base_service, ri->user_data);
			break;
		case 2:
			func_ptr (ri->registry, ri->user_data);
			break;
		}
	}

	if (nargs == 4)
		CORBA_free (ior);
}

static int lock_count = 0;

static void
matecomponent_activation_registries_lock (CORBA_Environment *ev)
{
	if (lock_count == 0)
		matecomponent_activation_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (MateComponentActivationBaseServiceRegistry, lock), 2);
	lock_count++;
}

static void
matecomponent_activation_registries_unlock (CORBA_Environment *ev)
{
	lock_count--;
	if (lock_count == 0)
		matecomponent_activation_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (MateComponentActivationBaseServiceRegistry, unlock),
					  2);
}

void
matecomponent_activation_base_service_unset (const MateComponentActivationBaseService *base_service,
			CORBA_Object obj, CORBA_Environment *ev)
{
	matecomponent_activation_registries_lock (ev);
	matecomponent_activation_registration_iterate (base_service, obj, ev,
				  G_STRUCT_OFFSET (MateComponentActivationBaseServiceRegistry,
						   unregister), 4);
	matecomponent_activation_registries_unlock (ev);
}

void
matecomponent_activation_base_service_set (const MateComponentActivationBaseService *base_service,
		      CORBA_Object obj, CORBA_Environment *ev)
{
	matecomponent_activation_registries_lock (ev);
	matecomponent_activation_registration_iterate (base_service, obj, ev,
				  G_STRUCT_OFFSET (MateComponentActivationBaseServiceRegistry,
						   register_new), 4);
	matecomponent_activation_registries_unlock (ev);
}

struct SysServerInstance
{
	CORBA_Object already_running;
	char *username, *hostname;
};

struct SysServer
{
	const char *name;
	const char **cmd;
	int fd_arg;
	GSList *instances;
}
activatable_servers[] =
{
                                /* cmd filled in at runtime */
	{"IDL:MateComponent/ActivationContext:1.0", NULL, 2, CORBA_OBJECT_NIL}, 
	{ NULL}
};

#define STRMATCH(x, y) ((!x && !y) || (x && y && !strcmp(x, y)))
static CORBA_Object
existing_check (const MateComponentActivationBaseService *base_service, struct SysServer *ss)
{
	GSList *link;

	for (link = ss->instances; link; link = link->next) {
		struct SysServerInstance *ssi;

		ssi = link->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, base_service->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, base_service->hostname)))
                        return ssi->already_running;
	}

	return CORBA_OBJECT_NIL;
}

void
matecomponent_activation_base_service_debug_shutdown (CORBA_Environment *ev)
{
        int     i;
        GSList *l, *instances;
        struct SysServerInstance *ssi;

	for (i = 0; activatable_servers[i].name; i++) {

                instances = activatable_servers[i].instances;
                activatable_servers[i].instances = NULL;

                for (l = instances; l; l = l->next) {
                        ssi = l->data;

                        CORBA_Object_release (ssi->already_running, ev);
                        g_free (ssi->username);
                        g_free (ssi->hostname);
                        g_free (ssi);
                }
                g_slist_free (instances);
        }
}

static void
matecomponent_activation_existing_set (const MateComponentActivationBaseService *base_service,
                                struct SysServer *ss,
                                CORBA_Object obj, CORBA_Environment *ev)
{
	GSList *link;
	struct SysServerInstance *ssi;

        ssi = NULL;

	for (link = ss->instances; link; link = link->next) {
		ssi = link->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, base_service->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, base_service->hostname))) break;
	}

	if (link == NULL) {
		ssi = g_new0 (struct SysServerInstance, 1);
		ssi->already_running = obj;
		ssi->username = g_strdup (base_service->username);
		ssi->hostname = g_strdup (base_service->hostname);
                ss->instances = g_slist_prepend (ss->instances, ssi);
	} else {
		CORBA_Object_release (ssi->already_running, ev);
		ssi->already_running = obj;
	}

        /* FIXME: all this code is unneccesarily abstract & buggy with it */
        if (!strcmp (base_service->name, "IDL:MateComponent/ActivationContext:1.0")) {
                matecomponent_activation_register_client (obj, ev);
        }
}

static GSList *activator_list = NULL;

static gint
ai_compare (gconstpointer a, gconstpointer b)
{
	const ActivatorInfo *ra, *rb;

	ra = a;
	rb = b;

	return (rb->priority - ra->priority);
}

void
matecomponent_activation_base_service_activator_add (MateComponentActivationBaseServiceActivator activator, 
                                              int priority)
{
	ActivatorInfo *new_act;

	new_act = g_new (ActivatorInfo, 1);
	new_act->priority = priority;
	new_act->activator = activator;
	activator_list =
		g_slist_insert_sorted (activator_list, new_act, ai_compare);
}

static CORBA_Object
matecomponent_activation_activators_use (const MateComponentActivationBaseService *base_service, const char **cmd,
                                  int fd_arg, CORBA_Environment *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	GSList *link;

	for (link = activator_list; CORBA_Object_is_nil (retval, ev) && link;
	     link = link->next) {
		ActivatorInfo *actinfo;
		actinfo = link->data;

		retval = actinfo->activator (base_service, cmd, fd_arg, ev);
	}

	return retval;
}

CORBA_Object
matecomponent_activation_internal_service_get_extended (
        const MateComponentActivationBaseService *base_service,
        gboolean              existing_only,
        CORBA_Environment    *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int i;
	CORBA_Environment myev, important_error_ev;
	gboolean ne;

	g_return_val_if_fail (base_service, CORBA_OBJECT_NIL);

        MATECOMPONENT_ACTIVATION_LOCK ();

	for (i = 0; activatable_servers[i].name; i++) {
		if (!strcmp (base_service->name, activatable_servers[i].name))
			break;
	}

	if (!activatable_servers[i].name) {
                MATECOMPONENT_ACTIVATION_UNLOCK ();
		return retval;
        }

	CORBA_exception_init (&myev);
        CORBA_exception_init (&important_error_ev);
        
	retval = existing_check (base_service, &activatable_servers[i]);
	if (!CORBA_Object_non_existent (retval, ev))
		goto out;

	matecomponent_activation_registries_lock (ev);

	retval = matecomponent_activation_base_service_check (base_service, &myev);
	ne = CORBA_Object_non_existent (retval, &myev);
	if (ne && !existing_only) {
		CORBA_Object race_condition;

		CORBA_Object_release (retval, &myev);
                
		retval =
			matecomponent_activation_activators_use (base_service,
					    activatable_servers[i].cmd,
					    activatable_servers[i].fd_arg,
					    &important_error_ev);

		race_condition = matecomponent_activation_base_service_check (base_service, &myev);

		if (!CORBA_Object_non_existent (race_condition, &myev)) {
			CORBA_Object_release (retval, &myev);
			retval = race_condition;
		} else if (!CORBA_Object_is_nil (retval, &myev)) {
			matecomponent_activation_base_service_set (base_service, retval, &myev);
                        CORBA_Object_release (race_condition, &myev);
                }
	}

	matecomponent_activation_registries_unlock (ev);

	if (!CORBA_Object_non_existent (retval, ev))
		matecomponent_activation_existing_set (base_service, &activatable_servers[i], retval, ev);

      out:
        /* If we overwrote ev with some stupid junk, replace
         * it with the real error
         */
        if (important_error_ev._major != CORBA_NO_EXCEPTION) {
                CORBA_exception_free (ev);
                /* This transfers memory ownership */
                *ev = important_error_ev;
        }
        
        CORBA_exception_free (&myev);

        MATECOMPONENT_ACTIVATION_UNLOCK ();

	return retval;
}

CORBA_Object
matecomponent_activation_service_get (const MateComponentActivationBaseService *base_service)
{
        CORBA_Environment ev;
        CORBA_Object obj;
        
        CORBA_exception_init (&ev);
        
        obj = matecomponent_activation_internal_service_get_extended (
                base_service, FALSE, &ev);

#ifdef G_ENABLE_DEBUG
        if (ev._major != CORBA_NO_EXCEPTION) {
                if (!strcmp (ev._id, "IDL:MateComponent/GeneralError:1.0")) {
                        MateComponent_GeneralError *err = ev._any._value;
		
                        if (!err || !err->description)
                                g_warning ("General error with no description");
                        else
                                g_warning ("Exception '%s' on activate", err->description);
                } else
                        g_warning ("Exception on activate\n");
        }
#endif

        CORBA_exception_free (&ev);

        return obj;
}

/*****Implementation of the IOR registration system via plain files ******/
static int lock_fd = -1;

static const char *
get_tmpdir (void)
{
        static const char *tmpdir = NULL;

        if (!tmpdir)
                tmpdir = MateCORBA_get_safe_tmp ();

        if (!tmpdir)
                g_error ("No secure tmpdir found");

        return tmpdir;
}

static const char *
get_session_guid (void)
{
#ifdef HAVE_DBUS
        const char *session_bus_address;
        const char *guid;

        session_bus_address = g_dbus_address_get_for_bus_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if (session_bus_address == NULL)
                return NULL;

        guid = g_strrstr (session_bus_address, "guid=");

        if (guid == NULL)
                return NULL;

        if (guid[0] == '\0')
                return NULL;

        return guid + strlen ("guid=");
#else
        return NULL;
#endif
}

char *
_matecomponent_activation_lock_fname_get (void)
{
       const char *session_guid;

       session_guid = get_session_guid ();

       if (session_guid == NULL)
               return g_build_filename (get_tmpdir (),
                                        "matecomponent-activation-register.lock",
                                        NULL);

       return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "matecomponent-activation-register-%s.lock",
                               get_tmpdir (), session_guid);
}

char *
_matecomponent_activation_ior_fname_get (void)
{
       const char *session_guid;

       session_guid = get_session_guid ();

       if (session_guid == NULL)
               return g_build_filename (get_tmpdir (),
                                        "matecomponent-activation-server-ior",
                                        NULL);

       return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "matecomponent-activation-server-%s-ior",
                               get_tmpdir (), session_guid);
}

static void
wait_for_lock (void)
{
#ifdef HAVE_USLEEP
        usleep (10000);

#elif defined(HAVE_NANOSLEEP)
        struct timespec timewait;
        timewait.tv_sec = 0;
        timewait.tv_nsec = 1000000;
        nanosleep (&timewait, NULL);

#else
#warning You might have bad performance without usleep
        g_usleep (10000);
#endif
        access ("matecomponent-activation lock wait", 0);
}

static void
rloc_file_lock (const MateComponentActivationBaseServiceRegistry *registry, 
                gpointer                                   user_data)
{
#if defined (F_SETFD) && defined (FD_CLOEXEC) && defined (F_SETLKW)
	char *fn;
	struct flock lock;
        int retval;
        char *err;

        fn = _matecomponent_activation_lock_fname_get ();

	while ((lock_fd = open (fn, O_CREAT | O_RDWR, 0700)) < 0) {

                if (errno == EEXIST)
                        wait_for_lock ();

		else {
                        g_message ("%s locking '%s'", g_strerror (errno), fn);
			break;
                }
	}

	fcntl (lock_fd, F_SETFD, FD_CLOEXEC);

	if (lock_fd >= 0) {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		lock.l_pid = getpid ();

		while ((retval = fcntl (lock_fd, F_SETLKW, &lock)) < 0
		       && errno == EINTR) /**/;

                if (retval < 0) {
                        /* FIXME: need to report this error in a better way. */
                        err = strerror (errno);
                        g_warning ("Failed to acquire lock: %s\n.", err);
                }
	}

        g_free (fn);
#elif defined (G_OS_WIN32)
	char *fn = _matecomponent_activation_lock_fname_get ();
        wchar_t *wfn = g_utf8_to_utf16 (fn, -1, NULL, NULL, NULL);
        
	while ((lock_fd = _wsopen (wfn, O_CREAT|O_RDWR|_O_SHORT_LIVED|_O_NOINHERIT, _SH_DENYRW, 0700)) < 0) {
                if (errno == EACCES) {
                        wait_for_lock ();
		} else {
                        g_message ("%s locking '%s'", g_strerror (errno), fn);
			break;
                }
	}
        g_free (wfn);
        g_free (fn);
#else
        g_warning ("No locking implemented\n");
#endif
}

static void
rloc_file_unlock (const MateComponentActivationBaseServiceRegistry *registry, 
                  gpointer                                   user_data)
{
#if defined (F_SETFD) && defined (FD_CLOEXEC) && defined (F_SETLKW)
        struct flock lock;

	if (lock_fd >= 0) {

		lock.l_type = F_UNLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		lock.l_pid = getpid ();

		fcntl (lock_fd, F_SETLKW, &lock);
		close (lock_fd);
		lock_fd = -1;
	}
#elif defined (G_OS_WIN32)
	if (lock_fd >= 0) {
		close (lock_fd);
		lock_fd = -1;
	}
#endif
}

static char *
rloc_file_check (const MateComponentActivationBaseServiceRegistry *registry,
		 const MateComponentActivationBaseService *base_service, int *ret_distance,
		 gpointer user_data)
{
	FILE *fh;
	char *fn;

        fn = _matecomponent_activation_ior_fname_get ();
	fh = g_fopen (fn, "r");
        g_free (fn);

	if (fh != NULL) {
		char iorbuf[8192];

		iorbuf[0] = '\0';
		while (fgets (iorbuf, sizeof (iorbuf), fh)
		       && strncmp (iorbuf, "IOR:", 4))
			/**/;
		g_strstrip (iorbuf);

		fclose (fh);

		if (!strncmp (iorbuf, "IOR:", 4)) {
			*ret_distance = 0;
			return g_strdup (iorbuf);
		}
	}

	return NULL;
}

static void
rloc_file_register (const MateComponentActivationBaseServiceRegistry *registry, const char *ior,
		    const MateComponentActivationBaseService *base_service,
		    gpointer user_data)
{
	char *fn;
	FILE *fh;

        fn = _matecomponent_activation_ior_fname_get ();
	fh = g_fopen (fn, "w");

        if (fh != NULL) {
                fprintf (fh, "%s\n", ior);
                fclose (fh);
        }       

        g_free (fn);
}

static void
rloc_file_unregister (const MateComponentActivationBaseServiceRegistry *registry, 
                      const char                                *ior,
		      const MateComponentActivationBaseService         *base_service,
		      gpointer                                   user_data)
{
	char *fn;

	g_unlink ((fn = _matecomponent_activation_ior_fname_get ()));
        g_free (fn);
}

static const MateComponentActivationBaseServiceRegistry rloc_file = {
	rloc_file_lock,
	rloc_file_unlock,
	rloc_file_check,
	rloc_file_register,
	rloc_file_unregister
};

static CORBA_Object
local_re_check_fn (const MateComponent_ActivationEnvironment *environment,
                   const char                         *act_iid,
                   gpointer                            user_data)
{
        CORBA_Environment ev;
        CORBA_Object result;

        CORBA_exception_init (&ev);
        result = matecomponent_activation_internal_service_get_extended
                (user_data, TRUE, &ev);
        CORBA_exception_free (&ev);

        return result;
}

static CORBA_Object
local_activator (const MateComponentActivationBaseService *base_service,
                 const char **cmd,
		 int fd_arg, 
                 CORBA_Environment *ev)
{
	if (
	    (!base_service->username
	     || STRMATCH (base_service->username, g_get_user_name ()))
	    && (!base_service->hostname
		|| STRMATCH (base_service->hostname, matecomponent_activation_hostname_get ()))) {

		return matecomponent_activation_server_by_forking (
                        cmd, FALSE, fd_arg, NULL, NULL, base_service->name,
                        TRUE, local_re_check_fn, (gpointer)base_service, ev);
	}

	return CORBA_OBJECT_NIL;
}

void
matecomponent_activation_base_service_init (void)
{
        const char *override_cmd;
#ifndef G_OS_WIN32
        static const char *matecomponent_activation_ac_cmd[] =
                { SERVER_LIBEXECDIR "/matecomponent-activation-server",
                  "--ac-activate", "--ior-output-fd=%d", NULL };
#else
        static const char *matecomponent_activation_ac_cmd[] =
                { NULL,
                  "--ac-activate", "--ior-output-fd=%d", NULL };
        static gboolean beenhere = FALSE;

        if (!beenhere) {
                matecomponent_activation_ac_cmd[0] =
                        g_build_filename (_matecomponent_activation_win32_get_server_libexecdir (),
                                          "matecomponent-activation-server",
                                          NULL);
                beenhere = TRUE;
        }
#endif

	matecomponent_activation_base_service_activator_add (local_activator, 0);

	matecomponent_activation_base_service_registry_add (&rloc_file, 0, NULL);

        if ((override_cmd = g_getenv ("MATECOMPONENT_ACTIVATION_SERVER")) &&
            (override_cmd[0] != '\0'))
                matecomponent_activation_ac_cmd [0] = override_cmd;

        activatable_servers[0].cmd = matecomponent_activation_ac_cmd;
}
