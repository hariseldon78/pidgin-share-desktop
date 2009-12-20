/*
 * ShareDesktop - Share desktop using vnc between two buddies
 * Copyright (C) 2009 Roberto Previdi <hariseldon78@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#define PURPLE_PLUGINS

#define PLUGIN_ID			"core-hariseldon78-sharedesk"
#define PLUGIN_NAME			"Share Desktop"
#define PLUGIN_STATIC_NAME		sharedesk
#define PLUGIN_SUMMARY			"Share desktop with buddies using vnc."
#define PLUGIN_DESCRIPTION		"Share desktop with buddies using vnc."
#define PLUGIN_AUTHOR			"Roberto Previdi <hariseldon78@gmail.com>"
#define PLUGIN_VERSION			"0.1"
#define PLUGIN_WEBSITE			"http://"

#define	PREF_PREFIX			"/plugins/core/" PLUGIN_ID
#define	PREF_SERVER_COMMAND_LINE	PREF_PREFIX "/server_command_line"
#define	PREF_CLIENT_COMMAND_LINE	PREF_PREFIX "/client_command_line"

#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "plugin.h"
#include "version.h"
#include "blist.h"
#include "request.h"

#include "upnp-functions.h"

PurplePlugin *the_plugin;
PurpleValue *server_ip, *port;
pid_t server_pid=0;



static void
sharedesk_accept_connection_cb(void* user_data)
{
	purple_debug_misc(PLUGIN_ID,"sharedesk_accept_connection_cb(...)\n");
	char** splitted_command=(char**)user_data;
/*	PurpleAccount* account=(PurpleAccount*)splitted_command[5];
	PurpleConversation *conv=purple_conversation_new (PURPLE_CONV_TYPE_IM, account, splitted_command[6]);
	PurpleConvIm *im = purple_conversation_get_im_data(conv);
	const char* msg="sharedesk|accept_connection|"
		"\nThe buddy is accepting to share your desktop"
		"\n|||";

	purple_conv_im_send(im, msg); */
	
	g_strfreev(splitted_command);
	if (fork()==0)
	{
		const char* cmd=purple_prefs_get_string(PREF_CLIENT_COMMAND_LINE);

		/* sostituisco la porta nella stringa di comando */
		GRegex *port_regex=g_regex_new("[$]PORT",0,0,NULL);
		char* cmd1=g_regex_replace(port_regex,cmd,-1,0,purple_value_get_string(port),0,NULL);

		GRegex *ip_regex=g_regex_new("[$]SERVER_IP",0,0,NULL);
		char* cmd2=g_regex_replace(ip_regex,cmd,-1,0,purple_value_get_string(server_ip),0,NULL);


		purple_debug_misc(PLUGIN_ID,"client command=\"%s\"\n",cmd2);

		system(cmd2);
		
		g_free(cmd2);
		g_free(ip_regex);
		g_free(cmd1);
		g_free(port_regex);
	}
}

static void
sharedesk_refuse_connection_cb(void* user_data)
{
	purple_debug_misc(PLUGIN_ID,"sharedesk_refuse_connection_cb(...)\n");
	char** splitted_command=(char**)user_data;
	PurpleAccount* account=(PurpleAccount*)splitted_command[5];
	PurpleConversation *conv=purple_conversation_new (PURPLE_CONV_TYPE_IM, account, splitted_command[6]);
	PurpleConvIm *im = purple_conversation_get_im_data(conv);
	const char* msg="sharedesk|refuse_connection|"
		"\nThe buddy is refusing to share your desktop"
		"\n|||";

	purple_conv_im_send(im, msg); 
	g_strfreev(splitted_command);
}




