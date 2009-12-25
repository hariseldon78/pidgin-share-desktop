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


#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "version.h"
#include "blist.h"
#include "request.h"

#include "sharedesk.h"
#include "client.h"
#include "server.h"
#include "upnp-functions.h"








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
		client_handle_connection_request(splitted_command,account,*sender);
		
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
	server_unload();
	return TRUE;
}




/* finestra presentata per configurare il plugin */
static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *pref;

	frame = purple_plugin_pref_frame_new();

	pref = purple_plugin_pref_new_with_name_and_label(PREF_SERVER_COMMAND_LINE,LABEL_SERVER_COMMAND_LINE);
	purple_plugin_pref_frame_add(frame, pref);

	pref = purple_plugin_pref_new_with_name_and_label(PREF_CLIENT_COMMAND_LINE,LABEL_CLIENT_COMMAND_LINE);
	purple_plugin_pref_frame_add(frame, pref);

	pref = purple_plugin_pref_new_with_name_and_label(PREF_USE_UPNP,LABEL_USE_UPNP);
	purple_plugin_pref_frame_add(frame, pref);
	
	pref = purple_plugin_pref_new_with_name_and_label(PREF_USE_LIBPURPLE_UPNP,LABEL_USE_LIBPURPLE_UPNP);
	purple_plugin_pref_frame_add(frame, pref); 
	
	pref = purple_plugin_pref_new_with_name_and_label(PREF_PORT,LABEL_PORT);
	purple_plugin_pref_frame_add(frame, pref);

/*
	pref = purple_plugin_pref_new_with_name_and_label(PREF_,LABEL_);
	purple_plugin_pref_frame_add(frame, pref); */

	
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










/* callback richiamata quando pidgin sta costruendo il menu di un buddy */
static void
sharedesk_extended_menu_cb(PurpleBlistNode *node, GList **m)
{

	PurpleMenuAction *bna = NULL;

	/* controlla che sia un nodo buddy */
	if (!(PURPLE_BLIST_NODE_IS_BUDDY(node) ))
		return;

	*m = g_list_append(*m, bna);

	bna = purple_menu_action_new("Share desktop...", PURPLE_CALLBACK(server_request_connection_cb), 
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

	server_load();

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

	purple_prefs_add_none	(PREF_PREFIX);
	purple_prefs_add_string	(PREF_SERVER_COMMAND_LINE, 	DEFAULT_SERVER_COMMAND_LINE	);
	purple_prefs_add_string	(PREF_CLIENT_COMMAND_LINE, 	DEFAULT_CLIENT_COMMAND_LINE	);
	purple_prefs_add_bool 	(PREF_USE_UPNP,			DEFAULT_USE_UPNP 		);
	purple_prefs_add_bool 	(PREF_USE_LIBPURPLE_UPNP,	DEFAULT_USE_LIBPURPLE_UPNP 	);
	purple_prefs_add_string (PREF_PORT,			DEFAULT_PORT 			);
	/*
	purple_prefs_add_bool (PREF_,DEFAULT_ );
	 gupnp_proxy=NULL;
	 gupnp_init();*/

}
PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)