#include <matecomponent-activation/matecomponent-activation.h>

#include "plugin.h"

static gpointer matecomponent_activation_plugin_impl_ptr = NULL;

static void
plugin_test_impl (PortableServer_Servant  servant,
		  CORBA_Environment      *ev)
{
	matecomponent_activation_plugin_unuse (matecomponent_activation_plugin_impl_ptr);
}

static PortableServer_ServantBase__epv base_epv = {
	NULL,
	NULL,
	NULL
};

static POA_Plugin__epv plugin_epv = {
	NULL,
	plugin_test_impl
};

static POA_Plugin__vepv plugin_vepv = {
	&base_epv,
	&plugin_epv
};

static POA_Plugin plugin_servant = {
	NULL,
	&plugin_vepv
};

static CORBA_Object
activate_plugin (PortableServer_POA  poa,
		 const char         *iid,
		 gpointer            impl_ptr,
		 CORBA_Environment  *ev)
{
	CORBA_Object objref;

	POA_Plugin__init (&plugin_servant, ev);

	matecomponent_activation_plugin_use (&plugin_servant, impl_ptr);

	matecomponent_activation_plugin_impl_ptr = impl_ptr;

	objref = PortableServer_POA_servant_to_reference (poa,
							  &plugin_servant,
							  ev);

	return objref;
}

static MateComponentActivationPluginObject plugin_list[] = {
	{"OAFIID:Plugin:20010713", activate_plugin},
	{NULL}
};

const  MateComponentActivationPlugin MateComponent_Plugin_info = {
	plugin_list,
	"MateComponent Activation Test Plugin"
};
