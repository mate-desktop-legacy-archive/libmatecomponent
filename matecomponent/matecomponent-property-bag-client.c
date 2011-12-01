/*
 * matecomponent-property-bag-client.c: C sugar for property bags.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *   Michael Meeks  (michael@ximian.com)
 *   Nat Friedman   (nat@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <config.h>
#include <string.h>
#include <matecomponent/matecomponent-arg.h>
#include <matecomponent/matecomponent-private.h>
#include <matecomponent/matecomponent-exception.h>
#include <matecomponent/matecomponent-moniker-util.h>
#include <matecomponent/matecomponent-property-bag-client.h>

#define MATECOMPONENT_PBCLIENT_DEFAULT_BAG "config:"

G_LOCK_DEFINE_STATIC (default_bag_lock);

static MateComponent_PropertyBag default_bag = CORBA_OBJECT_NIL;

void
matecomponent_property_bag_shutdown (void)
{
	if (default_bag != CORBA_OBJECT_NIL)
		matecomponent_object_release_unref (default_bag, NULL);
}

static MateComponent_PropertyBag
get_default_bag (CORBA_Environment  *ev)
{
	if (default_bag == CORBA_OBJECT_NIL) {
		G_LOCK (default_bag_lock);

		if (default_bag == CORBA_OBJECT_NIL)
			default_bag = matecomponent_get_object (
				MATECOMPONENT_PBCLIENT_DEFAULT_BAG, 
				"IDL:MateComponent/PropertyBag:1.0", ev);

		G_UNLOCK (default_bag_lock);
	}

	if (default_bag == CORBA_OBJECT_NIL)
		g_warning ("unable to get default property bag\n") ;


	return default_bag;
}

char *
matecomponent_pbclient_get_doc_title (MateComponent_PropertyBag  bag,
			       const char         *key,
			       CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	char *retval;

	matecomponent_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}
	
	retval = MateComponent_PropertyBag_getDocTitle (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get property title: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

char *
matecomponent_pbclient_get_doc (MateComponent_PropertyBag  bag,
			 const char         *key,
			 CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	char *retval;

	matecomponent_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}

	retval = MateComponent_PropertyBag_getDoc (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get property documentation: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

MateComponent_PropertyFlags
matecomponent_pbclient_get_flags (MateComponent_PropertyBag  bag,
			   const char         *key,
			   CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	MateComponent_PropertyFlags retval;

	matecomponent_return_val_if_fail (key != NULL, 0, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return 0;
	}

	retval = MateComponent_PropertyBag_getFlags (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get flags: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return 0;
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

CORBA_TypeCode
matecomponent_pbclient_get_type (MateComponent_PropertyBag  bag,
			  const char         *key,
			  CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_TypeCode retval;

	matecomponent_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}

	retval = MateComponent_PropertyBag_getType (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get type code: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

GList *
matecomponent_pbclient_get_keys (MateComponent_PropertyBag  bag,
			  CORBA_Environment  *opt_ev)
{
	MateComponent_KeyList *key_list;
	CORBA_Environment ev, *my_ev;
	GList *l = NULL;
	int i;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}

	key_list = MateComponent_PropertyBag_getKeys (bag, "", my_ev);
	if (MATECOMPONENT_EX (my_ev) || !key_list) {
		if (!opt_ev)			
			CORBA_exception_free (&ev);
		return NULL;
	}
	
	for (i = 0; i < key_list->_length; i++)
		l = g_list_prepend (l, g_strdup (key_list->_buffer [i]));

	if (!opt_ev)			
		CORBA_exception_free (&ev);

	return l;
}

void
matecomponent_pbclient_free_keys (GList *key_list)
{
	GList *l;

	if (!(l = key_list))
		return;

	while (l) {
		g_free (l->data);
		l = l->next;
	}

	g_list_free (l);
}


#define MAKE_GET_SIMPLE(c_type, default, name, corba_tc, extract_fn)          \
c_type matecomponent_pbclient_get_##name  (MateComponent_PropertyBag  bag,                  \
				    const char         *key,                  \
				    CORBA_Environment  *opt_ev)               \
{                                                                             \
	CORBA_any *value;                                                     \
	c_type retval;                                                        \
                                                                              \
	if (!(value = matecomponent_pbclient_get_value (bag, key, corba_tc, opt_ev)))\
		return default;                                               \
	retval = extract_fn;                                                  \
	CORBA_free (value);                                                   \
	return retval;                                                        \
}

#define MAKE_GET_DEFAULT(c_type, default, name, corba_tc, extract_fn)         \
c_type matecomponent_pbclient_get_default_##name  (MateComponent_PropertyBag  bag,          \
				            const char         *key,          \
				            CORBA_Environment  *opt_ev)       \
{                                                                             \
	CORBA_any *value;                                                     \
	c_type retval;                                                        \
                                                                              \
	if (!(value = matecomponent_pbclient_get_default_value (bag, key, corba_tc,  \
	 					         opt_ev)))            \
		return default;                                               \
	retval = extract_fn;                                                  \
	CORBA_free (value);                                                   \
	return retval;                                                        \
}

#define MAKE_GET_WITH_DEFAULT(c_type, name, assign_fn)                        \
c_type matecomponent_pbclient_get_##name##_with_default (MateComponent_PropertyBag  bag,    \
						  const char         *key,    \
						  c_type              defval, \
						  gboolean           *def)    \
{                                                                             \
	c_type retval;                                                        \
	CORBA_Environment ev;                                                 \
	CORBA_exception_init (&ev);                                           \
        if (def) *def = FALSE;                                                \
	retval = matecomponent_pbclient_get_##name (bag, key, &ev);                  \
	if (MATECOMPONENT_EX (&ev)) {                                                \
		retval = assign_fn (defval);                                  \
                if (def) *def = TRUE;                                         \
        }                                                                     \
	CORBA_exception_free (&ev);                                           \
	return retval;                                                        \
}

/**
 * matecomponent_pbclient_get_string:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a string from the PropertyBag
 *
 * Returns: the value contained in the database, or zero on error.
 */
