#
#   This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
#  USA.
# 
#  Project home page: http://github.com/twaik/libcw
#

# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.
AM_CFLAGS = -Iinclude -I$(top_srcdir)/include -fvisibility=hidden
AM_CXXFLAGS = $(AM_CFLAGS)

#$(error top_srcdir is $(top_srcdir) srcdir is $(srcdir))

lib_LTLIBRARIES = libcw.la
libcw_la_LDFLAGS = -module -avoid-version -shared --disable-static -L$(abspath $(srcdir))/.libs -lEGL -lGLESv2
libcw_la_LIBADD = libhardware.la

libcw_la_SOURCES = 	\
	ashmem-dev.c \
	ipc.c \
	gralloc_tex.c \
	drawtex.c \
	log.c \
	buffer.c \
	window.c \
	input.c \
	EGLUtils.cpp \
	libancillary/fd_send.c \
	libancillary/fd_recv.c \
	app-android.c


#That is just a stub library and it is needed only to resolve linking problems. The real one is already placed on a target device
libhardware_la_LTLIBRARIES = libhardware.la
libhardware_ladir = $(abspath $(srcdir))
libhardware_la_LDFLAGS = -module -avoid-version -shared --disable-static
libhardware_la_SOURCES = hardware_stub.c
