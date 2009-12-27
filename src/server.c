/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * pidgin-sharedesk
 * Copyright (C) Roberto Previdi 2009 <hariseldon78@gmail.com>
 * 
 * pidgin-sharedesk is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * pidgin-sharedesk is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server.h"
#include "sharedesk.h"
#include "network.h"

PurpleValue *server_ip, *port;
pid_t server_pid=0;

void
dont_do_it_cb(PurpleBlistNode *node, const char *ignore)
{
}




void 
send_connect_request_message(PurpleBlistNode *node)
{
	PurpleAccount* account=purple_buddy_get_account(PURPLE_BUDDY(node));

	PurpleConversation *conv=purple_conversation_new (PURPLE_CONV_TYPE_IM, account, purple_buddy_get_name (PURPLE_BUDDY(node)));
	PurpleConvIm *im = purple_conversation_get_im_data(conv);

	const char* remote_message="sharedesk|request_connection|$SERVER_IP|$PORT|"
		"\nPlease start a vnc connection. Sample command line:"
		"\n$CLIENT_CMD"
		"\n|||";
	GRegex *port_regex1=g_regex_new("[$]CLIENT_CMD",0,0,NULL);
	char* msg1=g_regex_replace(port_regex1,remote_message,-1,0,purple_prefs_get_string(PREF_CLIENT_COMMAND_LINE),0,NULL);
	GRegex *port_regex2=g_regex_new("[$]PORT",0,0,NULL);
	char* msg2=g_regex_replace(port_regex2,msg1,-1,0,purple_value_get_string(port),0,NULL);
	GRegex *port_regex3=g_regex_new("[$]SERVER_IP",0,0,NULL);
	char* msg3=g_regex_replace(port_regex3,msg2,-1,0,purple_value_get_string(server_ip),0,NULL);

	purple_conv_im_send(im, msg3); 


	g_free(msg3);
	g_free(msg2);
	g_free(msg1);
	g_free(port_regex1);
	g_free(port_regex2);
	g_free(port_regex3);
}




/* inizia la connessione quando tutti i dettagli sono disponibili */
void 
start_connection(PurpleBlistNode *node)
{
	purple_debug_misc(PLUGIN_ID,"server_ip=\"%s\"\n",purple_value_get_string(server_ip));

	const char* cmd=purple_prefs_get_string(PREF_SERVER_COMMAND_LINE);

	/* sostituisco la porta nella stringa di comando */
	GRegex *port_regex=g_regex_new("[$]PORT",0,0,NULL);
	char* command=g_regex_replace(port_regex,cmd,-1,0,purple_value_get_string(port),0,NULL);

	purple_debug_misc(PLUGIN_ID,"server command=\"%s\"\n",command);
	int server_pid=fork();
	if (server_pid==0)
	{
		/* child process: eseguo il server */
		gchar **splitted_command=g_strsplit(command," ",0);
		execvp(splitted_command[0],splitted_command);
		g_free(command);
	}
	else 
	{
		/* original process: invio la richiesta di connessione*/
		send_connect_request_message(node);

		/* il plugin ora non deve far altro che aspettare la richiesta di chiusura */
	}
	g_free(port_regex);
}

void 
no_upnp(PurpleBlistNode *node)
{
	char msg[100];
	snprintf(msg,100,"You must manually open the port %d on the router, if any",atoi(purple_value_get_string(port)));
	purple_notify_warning(the_plugin,"No upnp available","Port mapping",msg);
	char* ip=purple_network_get_my_ip(-1);
	purple_debug_misc(PLUGIN_ID,"purple_network_get_my_ip(-1)=\"%s\"\n",ip);
	purple_value_set_string(server_ip,ip);
	start_connection(node);
}

void 
upnp_discovery_cb(gboolean success, gpointer data)
{
	purple_notify_info(the_plugin,"Debug","Upnp discovery",success?"Ok!":"Failed!");
	if (!success)
		no_upnp((PurpleBlistNode *)data);
	if (purple_prefs_get_bool(PREF_USE_LIBPURPLE_UPNP))
	{
		char* ip=purple_upnp_get_public_ip();
		if (purple_network_ip_atoi(ip)==NULL)
			no_upnp((PurpleBlistNode *)data);
		purple_debug_misc(PLUGIN_ID,"purple_upnp_get_public_ip()=\"%s\"\n",ip);
		purple_value_set_string(server_ip,ip);
	
		purple_upnp_remove_port_mapping (atoi(purple_value_get_string(port)), "TCP", NULL, NULL);
		purple_upnp_set_port_mapping (atoi(purple_value_get_string(port)), "TCP", NULL, NULL);
	}
	else
	{
		char **ip;
		if (!upnp_get_ip(ip))
			no_upnp((PurpleBlistNode *)data);
		purple_debug_misc(PLUGIN_ID,"upnp_get_ip()=\"%s\"\n",*ip);
		purple_value_set_string(server_ip,*ip);
		
		if (!upnp_add_port_mapping(atoi(purple_value_get_string(port))))
			no_upnp((PurpleBlistNode *)data);
	}
	start_connection((PurpleBlistNode *)data);
}


/* callback richiamata quando l'utente clicca "share desktop" nel menu */
void
server_request_connection_cb(PurpleBlistNode *node, gpointer data)
{
	purple_value_set_string(port,purple_prefs_get_string(PREF_PORT));

	if (purple_prefs_get_bool(PREF_USE_UPNP))
	{
		if (purple_prefs_get_bool(PREF_USE_LIBPURPLE_UPNP))
		{
			purple_upnp_init();
			purple_upnp_discover (upnp_discovery_cb,(gpointer)node);
		}
		else
			upnp_discovery_cb(TRUE,(gpointer)node);
	}
	else
	{
		no_upnp(node);
	}
}

void server_unload()
{
	g_free(server_ip);
	g_free(port);
}

void server_load()
{
	port = purple_value_new(PURPLE_TYPE_STRING);
	server_ip = purple_value_new(PURPLE_TYPE_STRING);
}
