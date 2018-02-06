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

//#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include "utils.h"

EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
							  EGLenum target, EGLClientBuffer buffer,
							  const EGLint *attrib_list);
void glBindNativeBufferToTex(EGLClientBuffer buffer){
	// Create the EGLImageKHR from the native buffer
	EGLImageKHR img = eglCreateImageKHR(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_CONTEXT,
										EGL_NATIVE_BUFFER_ANDROID,
										buffer,
										NULL);
	 checkGlError("eglCreateImageKHR");
    if (img == EGL_NO_IMAGE_KHR) {
		utils_log_error("Did not allocate EGLImage\n");
        return;
    }

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)img);
    checkGlError("glEGLImageTargetTexture2DOES");
}
