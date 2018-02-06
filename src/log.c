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

#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

static int (*print)(const char*, ...);
static int (*error_print)(const char*, ...);

void utils_log_print( const char* format, ... ) {
    va_list args;
    va_start(args, format);
    if (print) print(format, args);
    else vfprintf(stdout, format, args);
    va_end(args);
}

void utils_log_error( const char* format, ... ) {
    va_list args;
    va_start(args, format);
    if (error_print) print(format, args);
    else vfprintf(stderr, format, args);
    va_end(args);
}

void utils_log_set_print(int (*new_print)(const char*, ...)){
	print = new_print;
}

void utils_log_set_error_print(int (*new_print)(const char*, ...)){
	error_print = new_print;
}