MAKE_GET_SIMPLE (gchar *, NULL, string, TC_CORBA_string, 
		 g_strdup (*(char **)value->_value))

MAKE_GET_DEFAULT (gchar *, NULL, string, TC_CORBA_string, 
		  g_strdup (*(char **)value->_value))

MAKE_GET_WITH_DEFAULT (gchar *, string, g_strdup)

/**
 * matecomponent_pbclient_get_short:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit integer from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint16, 0, short, TC_CORBA_short, (*(gint16 *)value->_value))

MAKE_GET_DEFAULT (gint16, 0, short, TC_CORBA_short, (*(gint16 *)value->_value))

MAKE_GET_WITH_DEFAULT (gint16, short, )

/**
 * matecomponent_pbclient_get_ushort:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit unsigned integer from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint16, 0, ushort, TC_CORBA_unsigned_short, (*(guint16 *)value->_value))

MAKE_GET_DEFAULT (guint16, 0, ushort, TC_CORBA_unsigned_short, (*(guint16 *)value->_value))

MAKE_GET_WITH_DEFAULT (guint16, ushort, )

/**
 * matecomponent_pbclient_get_long:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit integer from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint32, 0, long, TC_CORBA_long, (*(gint32 *)value->_value))

MAKE_GET_DEFAULT (gint32, 0, long, TC_CORBA_long, (*(gint32 *)value->_value))

MAKE_GET_WITH_DEFAULT (gint32, long, )

/**
 * matecomponent_pbclient_get_ulong:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit unsigned integer from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint32, 0, ulong, TC_CORBA_unsigned_long, (*(guint32 *)value->_value))

MAKE_GET_DEFAULT (guint32, 0, ulong, TC_CORBA_unsigned_long, (*(guint32 *)value->_value))

MAKE_GET_WITH_DEFAULT (guint32, ulong, )

/**
 * matecomponent_pbclient_get_float:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a single precision floating point value from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gfloat, 0.0, float, TC_CORBA_float, (*(gfloat *)value->_value))

MAKE_GET_DEFAULT (gfloat, 0.0, float, TC_CORBA_float, (*(gfloat *)value->_value))

MAKE_GET_WITH_DEFAULT (gfloat, float, )

/**
 * matecomponent_pbclient_get_double:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a double precision floating point value from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gdouble, 0.0, double, TC_CORBA_double, (*(gdouble *)value->_value))

MAKE_GET_DEFAULT (gdouble, 0.0, double, TC_CORBA_double,(*(gdouble *)value->_value))

MAKE_GET_WITH_DEFAULT (gdouble, double, )

/**
 * matecomponent_pbclient_get_char:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 8 bit character value from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gchar, '\0', char, TC_CORBA_char, (*(gchar *)value->_value))

MAKE_GET_DEFAULT (gchar, '\0', char, TC_CORBA_char, (*(gchar *)value->_value))

MAKE_GET_WITH_DEFAULT (gchar, char, )

/**
 * matecomponent_pbclient_get_boolean:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a boolean value from the PropertyBag
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gboolean, FALSE, boolean, TC_CORBA_boolean, 
		 (*(CORBA_boolean *)value->_value))

MAKE_GET_DEFAULT (gboolean, FALSE, boolean, TC_CORBA_boolean, 
		  (*(CORBA_boolean *)value->_value))

MAKE_GET_WITH_DEFAULT (gboolean, boolean, )

/**
 * matecomponent_pbclient_get_value:
 * @bag: a reference to the PropertyBag object
 * @key: key of the value to get
 * @opt_tc: the type of the value, optional
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a value from the PropertyBag
 *
 * Returns: the value contained in the PropertyBag, or zero on error.
 */
