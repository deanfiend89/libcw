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

#include <stdlib.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <utils.h>

const char vertex_src [] =
	"attribute vec4 position;\n"
	"attribute vec4 texcoords;\n"
	"varying vec2 textureCoordinate;\n"

	"void main()\n"
	"{\n"
	"	gl_Position = position;\n"
	"	textureCoordinate = texcoords.xy;\n"
	"}\n";

const char fragment_src [] =
	"varying highp vec2 textureCoordinate;\n"
	"uniform sampler2D texture;\n"

	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(texture, textureCoordinate);\n"
	"}\n";

const char fragment_src_bgra [] =
	"varying highp vec2 textureCoordinate;\n"
	"uniform sampler2D texture;\n"

	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(texture, textureCoordinate).bgra;\n"
	"}\n";

const float vertexArray[] = {
	0.0,  1.0,  0.0,
	-1.,  0.0,  0.0,
	0.0, -1.0,  0.0,
	1.,  0.0,  0.0,
	0.0,  1.,  0.0
};

const GLfloat squareVertices[] = {
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f,  1.0f,
	1.0f,  1.0f,
};

const GLfloat textureVertices[] = {
	1.0f, 1.0f,
	0.0f,  1.0f,
	1.0f, 0.0f,
	0.0f,  0.0f,
};

static GLuint loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    utils_log_error("Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader); checkGlError("glAttachShader");
        glAttachShader(program, pixelShader); checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    utils_log_error("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint shaderProgram;

GLint position_loc;
GLint texcoords_loc;
GLint texture_loc;

void shader_setup(void){
	shaderProgram = createProgram(vertex_src, fragment_src);

	position_loc  = glGetAttribLocation  ( shaderProgram , "position" ); checkGlError("glGetAttribLocation");
	texcoords_loc = glGetAttribLocation  ( shaderProgram , "texcoords" ); checkGlError("glGetAttribLocation");
	texture_loc = glGetUniformLocation ( shaderProgram , "texture" ); checkGlError("glGetUniformLocation");

	if ( position_loc < 0  ||  texcoords_loc < 0 || texture_loc < 0 ) {
		utils_log_error("EGLRenderer_Init: failed to get shader variables locations\n");
	}
}

void glDrawTex(){
	if (!shaderProgram) {
		shader_setup();
	}

    glUseProgram(shaderProgram); checkGlError("glUseProgram");

    glUniform1i(texture_loc, 0); checkGlError("glUniform1i");

    glVertexAttribPointer(position_loc, 2, GL_FLOAT, 0, 0, squareVertices); checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(position_loc); checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(texcoords_loc, 2, GL_FLOAT, 0, 0, textureVertices); checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(texcoords_loc); checkGlError("glEnableVertexAttribArray");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); checkGlError("glDrawArrays");

    glDisableVertexAttribArray(position_loc); checkGlError("glDisableVertexAttribArray");
    glDisableVertexAttribArray(texcoords_loc); checkGlError("glDisableVertexAttribArray");
}
