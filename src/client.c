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

#include "client.h"
#include "sharedesk.h"
#include <debug.h>
#include <unistd.h>
#include <request.h>

void client_accept_connection()
{
}

void
client_accept_connection_cb(void* _cp)
{
	purple_debug_misc(PLUGIN_ID,"sharedesk_accept_connection_cb(...)\n");
	connection_parameters *cp=(connection_parameters*)_cp;
	/*	PurpleAccount* account=(PurpleAccount*)splitted_command[5];
	 PurpleConversation *conv=purple_conversation_new (PURPLE_CONV_TYPE_IM, account, splitted_command[6]);
	 PurpleConvIm *im = purple_conversation_get_im_data(conv);
	 const char* msg="sharedesk|accept_connection|"
	 "\nThe buddy is accepting to share your desktop"
	 "\n|||";

	 purple_conv_im_send(im, msg); */

	if (fork()==0)
	{
		const char* cmd=purple_prefs_get_string(PREF_CLIENT_COMMAND_LINE);

		/* sostituisco la porta nella stringa di comando */
		GRegex *port_regex=g_regex_new("[$]PORT",0,0,NULL);
		char s_port[10];
		snprintf(s_port,10,"%d",cp->port);
		char* cmd1=g_regex_replace(port_regex,cmd,-1,0,s_port,0,NULL);

		GRegex *ip_regex=g_regex_new("[$]SERVER_IP",0,0,NULL);
		char* cmd2=g_regex_replace(ip_regex,cmd1,-1,0,cp->ip,0,NULL);


		purple_debug_misc(PLUGIN_ID,"client command=\"%s\"\n",cmd2);

		system(cmd2);

		g_free(cmd2);
		g_free(ip_regex);
		g_free(cmd1);
		g_free(port_regex);
		g_free(cp);
	}
}



void
client_refuse_connection_cb(void* _cp)
{
	purple_debug_misc(PLUGIN_ID,"sharedesk_refuse_connection_cb(...)\n");
	connection_parameters *cp=(connection_parameters*)_cp;
	PurpleAccount* account=cp->account;
	PurpleConversation *conv=purple_conversation_new (PURPLE_CONV_TYPE_IM, account, cp->sender);
	PurpleConvIm *im = purple_conversation_get_im_data(conv);
	const char* msg="sharedesk|refuse_connection|"
		"\nThe buddy is refusing to share your desktop"
		"\n|||";

	purple_conv_im_send(im, msg); 
	g_free(cp);
}

void 
client_handle_connection_request(char** splitted_command,PurpleAccount *account,char* sender)
{
	/* args:
	 2: ip
	 3: port
	 4: description
	 5,6,7: unused
	 */
	char msg[100];
	snprintf(msg,100,"Your buddy %s is asking to you to share his desktop. Accept?",sender);

	connection_parameters *cp=(connection_parameters*)g_malloc(sizeof(connection_parameters));
	cp->ip=g_strndup(splitted_command[2],17);
	cp->port=atoi(splitted_command[3]);
	cp->account=account;
	cp->sender=g_strdup(sender);
	
	purple_debug_misc(PLUGIN_ID,"purple_request_action(%s,...)\n",msg);
	purple_request_action(the_plugin,
	    "Share desktop",
	    "Connection request",
	    msg,
	    0,
	    account,
	    sender,
	    NULL,
	    cp,
	    2,
	    "_Accept", 
	    client_accept_connection_cb,
	    "_Refuse", 
	    client_refuse_connection_cb);
}

