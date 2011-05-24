/*
 * Copyright (C) 2011 RidgeRun
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>


#include "gstanaglyph.h"
#include "anaglyph.h"

GST_DEBUG_CATEGORY_STATIC(gst_anaglyph_debug);
#define GST_CAT_DEFAULT gst_anaglyph_debug

#define GST_ANAGLYPH_GET_STATE_LOCK(mix) \
  (GST_ANAGLYPH(mix)->state_lock)
#define GST_ANAGLYPH_STATE_LOCK(mix) \
  (g_mutex_lock(GST_ANAGLYPH_GET_STATE_LOCK(mix)))
#define GST_ANAGLYPH_STATE_UNLOCK(mix) \
  (g_mutex_unlock(GST_ANAGLYPH_GET_STATE_LOCK(mix)))

/* Filter signals and args */
enum
{
  LAST_SIGNAL
};

enum
{	
  PROP_0,
  PROP_MODE,	
};

/* the capabilities of the inputs and outputs.
 *
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink_%d",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("video/x-raw-rgb, "\
							"bpp=(int)24, "\
							"endianness=(int)4321, "\
							"depth=(int)24, "\
							"red_mask=(int)16711680, "\
							"green_mask=(int)65280, "\
							"blue_mask=(int)255, "\
							"width=(int)[1,MAX], "\
							"height=(int)[1,MAX], "\
							"framerate=(fraction)[0/1,60/1]")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-rgb, "\
							"bpp=(int)24, "\
							"endianness=(int)4321, "\
							"depth=(int)24, "\
							"red_mask=(int)16711680, "\
							"green_mask=(int)65280, "\
							"blue_mask=(int)255, "\
							"width=(int)[1,MAX], "\
							"height=(int)[1,MAX], "\
							"framerate=(fraction)[0/1,60/1]")
    );


GST_BOILERPLATE (GstAnaglyph, gst_anaglyph, GstElement,
    GST_TYPE_ELEMENT);

static void gst_anaglyph_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_anaglyph_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_anaglyph_finalize (GObject * object);
/* GstCollectPad */
static GstPad *gst_anaglyph_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name);
static void gst_anaglyph_release_pad (GstElement * element, GstPad * pad);
static GstStateChangeReturn gst_anaglyph_change_state (GstElement * element,
    GstStateChange transition);
static GstFlowReturn gst_anaglyph_collected (GstCollectPads * pads,
    GstAnaglyph * filter);
static gboolean gst_anaglyph_sink_pad_setcaps (GstPad *pad, GstCaps *caps);

/* GObject vmethod implementations */

static void
gst_anaglyph_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "anaglyph",
    "video",
    "Combines two visual perspectives into an anaglyph video stream",
    "Michael Gruner <michael.gruner@ridgerun.com>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));	 	   
}

/* initialize the anaglyph's class */
static void
gst_anaglyph_class_init (GstAnaglyphClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  
  gobject_class->finalize = 
      GST_DEBUG_FUNCPTR (gst_anaglyph_finalize);
  gobject_class->set_property = 
      GST_DEBUG_FUNCPTR (gst_anaglyph_set_property);
  gobject_class->get_property = 
      GST_DEBUG_FUNCPTR (gst_anaglyph_get_property);
  /* CollectPads */    
  gstelement_class->request_new_pad =
      GST_DEBUG_FUNCPTR (gst_anaglyph_request_new_pad);
  gstelement_class->release_pad =
      GST_DEBUG_FUNCPTR (gst_anaglyph_release_pad);
  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_anaglyph_change_state);
      
  g_object_class_install_property (gobject_class, PROP_MODE,
      g_param_spec_int ("mode", "Mode", "Anaglyph type of images",
			ANA_MONOCHROME, ANA_NONE, ANA_COLOR, G_PARAM_READWRITE));    
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_anaglyph_init (GstAnaglyph * filter,
    GstAnaglyphClass * gclass)
{
  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");

  filter->collect = gst_collect_pads_new ();
  filter->state_lock = g_mutex_new ();
  gst_collect_pads_set_function (filter->collect,
      (GstCollectPadsFunction) GST_DEBUG_FUNCPTR (gst_anaglyph_collected),
      filter);								

  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->mode = ANA_COLOR;
  filter->caps = NULL;
  filter->padcount = 0;
}