CORBA_any *
matecomponent_pbclient_get_value  (MateComponent_PropertyBag  bag,
			    const char         *key,
			    CORBA_TypeCode      opt_tc,
			    CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any *retval, *value;

	matecomponent_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}

	value = MateComponent_PropertyBag_getValue (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get value: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;

	}

	if (opt_tc != CORBA_OBJECT_NIL && value) {

		retval = NULL;

		if (value->_type->kind == CORBA_tk_null)
			CORBA_free (value);

		else if (!CORBA_TypeCode_equivalent (opt_tc, value->_type, my_ev)) {
			/* TODO: we can also try to do automatic conversion */
			matecomponent_exception_set (
				opt_ev, 
				ex_MateComponent_PropertyBag_InvalidType);
			CORBA_free (value);
		} else
			retval = value;
	} else
		retval = value;

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

/**
 * matecomponent_pbclient_get_default_value:
 * @bag: a reference to the PropertyBag object
 * @key: key of the value to get
 * @opt_tc: the type of the value, optional
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get the default value from the PropertyBag
 *
 * Returns: the default value contained in the PropertyBag, or zero on error.
 */
CORBA_any *
matecomponent_pbclient_get_default_value  (MateComponent_PropertyBag  bag,
				    const char         *key,
				    CORBA_TypeCode      opt_tc,
				    CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any *retval;

	matecomponent_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return NULL;
	}

	retval = MateComponent_PropertyBag_getDefault (bag, key, my_ev);

	if (MATECOMPONENT_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get default value: %s\n", 
				   matecomponent_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}


	if (retval && opt_tc != CORBA_OBJECT_NIL) {

		/* fixme: we can also try to do automatic type conversions */

		if (!CORBA_TypeCode_equal (opt_tc, retval->_type, my_ev)) {
			CORBA_free (retval);
			if (!opt_ev)
				CORBA_exception_free (&ev);
			matecomponent_exception_set (opt_ev, 
			        ex_MateComponent_PropertyBag_InvalidType);
			return NULL;
		}

	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

#define MAKE_SET_SIMPLE(c_type, name, corba_tc)                               \
void matecomponent_pbclient_set_##name (MateComponent_PropertyBag  bag,                     \
			         const char         *key,                     \
			         const c_type        value,                   \
			         CORBA_Environment  *opt_ev)                  \
{                                                                             \
	CORBA_any *any;                                                       \
	any = matecomponent_arg_new_from (corba_tc, &value);			      \
	matecomponent_pbclient_set_value (bag, key, any, opt_ev);                    \
	matecomponent_arg_release (any);                                             \
}

/**
 * matecomponent_pbclient_set_short:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit integer value in the PropertyBag.
 */
MAKE_SET_SIMPLE (gint16, short, TC_CORBA_short)
/**
 * matecomponent_pbclient_set_ushort:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit unsigned integer value in the PropertyBag.
 */
MAKE_SET_SIMPLE (guint16, ushort, TC_CORBA_unsigned_short)
/**
 * matecomponent_pbclient_set_long:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit integer value in the PropertyBag.
 */
MAKE_SET_SIMPLE (gint32, long, TC_CORBA_long)
/**
 * matecomponent_pbclient_set_ulong:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit unsigned integer value in the PropertyBag.
 */