/* callback richiamata quando arriva un messaggio im */
static gboolean 
receiving_im_cb(PurpleAccount *account, char **sender,
    char **message, PurpleConversation *conv,
                             PurpleMessageFlags *flags)
{
	/*cerco nel messaggio se c'e' qualche avviso per il plugin*/
	GRegex *command_regex=g_regex_new("sharedesk[|].*[|][|][|]",G_REGEX_DOTALL,0,NULL);
	GMatchInfo *match_info;
	if(!g_regex_match(command_regex,*message,0,&match_info))
		return FALSE; /* tiene il messaggio cosi' com'e' */

	char *command=g_match_info_fetch(match_info,0);
	purple_debug_misc(PLUGIN_ID,"received command:%s\n",command);

	gchar **splitted_command=g_strsplit(command,"|",0);
	int count_command_parts;
	purple_debug_misc(PLUGIN_ID,"splitting command\n");
	for (count_command_parts=0;splitted_command[count_command_parts]!=NULL;count_command_parts++)
	{
		purple_debug_misc(PLUGIN_ID,"im[%d]=\"%s\"\n",count_command_parts,splitted_command[count_command_parts]);
	}


	/* command processing */
	if (g_strcmp0(splitted_command[1],"request_connection")==0)
	{
		/* args:
		 2: ip
		 3: port
		 4: description
		 5,6,7: unused
		 */
		char msg[100];
		snprintf(msg,100,"Your buddy %s is asking to you to share his desktop. Accept?",*sender);
		purple_value_set_string(server_ip,splitted_command[2]);
		purple_value_set_string(port,splitted_command[3]);
		splitted_command[5]=(char*)account;
		splitted_command[6]=g_strdup(*sender);
		purple_debug_misc(PLUGIN_ID,"purple_request_action(%s,...)\n",msg);
		purple_request_action(the_plugin,
			"Share desktop",
		    	"Connection request",
		    	msg,
			0,
			account,
			*sender,
			conv,
			splitted_command,
		    	2,
		    	"_Accept", 
		    	sharedesk_accept_connection_cb,
		    	"_Refuse", 
			sharedesk_refuse_connection_cb);
		
	}
	else if(g_strcmp0(splitted_command[1],"refuse_connection")==0)
	{
		char msg[100];
		snprintf(msg,100,"%s has refused to share your desktop",*sender);
		purple_notify_info(the_plugin,"Share desktop","Refused connection",msg);
	}

	char* msg=g_regex_replace(command_regex,*message,-1,0,"",0,NULL);
	g_free(*message);
	*message=msg; /* giusto??? controllare! */
		return FALSE;

	/*trimmiamo*/
	g_strstrip(*message);

	/*purple_notify_info(the_plugin,"Debug","Messaggio modificato",*message);*/

	/* dopo aver sottratto la stringa di comando da message, se rimane qualcosa	
	 return FALSE;
	 altrimenti se il messaggio e' una stringa vuota (trimmando) 
	 return TRUE
	 che fa annullare la ricezione del messaggio, cosi' non appare una finestra vuota*/

	g_free(command_regex);
	g_free(match_info);

	if (g_strcmp0("",*message)==0)
		return TRUE;
	else 
		return FALSE;
}






static gboolean
plugin_unload(PurplePlugin *plugin)
{
	g_free(server_ip);
	g_free(port);

	return TRUE;
}




/* finestra presentata per configurare il plugin */
static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *pref;

	frame = purple_plugin_pref_frame_new();

	pref = purple_plugin_pref_new_with_name_and_label(PREF_SERVER_COMMAND_LINE,"Vnc server command line");
	purple_plugin_pref_frame_add(frame, pref);

	pref = purple_plugin_pref_new_with_name_and_label(PREF_CLIENT_COMMAND_LINE,"Vnc client command line");
	purple_plugin_pref_frame_add(frame, pref);

	return frame;
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};




static void
dont_do_it_cb(PurpleBlistNode *node, const char *ignore)
{
}




static void 
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
static void 
start_connection(PurpleBlistNode *node)
{

	const char* cmd=purple_prefs_get_string(PREF_SERVER_COMMAND_LINE);

	/* sostituisco la porta nella stringa di comando */
	GRegex *port_regex=g_regex_new("[$]PORT",0,0,NULL);
	char* msg=g_regex_replace(port_regex,cmd,-1,0,purple_value_get_string(port),0,NULL);

	purple_debug_misc(PLUGIN_ID,"server command=\"%s\"\n",msg);
	server_pid=fork();
	if (server_pid==0)
	{
		/* child process: eseguo il server */
		system(msg);
	}
	else 
	{
		/* original process: invio la richiesta di connessione*/
		send_connect_request_message(node);

		/* il plugin ora non deve far altro che aspettare la richiesta di chiusura */
	}
	g_free(msg);
	g_free(port_regex);
}




/* callback richiamata quando l'utente ha inserito la porta */
static void
sharedesk_set_ip_cb(PurpleBlistNode *node, const char *_server_ip)
{


	purple_value_set_string(server_ip,_server_ip);
	start_connection(node);

}