static void
gst_anaglyph_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAnaglyph *filter = GST_ANAGLYPH (object);

  switch (prop_id)
  {
	case PROP_MODE:
	  filter->mode = g_value_get_int (value);
      break;  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_anaglyph_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAnaglyph *filter = GST_ANAGLYPH (object);

  switch (prop_id) 
  {
	case PROP_MODE:
	  g_value_set_int (value, filter->mode);
	  break;  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */
static GstStateChangeReturn
gst_anaglyph_change_state (GstElement * element, GstStateChange transition)
{
  GstAnaglyph *mix = GST_ANAGLYPH(element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  
  g_return_val_if_fail (GST_IS_ANAGLYPH (element), GST_STATE_CHANGE_FAILURE);

  /* Handle ramp-up state changes */
  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      gst_collect_pads_start (mix->collect);
      break;
    default:
      break;
  }

  /* Pass state changes to base class */
  ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
	return ret;

  /* Handle ramp-down state changes */
  switch (transition) 
  {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_collect_pads_stop (mix->collect);
      break;
    default:
      break;
  }

  return ret;
}  

static void
gst_anaglyph_release_pad (GstElement * element, GstPad * pad)
{  	
  GstAnaglyph *mix = GST_ANAGLYPH(element);
  
  GST_ANAGLYPH_STATE_LOCK(mix);
  if (!gst_collect_pads_remove_pad (mix->collect, pad)) {
    GST_ELEMENT_ERROR(element,STREAM,FAILED,(NULL),
      ("Failed to remove pad from collector"));
  }
  GST_ANAGLYPH_STATE_UNLOCK(mix);

  if (!gst_element_remove_pad (element, pad)) {
    GST_ELEMENT_ERROR(element,STREAM,FAILED,(NULL),
      ("Failed to remove pad from element"));
  }
}

static gboolean gst_anaglyph_sink_pad_setcaps (GstPad *pad, 
	GstCaps *caps)
{
  GstAnaglyph *mix = GST_ANAGLYPH(GST_PAD_PARENT(pad));
  GList *pads;
  GstStructure *structure;

  GST_DEBUG_OBJECT (mix, "setting caps on pad %p,%s to %s", pad,
    GST_PAD_NAME (pad), gst_caps_to_string(caps));

  GST_OBJECT_LOCK(mix);
  pads = GST_ELEMENT(mix)->pads;
  while (pads) {
    GstPad *otherpad = GST_PAD(pads->data);

    if (otherpad != pad){
       if (gst_pad_get_direction(otherpad) == GST_PAD_SINK) {
           if (GST_PAD_CAPS(otherpad)){
			   if (!gst_caps_is_equal(GST_PAD_CAPS(otherpad),caps)){
			     GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
                   ("Got different caps on each sink pad"));
				 return FALSE;
			   }
		   }
       }
    }
    pads = g_list_next(pads);
  }
  GST_OBJECT_UNLOCK(mix);

  /* If we have not defined the src pad caps yet, lets do it */
  if (!GST_PAD_CAPS(mix->srcpad)) {
    structure = gst_caps_get_structure(caps, 0);

    if (!gst_structure_get_int(structure, "width", &mix->width)) {
      GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
        ("Width caps are required"));
	  return FALSE;
    }
    if (!gst_structure_get_int(structure, "height", &mix->height)){
      GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
        ("Height caps are required"));
	  return FALSE;
    }
	if (!gst_structure_get_int(structure, "depth", &mix->bpp)) {
      GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
        ("depth caps are required"));
	  return FALSE;
	}
	mix->bpp /= 8;
    mix->imagesize = mix->width * mix->height * mix->bpp;

    if (!gst_pad_set_caps(mix->srcpad,caps)) {
      GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
        ("Unable to set caps on the src pad"));
	  return FALSE;
    }
  }

  return TRUE;
}											   

