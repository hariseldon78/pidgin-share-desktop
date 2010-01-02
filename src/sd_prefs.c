/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

#include "sd_prefs.h"
/* finestra presentata per configurare il plugin */
PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{
	PurplePluginPrefFrame *frame;
	PurplePluginPref *pref;

	frame = purple_plugin_pref_frame_new();

	pref=purple_plugin_pref_new_with_label("Share desktop plugin\n\nbuild time: "BUILD_TIME);
	purple_plugin_pref_frame_add(frame, pref);
	

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

void prefs_init()
{
		purple_prefs_add_none	(PREF_PREFIX);
	purple_prefs_add_string	(PREF_SERVER_COMMAND_LINE, 	DEFAULT_SERVER_COMMAND_LINE	);
	purple_prefs_add_string	(PREF_CLIENT_COMMAND_LINE, 	DEFAULT_CLIENT_COMMAND_LINE	);
	purple_prefs_add_bool 	(PREF_USE_UPNP,			DEFAULT_USE_UPNP 		);
	purple_prefs_add_bool 	(PREF_USE_LIBPURPLE_UPNP,	DEFAULT_USE_LIBPURPLE_UPNP 	);
	purple_prefs_add_string (PREF_PORT,			DEFAULT_PORT 			);
	/*
	purple_prefs_add_bool (PREF_,DEFAULT_ );
*/
}

PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};