/*
 * Copyright (C) 2011 RidgeRun
 *
 */

#ifndef __ANAGLYPH_H__
#define __ANAGLYPH_H__

#include <glib.h>

/**
 * Type of anaglyph image to produce.
 */
typedef enum _ana_mode
{
  /** Monochromic anaglyph image. */
  ANA_MONOCHROME = 0,
  
  /** The simplest color anaglyph image. */
  ANA_COLOR,
  
  /** Anaglyph "half-color" technique. */
  ANA_HALF_COLOR,
  
  /** Retine optimized anaglyph image production */
  ANA_OPTIMIZED,
  
  /** 
   * No stereo image produced. It uses left channel 
   * by default. To specify the channel use ANA_NONE_LEFT
   * or ANA_NONE_RIGHT instead.
   */ 	
  ANA_NONE,
  
  /** No anaglyph produced, passing left channel. (Same as ANA_NONE) */
  ANA_NONE_LEFT =  ANA_NONE,
  
  /** No anaglyph produced, passing right channel. */
  ANA_NONE_RIGHT
} ana_mode;	
	

/**
 * Process to produce the anaglyph image.
 * \param left Buffer corresponding to the left channel.
 * \param right Buffer corresponding to the right channel.
 * \param stereo Allocated buffer in which the stereo image will be written.
 * \param size Size in bytes of the buffers.
 * \param mode Anaglyph mode. See #ana_mode.
 */
extern void ana_process(guint8 *left,
                        guint8 *right,
                        guint8 *stereo,
                        guint size,
                        ana_mode mode);  
#endif /* __ANAGLYPH_H__ */ 