MAKE_SET_SIMPLE (guint32, ulong, TC_CORBA_unsigned_long)
/**
 * matecomponent_pbclient_set_float:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a single precision floating point value in the PropertyBag.
 */
MAKE_SET_SIMPLE (gfloat, float, TC_CORBA_float)
/**
 * matecomponent_pbclient_set_double:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a double precision floating point value in the PropertyBag.
 */
MAKE_SET_SIMPLE (gdouble, double, TC_CORBA_double)
/**
 * matecomponent_pbclient_set_char:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 8 bit characte value in the PropertyBag.
 */
MAKE_SET_SIMPLE (gchar, char, TC_CORBA_char)
/**
 * matecomponent_pbclient_set_string:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a string value in the PropertyBag.
 */
void
matecomponent_pbclient_set_string (MateComponent_PropertyBag  bag,
			    const char         *key,
			    const char         *value,
			    CORBA_Environment  *opt_ev)
{
	CORBA_any *any;

	matecomponent_return_if_fail (value != NULL, opt_ev);

	any = matecomponent_arg_new (TC_CORBA_string);

	MATECOMPONENT_ARG_SET_STRING (any, value);

	matecomponent_pbclient_set_value (bag, key, any, opt_ev);

	matecomponent_arg_release (any);
}

/**
 * matecomponent_pbclient_set_boolean:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a boolean value in the PropertyBag.
 */
void
matecomponent_pbclient_set_boolean (MateComponent_PropertyBag  bag,
			     const char         *key,
			     gboolean            value,
			     CORBA_Environment  *opt_ev)
{
	CORBA_any *any;
	CORBA_boolean val = value ? 1 : 0;

	any = matecomponent_arg_new_from (TC_CORBA_boolean, &val);
	matecomponent_pbclient_set_value (bag, key, any, opt_ev);
	matecomponent_arg_release (any);
}

/**
 * matecomponent_pbclient_set_value:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a value in the PropertyBag.
 */
void
matecomponent_pbclient_set_value  (MateComponent_PropertyBag  bag,
			    const char         *key,
			    CORBA_any          *value,
			    CORBA_Environment  *opt_ev)
{
	CORBA_Environment ev, *my_ev;

	matecomponent_return_if_fail (key != NULL, opt_ev);
	matecomponent_return_if_fail (value != NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (bag == CORBA_OBJECT_NIL)
		bag = get_default_bag (my_ev);

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return;
	}

	MateComponent_PropertyBag_setValue (bag, key, value, my_ev);
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}

