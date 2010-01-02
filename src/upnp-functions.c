#include <libgupnp/gupnp-control-point.h>

#include <upnp.h>
#include <debug.h>
#include <network.h>

#include "bindings.h"
#include "sd_prefs.h"

#define TIMEOUT 		10
#define NOTIFY_INTERVAL 	1
#define PORT_MAPPING_TTL_SEC 	1800

static char **ip = NULL;

char *internal_ip;
int port_to_open;
gboolean callback_executed,success;

gboolean upnp_available=FALSE, upnp_searched=FALSE;

static void
upnp_add_port_mapping_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;
	
	purple_debug_misc(PLUGIN_ID,"calling DeletePortMapping for port %d \n",port_to_open);
	/* prima elimino il vecchio port mapping */
	delete_port_mapping (proxy,
                     "",
                     port_to_open,
                     "TCP",
                     &error);

	if (error!=NULL)
	{
		purple_debug_misc(PLUGIN_ID,"delete_port_mapping: Error: %s\n", error->message);
		g_error_free (error);
	}	

	error=NULL;
	/* poi aggiungo il nuovo */
	purple_debug_misc(PLUGIN_ID,"calling AddPortMapping for port %d to ip %s\n",port_to_open,internal_ip);
	add_port_mapping (proxy,
                  "",
                  port_to_open,
                  "TCP",
                  port_to_open,
                  internal_ip,
                  TRUE,
                  "pidgin-sharedesk-port-mapping",
                  0,
                  &error);

	
	if (error == NULL) {
		purple_debug_misc(PLUGIN_ID,"port mapping %d added for %s\n",port_to_open,internal_ip);
		success=TRUE;
	} else {
		purple_debug_misc(PLUGIN_ID,"add_port_mapping: Error: %s\n", error->message);
		g_error_free (error);
	}
	callback_executed=TRUE;

}

static void
gupnp_ask_external_ip_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;
	purple_debug_misc(PLUGIN_ID,"calling GetExternalIPAddress\n");
	gupnp_service_proxy_send_action (proxy,
	    /* Action name and error location */
	    "GetExternalIPAddress", &error,
	    /* IN args */
	    NULL,
	    /* OUT args */
	    "NewExternalIPAddress",
	    G_TYPE_STRING, ip,
	    NULL);

	if (error == NULL) {
		purple_debug_misc(PLUGIN_ID,"External IP address is %s\n", *ip);
		success=TRUE;
	} else {
		purple_debug_misc(PLUGIN_ID,"Error: %s\n", error->message);
		g_error_free (error);
	}
	callback_executed=TRUE;
}

static GUPnPControlPoint* upnp_get_control_point()
{
	GUPnPContext *context;
	GUPnPControlPoint *cp;

	/* Required initialisation */
	/*	g_thread_init (NULL);*/
	g_type_init ();

	/* Create a new GUPnP Context.  By here we are using the default GLib main
	 context, and connecting to the current machine's default IP on an
	 automatically generated port. */
	context = gupnp_context_new (NULL, NULL, 0, NULL);

	/* Create a Control Point targeting WAN IP Connection services */
	cp=gupnp_control_point_new
		(context, "urn:schemas-upnp-org:service:WANIPConnection:1");
	g_object_unref (context);
	return cp;
}

static void do_loop()
{
	purple_debug_misc(PLUGIN_ID,"do_loop()\n");
	time_t t,t2,t3;
	for (t3=t2=t=time(NULL);callback_executed==FALSE && t2-t<TIMEOUT ; t2=time(NULL))
	{
		if (t2-t3>=NOTIFY_INTERVAL)
		{
			purple_debug_misc(PLUGIN_ID,"[t:%d]\n",(int)(t2-t3));
			t3=time(NULL);
		}
		g_main_context_iteration(NULL,FALSE);	
	}
}

static void init()
{
	purple_debug_misc(PLUGIN_ID,"init()\n");
	callback_executed=FALSE;
	success=FALSE;
}

