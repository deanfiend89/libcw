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

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	tester.cpp \
	src/ashmem-dev.c \
	src/ipc.c \
	src/gralloc_tex.c \
	src/drawtex.c \
	src/log.c \
	src/buffer.c \
	src/window.c \
	src/EGLUtils.cpp \
	src/WindowSurface.cpp \
	src/libancillary/fd_send.c \
	src/libancillary/fd_recv.c

LOCAL_SHARED_LIBRARIES := libcutils libEGL libGLESv2 libui libgui libutils libhardware

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include $(LOCAL_PATH)/src $(LOCAL_PATH)/src/libancillary
LOCAL_MODULE:= tester

include $(BUILD_EXECUTABLE)