#define SEND(pb,name,args,corbat,ansip) 				      \
	case CORBA_tk##corbat:						      \
		matecomponent_pbclient_set##corbat (pb, name,                        \
		       (CORBA##corbat) va_arg (args, ansip), ev);             \
		break;

/**
 * matecomponent_pbclient_setv:
 * @bag: the property bag
 * @ev: optional CORBA exception environment or NULL
 * @first_arg: first argument name
 * @var_args: list of subsequent name / type / value triplets
 * 
 * This function provides the grunt implementation for
 * other var-arg functions like matecomponent_widget_set_property
 * 
 * Return value: an error string on error or NULL on success.
 **/
char *
matecomponent_pbclient_setv (MateComponent_PropertyBag       bag,
		      CORBA_Environment       *ev,
		      const char              *first_arg,
		      va_list                  var_args)
{
	const char *arg_name;

	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = va_arg (var_args, CORBA_TypeCode);

		switch (type->kind) {
			SEND (bag, arg_name, var_args, _boolean, int);
			SEND (bag, arg_name, var_args, _long, int);
			SEND (bag, arg_name, var_args, _float, double);
			SEND (bag, arg_name, var_args, _double, double);

		case CORBA_tk_string:
			matecomponent_pbclient_set_string (bag, arg_name, 
						    va_arg (var_args, 
							    CORBA_char *), ev);
			break;

		case CORBA_tk_any:
			matecomponent_pbclient_set_value (bag, arg_name, 
						   va_arg (var_args, 
							   MateComponentArg *), ev);
			break;

		default:
			return g_strdup_printf ("Unhandled setv arg "
				"'%s' type %u", arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);

		if (MATECOMPONENT_EX (ev))
			return matecomponent_exception_get_text (ev);
	}

	return NULL;
}
#undef SEND

#define RECEIVE(pb,name,args,corbat,ansip)			     	     \
	case CORBA_tk##corbat:						     \
		*((CORBA##corbat *)va_arg (args, ansip *)) =		     \
		    matecomponent_pbclient_get##corbat (pb, name, ev);              \
		break;

/**
 * matecomponent_pbclient_getv:
 * @bag: the property bag
 * @ev: optional CORBA exception environment or NULL
 * @first_arg: first argument name
 * @var_args: list of subsequent name / type / value triplets
 * 
 * This function provides the grunt implementation for
 * other var-arg functions like matecomponent_widget_get_property.
 * 
 * Return value: an error string on error or NULL on success.
 **/
char *
matecomponent_pbclient_getv (MateComponent_PropertyBag bag,
		      CORBA_Environment *ev,
		      const char        *first_arg,
		      va_list            var_args)
{
	const char *arg_name;

	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = va_arg (var_args, CORBA_TypeCode);
		
		switch (type->kind) {

			RECEIVE (bag, arg_name, var_args, _boolean, int);
			RECEIVE (bag, arg_name, var_args, _long, int);
			RECEIVE (bag, arg_name, var_args, _float, double);
			RECEIVE (bag, arg_name, var_args, _double, double);

		case CORBA_tk_string:
			*((CORBA_char **)(va_arg (var_args, CORBA_char **))) =
				matecomponent_pbclient_get_string (bag, arg_name, ev);
			break;

		case CORBA_tk_any:
			*((MateComponentArg **)(va_arg (var_args, MateComponentArg **))) =
				matecomponent_pbclient_get_value (bag, arg_name, NULL,
							   ev);
			break;

		default:
			return g_strdup_printf ("Unhandled getv arg "
			        "'%s' type %u", arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);

		if (MATECOMPONENT_EX (ev))
			return matecomponent_exception_get_text (ev);
	}

	return NULL;
}
#undef RECEIVE

void
matecomponent_pbclient_set (MateComponent_PropertyBag   pb,
		     CORBA_Environment   *opt_ev,
		     const char          *first_prop,
		     ...)
{
	char               *err;
	CORBA_Environment  *ev, temp_ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);

	va_start (args, first_prop);

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	if ((err = matecomponent_property_bag_client_setv (pb, ev, first_prop, args))) {
		g_warning ("Error '%s'", err);
		g_free (err);
	}

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	va_end (args);
}

void
matecomponent_pbclient_get (MateComponent_PropertyBag   pb,
		     CORBA_Environment   *opt_ev,
		     const char          *first_prop,
		     ...)
{
	char               *err;
	CORBA_Environment  *ev, temp_ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);

	va_start (args, first_prop);

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	if ((err = matecomponent_property_bag_client_getv (pb, ev, first_prop, args))) {
		g_warning ("Error '%s'", err);
		g_free (err);
	}

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	va_end (args);
}

static MateCORBA_IMethod *
get_set_value_imethod (void)
{
	static MateCORBA_IMethod *imethod = NULL;

	if (!imethod) {
		guint i;
		MateCORBA_IMethods *methods;

		methods = &MateComponent_PropertyBag__iinterface.methods;

		for (i = 0; i < methods->_length; i++) {
			if (!strcmp (methods->_buffer [i].name,
				     "setValue"))
				imethod = &methods->_buffer [i];
		}
		g_assert (imethod != NULL);
	}

	return imethod;
}

/**
 * matecomponent_pbclient_set_value_async:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a value on the PropertyBag asynchronously, discarding any
 * possible roundtrip exceptions.
 */
void
matecomponent_pbclient_set_value_async (MateComponent_PropertyBag  bag,
				 const char         *key,
				 CORBA_any          *value,
				 CORBA_Environment  *opt_ev)
{
	gpointer args [2];
	CORBA_Environment ev, *my_ev;

	g_return_if_fail (key != NULL);
	g_return_if_fail (value != NULL);
	g_return_if_fail (bag != CORBA_OBJECT_NIL);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (MATECOMPONENT_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return;
	}

	args [0] = (gpointer) &key;
	args [1] = (gpointer) value;

	MateCORBA_small_invoke_async
		(bag, get_set_value_imethod (),
		 NULL, NULL, args, NULL, my_ev);
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}
