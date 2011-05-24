/*
 * GStreamer
 * Copyright (C) 2011 RidgeRun
 *
 */

#include <config.h>
#include <gst/gst.h>
#include "gstanaglyph.h"

static gboolean
anaglyph_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "anaglyph", GST_RANK_NONE,
      GST_TYPE_ANAGLYPH)) {
    return FALSE;
  }

  return TRUE;
}

/* gstreamer looks for this structure to register plugins
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "anaglyph",
    "Anaglyph tools plugin",
    anaglyph_init,
    PACKAGE_VERSION,
    "BSD",
    "RidgeRun",
    "http://ridgerun.github.com"
)
