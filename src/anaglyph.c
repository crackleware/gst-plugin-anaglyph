/*
 * Copyright (C) 2011 RidgeRun
 *
 */
#include "anaglyph.h"

/* Anaglyph main process */
void ana_process(guint8 *left,
                 guint8 *right,
                 guint8 *stereo,
                 guint size,
                 ana_mode mode)
{
  guint i;

  switch (mode)
  {
    case ANA_MONOCHROME:
	  for (i = 0; i < size ; i = i+3)
      { 
		/* Red Channel */  
        stereo[i] = (left[i]*299 + left[i+1]*587 + left[i+2]*114)/1000;
        /* Green channel */
        stereo[i+1] = (right[i]*299 + right[i+1]*587 + right[i+2]*114)/1000;
        /* Blue channel */
        stereo[i+2] = (right[i]*299 + right[i+1]*587 + right[i+2]*114)/1000; 		  
      }
	  
      break;
    case ANA_COLOR:
      for (i = 0 ; i < size ; i = i+3)
      {
		   
		/* Red Channel */  
        stereo[i] = left[i];
        /* Green channel */
        stereo[i+1] = right[i+1];
        /* Blue channel */
        stereo[i+2] = right[i+2]; 		  
      }
      break;
    case ANA_HALF_COLOR:
      for (i = 0; i < size ; i = i+3)
      {
		/* Red Channel */  
        stereo[i] = (left[i]*299 + left[i+1]*587 + left[i+2]*114)/1000;
		/* Green channel */
        stereo[i+1] = right[i+1];
        /* Blue channel */
        stereo[i+2] = right[i+2]; 		    
      }
      break;
    case ANA_OPTIMIZED:
      for (i = 0; i < size ; i = i+3)
      {
		/* Red Channel */  
        stereo[i] = (left[i+1]*450 + left[i+2]*150)/1000;
		/* Green channel */
        stereo[i+1] = right[i+1];
        /* Blue channel */
        stereo[i+2] = right[i+2]; 		    
      }
      break;
    case ANA_NONE_LEFT:
      for (i = 0; i < size ; ++i)
      {
		/* Copy all */  
        stereo[i] = left[i];	    
      }
      break;
    case (ANA_NONE_RIGHT):
      for (i = 0; i < size ; ++i)
      {
		/* Copy all */  
        stereo[i] = right[i];	    
      }
      break;          	  
  }	  	
}	                         