static GstPad *
gst_anaglyph_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * req_name)
{
  GstAnaglyph *mix = GST_ANAGLYPH(element);
  gchar *name = NULL;
  GstPad *newpad;
  
  g_return_val_if_fail (templ != NULL, NULL);

  if (G_UNLIKELY(templ->direction != GST_PAD_SINK)) 
  {
    GST_ELEMENT_ERROR(element,STREAM,FAILED,(NULL),
      ("Request pad that is not a SINK pad"));
    return NULL;
  }

  name = g_strdup_printf("sink%d", 
	g_atomic_int_exchange_and_add(&mix->padcount, 1));
		
  /* Create the pad */
  newpad = gst_pad_new_from_template(templ,name);
  g_free (name);

  gst_pad_set_setcaps_function(newpad, GST_DEBUG_FUNCPTR(
    gst_anaglyph_sink_pad_setcaps));

  GST_ANAGLYPH_STATE_LOCK(mix);
  if (gst_collect_pads_add_pad (mix->collect, newpad,
	    sizeof (GstCollectData)) == NULL) {
    GST_ELEMENT_ERROR(element,STREAM,FAILED,(NULL),
      ("Unable to add requested pad to the collector"));
	GST_ANAGLYPH_STATE_UNLOCK(mix);
    return NULL;  	 	
  }
	
  GST_ANAGLYPH_STATE_UNLOCK(mix);

  /* Add the pad to the element */
  if (!gst_element_add_pad (element, newpad)){
    GST_ELEMENT_ERROR(element,STREAM,FAILED,(NULL),
      ("Unable to add requested pad to the element"));
  }

  return newpad;
}


static GstFlowReturn
gst_anaglyph_collected (GstCollectPads *pads, GstAnaglyph * mix)
{
  GstBuffer *outbuf = NULL, *buf1 = NULL, *buf2 = NULL;

  g_return_val_if_fail (GST_IS_ANAGLYPH (mix), GST_FLOW_ERROR);
  
  if (!GST_PAD_CAPS(mix->srcpad)) {
    GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
      ("Anaglyph requires input caps to be set to work"));
  }
    
  GST_ANAGLYPH_STATE_LOCK (mix);
  /* Ensure strictly two sink pads were requested */   
  if (mix->padcount != 2)
  {
    GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
      ("Anaglyph works with 2 and only 2 inputs, currently only %d is provided",
	    mix->padcount));
    GST_ANAGLYPH_STATE_UNLOCK (mix);
	return GST_FLOW_ERROR;	  
  } 

  if (gst_collect_pads_available(pads) < mix->imagesize) {
	if (gst_collect_pads_available(pads) == 0) {
		/* This is an EOS */
		gst_pad_push_event(mix->srcpad,gst_event_new_eos());
	} else {
  	  /* We need more data, lets talk later */
	  GST_WARNING_OBJECT(mix,
	    "Not enought input data to the anaglyph (%s), waiting for more..",
	    gst_collect_pads_available(pads));
	}
    GST_ANAGLYPH_STATE_UNLOCK (mix);
	return GST_FLOW_OK;
  }

  /* First buffer */
  buf1 = gst_collect_pads_take_buffer (pads, pads->data->data,mix->imagesize);
  
  /* Second buffer */
  buf2 = gst_collect_pads_take_buffer (pads,
    g_slist_next(pads->data)->data,mix->imagesize);
 
  if (gst_pad_alloc_buffer_and_set_caps (mix->srcpad,0,mix->imagesize,
		GST_PAD_CAPS(mix->srcpad),&outbuf) != GST_FLOW_OK)
  {		
    GST_ELEMENT_ERROR(mix,STREAM,FAILED,(NULL),
      ("Unable to allocate buffer from src pad"));
	gst_buffer_unref (buf1);
	gst_buffer_unref (buf2);  
    GST_ANAGLYPH_STATE_UNLOCK (mix);  
	return GST_FLOW_ERROR;
  }	

  /* Anaglyph main process */
  ana_process (GST_BUFFER_DATA(buf1), GST_BUFFER_DATA(buf2), 
               GST_BUFFER_DATA(outbuf), GST_BUFFER_SIZE(outbuf), 
               mix->mode); 
			   
  gst_buffer_copy_metadata (outbuf, buf2, GST_BUFFER_COPY_ALL);

  gst_buffer_unref (buf1);
  gst_buffer_unref (buf2); 

  if (gst_pad_push (mix->srcpad, outbuf) != GST_FLOW_OK) {		
    GST_WARNING_OBJECT(mix,"Can't push buffer to src pad");
  }
  
  GST_ANAGLYPH_STATE_UNLOCK (mix);

  return GST_FLOW_OK;
}

static void
gst_anaglyph_finalize (GObject * object)
{
  GstAnaglyph *mix = GST_ANAGLYPH (object);

  // FIXME
  gst_object_unref (mix->collect);
  g_mutex_free (mix->state_lock);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}
