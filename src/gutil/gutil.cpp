/* gutil.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gst/gst.h>
#include <gst/audio/multichannel.h>
#include <gtk/gtk.h>
#include "util/logWriter.h"
#include "util/sigint.h"
#include "gutil/gutil.h"

int gutil::killMainLoop(gpointer /*data*/)
{
    static bool called = false;
    if (not called)
    {
        gtk_main_quit();

        LOG_DEBUG("Quitting...");
        called = true;
    }
    return FALSE;       // won't be called again
}

namespace {
int checkSignal(gpointer /*data*/)
{
    if (signal_handlers::signalFlag())
    {
        gutil::killMainLoop();
        return FALSE; // won't be called again
    }

    return TRUE; // keep calling
}
}

/// ms to run - 0 is forever
void gutil::runMainLoop(int ms)
{
    /// the gtk main loop will die after ms has elapsed
    if (ms != 0)
        g_timeout_add(ms, static_cast<GSourceFunc>(gutil::killMainLoop), NULL);

    // FIXME: this isn't very smart
    // poll signal status every quarter second
    g_timeout_add(300 /*ms*/,
            static_cast<GSourceFunc>(::checkSignal),
            NULL);

    gtk_main();
}

void gutil::init_gst_gtk(int argc, char **argv)
{
    // must initialise the threading system before using any other GLib funtion
    if (!g_thread_supported ())
        g_thread_init (NULL);

    gst_init(&argc, &argv);
    if (getenv("DISPLAY") != NULL)
        gtk_init(&argc, &argv);
    else
        LOG_DEBUG("DISPLAY variable has not been set");
}

void gutil::initAudioCapsFilter(GstElement *capsfilter, int numChannels)
{
    const gchar *audioFormats[] = {"audio/x-raw-float",
        "audio/x-raw-int", NULL};

    GstCaps *audioCaps = gst_caps_new_empty ();
    for (int i = 0; audioFormats[i] != NULL; i++)
    {
        GstStructure *structure = gst_structure_new (audioFormats[i], NULL);
        gst_structure_set (structure, "channels", G_TYPE_INT, numChannels, NULL);

        // set channel layout to none for more than 8 channels
        if (numChannels > 8)
        {
            GstAudioChannelPosition *ch_layout;

            ch_layout = g_new (GstAudioChannelPosition, numChannels);
            for (int j = 0; j < numChannels; ++j)
                ch_layout[j] = GST_AUDIO_CHANNEL_POSITION_NONE;

            gst_audio_set_channel_positions (structure, ch_layout);
            g_free (ch_layout);
        }

        gst_caps_append_structure (audioCaps, structure);
    }

    gchar *capsStr = gst_caps_to_string (audioCaps);
    LOG_DEBUG("Setting audio caps " << capsStr);
    g_free (capsStr);

    g_assert(audioCaps);
    g_object_set(G_OBJECT(capsfilter), "caps", audioCaps, NULL);
    gst_caps_unref(audioCaps);
}