static gboolean gupnp_get_ip(char** outIp)
{
	purple_debug_misc(PLUGIN_ID,"gupnp_get_ip(...)\n");
	ip=outIp;

	GUPnPControlPoint *cp=upnp_get_control_point();
	init();
	
	/* The service-proxy-available signal is emitted when any services which match
	 our target are found, so connect to it */
	g_signal_connect (cp,
	    "service-proxy-available",
	    G_CALLBACK (gupnp_ask_external_ip_cb),
	    NULL);

	/* Tell the Control Point to start searching */
	gssdp_resource_browser_set_active (GSSDP_RESOURCE_BROWSER (cp), TRUE);

	/* Enter the main loop. This will start the search and result in callbacks to
	 callbacks. */
	do_loop();
	purple_debug_misc(PLUGIN_ID,"gupnp_get_ip(...)->loop finished\n");
	
	/* Clean up */
	g_object_unref (cp);

	return callback_executed && success;

}

static gboolean gupnp_add_port_mapping(int port)
{
	purple_debug_misc(PLUGIN_ID,"gupnp_add_port_mapping(%d)\n",port);
	GUPnPContext *context;
	context = gupnp_context_new (NULL, NULL, 0, NULL);
	internal_ip=g_strdup (gupnp_context_get_host_ip (context));;
	port_to_open=port;

	GUPnPControlPoint *cp=upnp_get_control_point();
	init();
	
	/* The service-proxy-available signal is emitted when any services which match
	 our target are found, so connect to it */
	g_signal_connect (cp,
	    "service-proxy-available",
	    G_CALLBACK (upnp_add_port_mapping_cb),
	    NULL);

	/* Tell the Control Point to start searching */
	gssdp_resource_browser_set_active (GSSDP_RESOURCE_BROWSER (cp), TRUE);

	/* Enter the main loop. This will start the search and result in callbacks to
	 callbacks. */
	do_loop();
	
	/* Clean up */
	g_object_unref (cp);

	return callback_executed && success;

};
static void gupnp_discovery_cb (GUPnPControlPoint *cp, GUPnPServiceProxy *proxy, gpointer userdata)
{
	purple_debug_misc(PLUGIN_ID,"gupnp_discovery_cb(...)\n");
	upnp_searched=TRUE;
	upnp_available=TRUE;
	callback_executed=TRUE;
};

static void gupnp_discover()
{
	GUPnPContext *context;
	context = gupnp_context_new (NULL, NULL, 0, NULL);
	GUPnPControlPoint *cp=upnp_get_control_point();

	g_signal_connect (cp,
	    "service-proxy-available",
	    G_CALLBACK (gupnp_discovery_cb),
	    NULL);

	init();
	
	gssdp_resource_browser_set_active (GSSDP_RESOURCE_BROWSER (cp), TRUE);
	do_loop();
	upnp_searched=TRUE;

	g_object_unref (cp);
	g_object_unref (context);
};

static void purple_upnp_discovery_cb(gboolean success, gpointer data)
{
	upnp_searched=TRUE;
	upnp_available=success;
	callback_executed=TRUE;
};

gboolean upnp_get_ip(char** outIp)
{
	purple_debug_misc(PLUGIN_ID,"upnp_get_ip(...)\n");
	if (purple_prefs_get_bool(PREF_USE_LIBPURPLE_UPNP))
	{
		*outIp=purple_upnp_get_public_ip();
		return purple_network_ip_atoi(*outIp)!=NULL;
	}
	else
		return gupnp_get_ip(outIp);
};

gboolean upnp_add_port_mapping(int port)
{
	purple_debug_misc(PLUGIN_ID,"upnp_add_port_mapping(%d)\n",port);
	if (purple_prefs_get_bool(PREF_USE_LIBPURPLE_UPNP))
	{
		purple_upnp_remove_port_mapping (port, "TCP", NULL, NULL);
		purple_upnp_set_port_mapping (port, "TCP", NULL, NULL);
		return TRUE;
	}
	else
		return gupnp_add_port_mapping(port);
	
};

void upnp_load()
{
	purple_debug_misc(PLUGIN_ID,"upnp_load()\n");

	if (purple_prefs_get_bool(PREF_USE_UPNP))
	{
		if (purple_prefs_get_bool(PREF_USE_LIBPURPLE_UPNP))
			purple_upnp_discover (&purple_upnp_discovery_cb, NULL);
		else
			gupnp_discover();
	}
};
gboolean upnp_is_available()
{
	return upnp_searched && upnp_available;
};


