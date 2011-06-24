/*
 * Copyright (C) 2011 RidgeRun
 *
 */

#ifndef __GST_ANAGLYPH_H__
#define __GST_ANAGLYPH_H__

#include <gst/gst.h>
#include <gst/base/gstcollectpads.h>
#include "anaglyph.h"

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_ANAGLYPH \
  (gst_anaglyph_get_type())
#define GST_ANAGLYPH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ANAGLYPH,GstAnaglyph))
#define GST_ANAGLYPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ANAGLYPH,GstAnaglyphClass))
#define GST_IS_ANAGLYPH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ANAGLYPH))
#define GST_IS_ANAGLYPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ANAGLYPH))

typedef struct _GstAnaglyph      GstAnaglyph;
typedef struct _GstAnaglyphClass GstAnaglyphClass;

struct _GstAnaglyph
{
  GstElement element;

  GstPad *srcpad;
  
  /* Caps of the requested pads to compare */
  GstCaps *caps;
  
  /* Sink pads using Collect Pads from core's base library */
  GstCollectPads *collect;
  /* Lock to prevent the state to change while working */
  GMutex *state_lock;
  
  /* Type of anaglyph image to produce */
  ana_mode mode;
  
  /* How many pads we have allocated */
  gint padcount;
  /* Image information */
  gint width;
  gint height;
  gint bpp;
  gint imagesize;
};

struct _GstAnaglyphClass 
{
  GstElementClass parent_class;
};

GType gst_anaglyph_get_type (void);

G_END_DECLS

#endif /* __GST_ANAGLYPH_H__ */
