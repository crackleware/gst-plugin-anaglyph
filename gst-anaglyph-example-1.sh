#!/bin/sh

gst-launch-0.10 -v \
	filesrc location=left.png ! pngdec name=pd1 \
	filesrc location=right.png ! pngdec name=pd2 \
	pd1.src ! ana.sink_0 \
	pd2.src ! ana.sink_1 \
	anaglyph name=ana mode=1 ! pngenc ! filesink location=stereo.png

