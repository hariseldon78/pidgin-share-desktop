/* We're including glib.h again for the gboolean type. */
#include <glib.h>

/* This is the required definition of PURPLE_PLUGINS as required for a plugin */
#define PURPLE_PLUGINS

/* Here we're including the necessary libpurple headers for this plugin.  Note
 * that we're including them in alphabetical order.  This isn't necessary but
 * we do this throughout our source for consistency. */
#include "debug.h"
#include "plugin.h"
#include "version.h"

/* It's more convenient to type PLUGIN_ID all the time than it is to type
 * "core-debugexample", so define this convenience macro. */
#define PLUGIN_ID "core-debugexample"

/* Common practice in third-party plugins is to define convenience macros for
 * many of the fields of the plugin info struct, so we'll do that for the
 * purposes of demonstration. */
#define PLUGIN_AUTHOR "John Bailey <rekkanoryo@rekkanoryo.org>"

/* As we've covered before, libpurple calls this function, if present, when it
 * loads the plugin.  Here we're using it to show off the capabilities of the
 * debug API and just blindly returning TRUE to tell libpurple it's safe to
 * continue loading. */
static gboolean
plugin_load(PurplePlugin *plugin)
{
    /* Define these for convenience--we're just using them to show the
     * similarities of the debug functions to the standard printf(). */
    gint i = 256;
    gfloat f = 512.1024;
    const gchar *s = "example string";

    /* Introductory message */
    purple_debug_info(PLUGIN_ID,
            "Called plugin_load.  Beginning debug demonstration\n");

    /* Show off the debug API a bit */
    purple_debug_misc(PLUGIN_ID,
            "MISC level debug message.  i = %d, f = %f, s = %s\n", i, f, s);

    purple_debug_info(PLUGIN_ID,
            "INFO level debug message.  i = %d, f = %f, s = %s\n", i, f, s);

    purple_debug_warning(PLUGIN_ID,
            "WARNING level debug message.  i = %d, f = %f, s = %s\n", i, f, s);

    purple_debug_error(PLUGIN_ID,
            "ERROR level debug message.  i = %d, f = %f, s = %s\n", i, f, s);

    purple_debug_fatal(PLUGIN_ID,
            "FATAL level debug message. i = %d, f = %f, s = %s\n", i, f, s);

    /* Now just return TRUE to tell libpurple to finish loading. */
    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,        /* magic number */
    PURPLE_MAJOR_VERSION,       /* purple major */
    PURPLE_MINOR_VERSION,       /* purple minor */
    PURPLE_PLUGIN_STANDARD,     /* plugin type */
    NULL,                       /* UI requirement */
    0,                          /* flags */
    NULL,                       /* dependencies */
    PURPLE_PRIORITY_DEFAULT,    /* priority */

    PLUGIN_ID,                  /* id */
    "Debug API Example",        /* name */
    "1.0",                      /* version */
    "Debug API Example",        /* summary */
    "Debug API Example",        /* description */
    PLUGIN_AUTHOR,              /* author */
    "http://pidgin.im",         /* homepage */

    plugin_load,                /* load */
    NULL,                       /* unload */
    NULL,                       /* destroy */

    NULL,                       /* ui info */
    NULL,                       /* extra info */
    NULL,                       /* prefs info */
    NULL,                       /* actions */
    NULL,                       /* reserved */
    NULL,                       /* reserved */
    NULL,                       /* reserved */
    NULL                        /* reserved */
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(debugexample, init_plugin, info)