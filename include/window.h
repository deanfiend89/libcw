/*
 *  This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: http://github.com/twaik/libcw
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void* ACustomNativeWindow_getListener(char* path);
int ACustomNativeWindow_frame_available(void *listener_ptr);
void ACustomNativeWindow_releaseListener(void *listener_ptr);

EGLNativeWindowType ACustomNativeWindow_create(int w, int h);
void ACustomNativeWindow_start_sharing(EGLNativeWindowType window);

void glBindNativeBufferToTex(EGLClientBuffer buffer);
void glDrawTex();

//Errors
#define CW_ERROR_PROCESS_DIED -10
#define CW_ERROR_INPUT_UNIMPLEMENTED -11

#ifdef __cplusplus
}
#endif
#endif // __WINDOW_H__
