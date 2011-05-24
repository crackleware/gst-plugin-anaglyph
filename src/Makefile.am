plugin_LTLIBRARIES = libgstanaglyph.la

libgstanaglyph_la_SOURCES = gstplugin.c gstanaglyph.c gstanaglyph.h \
                            anaglyph.c anaglyph.h gstanaglyphpad.h                

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstanaglyph_la_CFLAGS = $(GST_CFLAGS)
libgstanaglyph_la_LIBADD = $(GST_LIBS)
libgstanaglyph_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstanaglyph_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstanaglyph.h anaglyph.h gstanaglyphpad.h
