/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <matecorba/matecorba.h>

#include <matecomponent-activation/matecomponent-activation.h>

#include "empty.h"

Empty empty_client = CORBA_OBJECT_NIL;

static void do_Nothing (PortableServer_Servant servant,
			CORBA_Environment * ev);

PortableServer_ServantBase__epv base_epv = {
	NULL,
	NULL,
	NULL
};
POA_Empty__epv empty_epv = { NULL, do_Nothing };
POA_Empty__vepv poa_empty_vepv = { &base_epv, &empty_epv };
POA_Empty poa_empty_servant = { NULL, &poa_empty_vepv };

static void
do_exit (int arg)
{
	exit (2);
}

int
main (int argc, char *argv[])
{
	PortableServer_ObjectId *objid;
	PortableServer_POA poa;
	GOptionContext *ctx;
	GOptionGroup *goption_group;
	GError *error = NULL;
        GMainLoop *loop;

	CORBA_Environment ev;
	CORBA_ORB orb;
	GSList *reg_env = NULL;

	signal (SIGINT, do_exit);
	signal (SIGTERM, do_exit);

        g_thread_init (NULL);

	CORBA_exception_init (&ev);
	orb = matecomponent_activation_init (argc, argv);

	g_set_prgname ("matecomponent-activation-empty-server");
	ctx = g_option_context_new (NULL);
	goption_group = matecomponent_activation_get_goption_group ();
	g_option_context_set_main_group (ctx, goption_group);
	if (!g_option_context_parse (ctx, &argc, &argv, &error)) {
		g_printerr ("%s\n", error->message);
		g_error_free (error);
		exit (1);
	}
	g_option_context_free (ctx);

	POA_Empty__init (&poa_empty_servant, &ev);

	poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
	objid = PortableServer_POA_activate_object (poa, &poa_empty_servant, &ev);

	empty_client = PortableServer_POA_servant_to_reference (poa,
								&poa_empty_servant,
								&ev);
	if (!empty_client) {
		printf ("Cannot get objref\n");
		return 1;
	}

#ifndef G_OS_WIN32
	reg_env = matecomponent_activation_registration_env_set (
			reg_env, "DISPLAY", getenv ("DISPLAY"));
	reg_env = matecomponent_activation_registration_env_set (
			reg_env, "SESSION_MANAGER", getenv ("SESSION_MANAGER"));
	reg_env = matecomponent_activation_registration_env_set (
			reg_env, "AUDIODEV", getenv ("AUDIODEV"));
#endif
	reg_env = matecomponent_activation_registration_env_set (
			reg_env, "LANG", getenv ("LANG"));

        /*
         * NB. It is imperative to register the server that is being
         * requested last - or we can still race in the activation daemon.
         */
	matecomponent_activation_register_active_server ("OAFIID:Empty2:19991025", empty_client, reg_env);
        g_usleep (500000); /* 1/2 sec */
	matecomponent_activation_register_active_server ("OAFIID:Empty:19991025", empty_client, reg_env);

	matecomponent_activation_registration_env_free (reg_env);

	PortableServer_POAManager_activate
		(PortableServer_POA__get_the_POAManager (poa, &ev), &ev);

        /* run the CORBA main loop for a couple of seconds then quit */
        loop = g_main_loop_new (NULL, FALSE);
        g_timeout_add (1000, (GSourceFunc) g_main_loop_quit, loop);
        g_main_loop_run (loop);

	matecomponent_activation_active_server_unregister ("OAFIID:Empty:19991025", empty_client);
	matecomponent_activation_active_server_unregister ("OAFIID:Empty2:19991025", empty_client);

	PortableServer_POA_deactivate_object (poa, objid, &ev);

	return 0;
}

static void
do_Nothing (PortableServer_Servant servant, CORBA_Environment *ev)
{
	g_print ("doNothing called!");
}
