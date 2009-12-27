/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef SHAREDESK_H
#define SHAREDESK_H
#include <account.h>
#include "plugin.h"

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
#define	LABEL_SERVER_COMMAND_LINE	"Vnc server command line"
#define	TOOLTIP_SERVER_COMMAND_LINE	"Vnc server command line"
#define	DEFAULT_SERVER_COMMAND_LINE	"x11vnc -shared -rfbport $PORT"

#define	PREF_CLIENT_COMMAND_LINE	PREF_PREFIX "/client_command_line"
#define	LABEL_CLIENT_COMMAND_LINE	"Vnc client command line"
#define	TOOLTIP_CLIENT_COMMAND_LINE	"Vnc client command line"
#define	DEFAULT_CLIENT_COMMAND_LINE	"vncviewer $SERVER_IP::$PORT"

#define PREF_PORT			PREF_PREFIX "/port"
#define LABEL_PORT			"The connection port"
#define TOOLTIP_PORT			"The vnc connection port"
#define DEFAULT_PORT			"5901"

#define	PREF_USE_UPNP			PREF_PREFIX "/use_upnp"
#define	LABEL_USE_UPNP			"Use upnp"
#define	TOOLTIP_USE_UPNP		"Use upnp for ip detection and port mapping"
#define	DEFAULT_USE_UPNP		TRUE

#define	PREF_USE_LIBPURPLE_UPNP		PREF_PREFIX "/use_libpurple_upnp"
#define	LABEL_USE_LIBPURPLE_UPNP	"Use libpurple upnp implementation"
#define	TOOLTIP_USE_LIBPURPLE_UPNP	"Use libpurple upnp implementation instead of libgupnp"
#define	DEFAULT_USE_LIBPURPLE_UPNP	FALSE


PurplePlugin *the_plugin;

typedef  struct _connection_parameters
{
	char* ip;
	int port;
	PurpleAccount *account;
	char* sender;
} connection_parameters;

#endif