/* callback richiamata quando l'utente ha inserito la porta su cui eseguire */
static void
sharedesk_set_port_cb(PurpleBlistNode *node, const char *_port)
{

	purple_value_set_string(port,_port);
	if (!upnp_add_port_mapping(atoi(_port)))
	{
		char msg[100];
		snprintf(msg,100,"You must manually open the port %d on the router, if any",atoi(_port));
		purple_notify_warning(the_plugin,"No upnp available","Port mapping",msg);
	}


	char* upnp_ip=NULL;
	upnp_get_ip(&upnp_ip);

	purple_debug_misc(PLUGIN_ID,"ip=\"%s\"\n",upnp_ip);
	purple_value_set_string(server_ip,upnp_ip);


	purple_debug_misc(PLUGIN_ID,"server_ip=\"%s\"\n",purple_value_get_string(server_ip));

	/* se non è disponibile lo chiedo */
	if (g_strcmp0(purple_value_get_string(server_ip),"")==0 || g_strcmp0(purple_value_get_string(server_ip),"(null)")==0 )
		purple_request_input(node, "Share desktop",
		    "Enter the local ip address...",
		    NULL,
		    "", TRUE, FALSE, "html",
		    "Save", G_CALLBACK(sharedesk_set_ip_cb),
		    "Cancel", G_CALLBACK(dont_do_it_cb),
		    NULL, NULL, NULL,
		    node);
	else
	{
		purple_notify_info(the_plugin,"Debug","Ip rilevato via upnp",purple_value_get_string(server_ip));
		start_connection(node);
	}
}





/* callback richiamata quando l'utente clicca "share desktop" nel menu */
static void
sharedesk_request_connection_cb(PurpleBlistNode *node, gpointer data)
{

	purple_request_input(node,"Share desktop",
	    "Choose a port ...",
	    NULL,
	    "", TRUE, FALSE, "html",
	    "Save", G_CALLBACK(sharedesk_set_port_cb),
	    "Cancel", G_CALLBACK(dont_do_it_cb),
	    NULL, NULL, NULL,
	    node);

}





/* callback richiamata quando pidgin sta costruendo il menu di un buddy */
static void
sharedesk_extended_menu_cb(PurpleBlistNode *node, GList **m)
{

	PurpleMenuAction *bna = NULL;

	/* controlla che sia un nodo buddy */
	if (!(PURPLE_BLIST_NODE_IS_BUDDY(node) ))
		return;

	*m = g_list_append(*m, bna);

	bna = purple_menu_action_new("Share desktop...", PURPLE_CALLBACK(sharedesk_request_connection_cb), 
	    NULL, NULL);

	*m = g_list_append(*m, bna);

}



static gboolean
plugin_load(PurplePlugin *plugin)
{
	/* leggo i messaggi in arrivo per cercare un avviso di connessione*/
	purple_signal_connect(purple_conversations_get_handle(), "receiving-im-msg",
	    plugin, PURPLE_CALLBACK(receiving_im_cb),NULL);

	/* attacco al nodo la voce di menu */
	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu",
	    plugin, PURPLE_CALLBACK(sharedesk_extended_menu_cb), NULL);

	/* inizializzo le opzioni */

	port = purple_value_new(PURPLE_TYPE_STRING);
	server_ip = purple_value_new(PURPLE_TYPE_STRING);


	return TRUE;
}


static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,			/* Magic				*/
	PURPLE_MAJOR_VERSION,			/* Purple Major Version	*/
	PURPLE_MINOR_VERSION,			/* Purple Minor Version	*/
	PURPLE_PLUGIN_STANDARD,			/* plugin type			*/
	NULL,					/* ui requirement		*/
	0,					/* flags				*/
	NULL,					/* dependencies			*/
	PURPLE_PRIORITY_DEFAULT,			/* priority				*/

	PLUGIN_ID,				/* plugin id			*/
	PLUGIN_NAME,				/* name					*/
	PLUGIN_VERSION,				/* version				*/
	PLUGIN_SUMMARY,				/* summary				*/
	PLUGIN_DESCRIPTION,			/* description			*/
	PLUGIN_AUTHOR,				/* author				*/
	PLUGIN_WEBSITE,				/* website				*/

	plugin_load,				/* load					*/
	plugin_unload,				/* unload				*/
	NULL,					/* destroy				*/

	NULL,					/* ui_info				*/
	NULL,					/* extra_info			*/
	&prefs_info,				/* prefs_info			*/
	NULL,					/* actions				*/

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin(PurplePlugin *plugin) {

	/* per le notify */
	the_plugin=plugin;

	purple_prefs_add_none(PREF_PREFIX);
	purple_prefs_add_string(PREF_SERVER_COMMAND_LINE, "x11vnc -shared -rfbport $PORT");
	purple_prefs_add_string(PREF_CLIENT_COMMAND_LINE, "vncviewer $SERVER_IP::$PORT");
	/*
	 gupnp_proxy=NULL;
	 gupnp_init();*/

}
PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)