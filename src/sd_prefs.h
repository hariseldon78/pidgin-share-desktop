/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * pidgin-sharedesk
 * Copyright (C) Roberto Previdi 2010 <hariseldon78@gmail.com>
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

#ifndef _SD_PREFS_H_
#define _SD_PREFS_H_

#include "sharedesk.h"
#include <pluginpref.h>

#define	PREF_PREFIX				"/plugins/core/" PLUGIN_ID

#define	PREF_SERVER_COMMAND_LINE		PREF_PREFIX "/server_command_line"
#define	LABEL_SERVER_COMMAND_LINE		"Vnc server command line"
#define	DEFAULT_SERVER_COMMAND_LINE		"x11vnc -shared -rfbport $PORT"

#define	PREF_CLIENT_COMMAND_LINE		PREF_PREFIX "/client_command_line"
#define	LABEL_CLIENT_COMMAND_LINE		"Vnc client command line"
#define	DEFAULT_CLIENT_COMMAND_LINE		"vncviewer $SERVER_IP::$PORT"

#define	PREF_REVERSE_SERVER_COMMAND_LINE	PREF_PREFIX "/server_command_line"
#define	LABEL_REVERSE_SERVER_COMMAND_LINE	"Vnc server command line"
#define	DEFAULT_REVERSE_SERVER_COMMAND_LINE	"x11vnc -shared -connect $CLIENT_IP"

#define	PREF_REVERSE_CLIENT_COMMAND_LINE	PREF_PREFIX "/client_command_line"
#define	LABEL_REVERSE_CLIENT_COMMAND_LINE	"Vnc client command line"
#define	DEFAULT_REVERSE_CLIENT_COMMAND_LINE	"vncviewer -listen"

#define PREF_PORT				PREF_PREFIX "/port"
#define LABEL_PORT				"The connection port"
#define DEFAULT_PORT				"5900"

#define	PREF_USE_UPNP				PREF_PREFIX "/use_upnp"
#define	LABEL_USE_UPNP				"Use upnp (need reload)"
#define	DEFAULT_USE_UPNP			TRUE

#define	PREF_USE_LIBPURPLE_UPNP			PREF_PREFIX "/use_libpurple_upnp"
#define	LABEL_USE_LIBPURPLE_UPNP		"Use libpurple upnp implementation (need reload)"
#define	DEFAULT_USE_LIBPURPLE_UPNP		FALSE


PurplePluginPrefFrame * get_plugin_pref_frame(PurplePlugin *plugin);

void prefs_init();

#endif // _PREFS_H_
