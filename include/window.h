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
#include <EGLUtils.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define CW_API __attribute__((visibility("default")))

CW_API EGLNativeWindowType ACustomNativeWindow_create(int w, int h);
CW_API void ACustomNativeWindow_start_sharing(EGLNativeWindowType window);
CW_API int ACustomNativeWindow_frame_available(EGLNativeWindowType window);

CW_API void glBindNativeBufferToTex(EGLClientBuffer buffer);

CW_API void glDrawPixels(GLuint disp_id, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data);
CW_API void glDrawTex(GLuint disp_id);
CW_API GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

#ifdef __cplusplus
}
#endif
#endif // __WINDOW_H__
