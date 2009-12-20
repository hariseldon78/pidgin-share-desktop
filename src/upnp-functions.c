#include <libgupnp/gupnp-control-point.h>
#include "bindings.h"

#define TIMEOUT 		100000
#define NOTIFY_INTERVAL 	1
#define PORT_MAPPING_TTL_SEC 	1800

char **ip = NULL;

char *internal_ip;
int port_to_open;
gboolean callback_executed,success;

static void
upnp_add_port_mapping_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;
	
	g_printerr ("calling DeletePortMapping for port %d \n",port_to_open);
	/* prima elimino il vecchio port mapping */
	delete_port_mapping (proxy,
                     "",
                     port_to_open,
                     "TCP",
                     error);

	if (error!=NULL)
	{
		g_printerr ("delete_port_mapping: Error: %s\n", error->message);
		g_error_free (error);
	}	

	error=NULL;
	/* poi aggiungo il nuovo */
	g_printerr ("calling AddPortMapping for port %d to ip %s\n",port_to_open,internal_ip);
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
		g_printerr ("port mapping %d added for %s\n",port_to_open,internal_ip);
		success=TRUE;
	} else {
		g_printerr ("add_port_mapping: Error: %s\n", error->message);
		g_error_free (error);
	}
	callback_executed=TRUE;

}

static void
upnp_ask_external_ip_cb (GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy,
    gpointer           userdata)
{
	GError *error = NULL;

	g_printerr ("calling GetExternalIPAddress\n");
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
		g_printerr ("External IP address is %s\n", ip);
		success=TRUE;
	} else {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}
	callback_executed=TRUE;
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

void do_loop()
{
	g_printerr ("do_loop()\n");
	time_t t,t2,t3;
	for (t3=t2=t=time(NULL);callback_executed==FALSE && t2-t<TIMEOUT ; t2=time(NULL))
	{
		if (t2-t3>=NOTIFY_INTERVAL)
		{
			g_printerr ("[t:%d]",t2-t3);
			t3=time(NULL);
		}
		g_main_context_iteration(NULL,FALSE);	
	}
}

void init()
{
	g_printerr ("init()\n");
	callback_executed=FALSE;
	success=FALSE;
}

gboolean upnp_get_ip(char** outIp)
{
	g_printerr ("upnp_get_ip(...)\n");
	ip=outIp;

	GUPnPControlPoint *cp=upnp_get_control_point();
	init();
	
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
	do_loop();
	
	/* Clean up */
	g_object_unref (cp);

	return callback_executed && success;
}

gboolean upnp_add_port_mapping(int port)
{
	g_printerr ("upnp_add_port_mapping(%d)\n",port);
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


