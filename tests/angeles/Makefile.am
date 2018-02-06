AM_CFLAGS = -I$(top_srcdir)/include
AM_CXXFLAGS = $(AM_CFLAGS)
bin_PROGRAMS = angeles

angeles_SOURCES = app-linux.cpp demo.c
angeles_LDADD = $(top_srcdir)/src/libcw.la -lEGL -lGLESv1_CM
