#include <libgupnp/gupnp-control-point.h>
#include "bindings.h"

/*static GMainLoop *main_loop;*/
char **ip = NULL;

char *internal_ip;
int port_to_open;
gboolean callback_executed;

static void
upnp_add_port_mapping_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;
	
	g_printerr ("calling DeletePortMapping for port %d ",port_to_open);
	/* prima elimino il vecchio port mapping */
	gupnp_service_proxy_send_action (proxy,
	    /* Action name and error location */
	    "DeletePortMapping", &error,
	    /* IN args */
	    "NewRemoteHost",
	    G_TYPE_STRING, "",

	    "NewExternalPort",
	    G_TYPE_UINT, port_to_open,

	    "NewProtocol",
	    G_TYPE_STRING, "TCP",
	    
	    NULL,
	    /* OUT args */
	    NULL);
	if (error!=NULL)
	{
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}	

	error=NULL;
	/* poi aggiungo il nuovo */
	g_printerr ("calling AddPortMapping for port %d to ip %s",port_to_open,internal_ip);
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
	/*gupnp_service_proxy_send_action (proxy,
	    "AddPortMapping", &error,
	
	    "NewRemoteHost",
	    G_TYPE_STRING, "",

	    "NewExternalPort",
	    G_TYPE_UINT, port_to_open,

	    "NewProtocol",
	    G_TYPE_STRING, "TCP",

	    "NewInternalPort",
	    G_TYPE_UINT, port_to_open,

	    "NewInternalClient",
	    G_TYPE_STRING, internal_ip,

	    "NewEnabled",
	    G_TYPE_BOOLEAN, TRUE,

	    "NewPortMappingDescription",
	    G_TYPE_STRING, "pidgin-sharedesk-port-mapping",

	    "NewLeaseDuration",
	    G_TYPE_UINT64, 0,

	    
	    NULL,
	    
	    NULL);*/

	if (error == NULL) {
		g_printerr ("port mapping %d added for %s",port_to_open,internal_ip);
	} else {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}

}

static void
upnp_ask_external_ip_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;

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
		g_print ("External IP address is %s\n", ip);
		//		g_free (ip);
	} else {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}
	/*	g_main_loop_quit (main_loop);*/

}

GUPnPControlPoint* upnp_get_control_point()
{
	GUPnPContext *context;
	GUPnPControlPoint *cp;

	/* Required initialisation */
	//	g_thread_init (NULL);
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

void
upnp_get_ip(char** outIp)
{
	ip=outIp;
	GUPnPControlPoint *cp=upnp_get_control_point();

	/* The service-proxy-available signal is emitted when any services which match
	 our target are found, so connect to it */
	g_signal_connect (cp,
	    "service-proxy-available",
	    G_CALLBACK (upnp_ask_external_ip_cb),
	    NULL);

	/* Tell the Control Point to start searching */
	gssdp_resource_browser_set_active (GSSDP_RESOURCE_BROWSER (cp), TRUE);

	/* Enter the main loop. This will start the search and result in callbacks to
	 callbacks. */
	int i=0;
	for (;*outIp==NULL||i<10;i++)
		g_main_context_iteration(NULL,FALSE);	
	
	/* Clean up */
	g_object_unref (cp);

	return 0;
}

gboolean upnp_add_port_mapping(int port)
{
	GUPnPControlPoint *cp=upnp_get_control_point();
	callback_executed=FALSE;

	GUPnPContext *context;
	context = gupnp_context_new (NULL, NULL, 0, NULL);
	internal_ip=g_strdup (gupnp_context_get_host_ip (context));;
	port_to_open=port;

	
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
	int i=0;
	for (;callback_executed==FALSE||i<10;i++)
		g_main_context_iteration(NULL,FALSE);	
	
	/* Clean up */
	g_object_unref (cp);

	return callback_executed;
};

