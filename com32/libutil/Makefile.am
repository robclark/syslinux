#
#  Copyright (c) 2015 Rob Clark <robdclark@gmail.com>
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

libutil_LIBRARIES  = libutil.a
libutildir         = $(libdir)
libutil_a_CFLAGS   = \
	-O0 -g \
	-include $(top_srcdir)/override.h \
	$(WARN_CFLAGS) \
	-I$(top_srcdir)/libutil/include \
	-I$(top_srcdir)/include
libutil_a_LDFLAGS  = -static

libutil_a_SOURCES  = \
	ansiline.c \
	ansiraw.c \
	base64.c \
	crypt-md5.c \
	keyname.c \
	md5.c \
	quicksort.c \
	sha1hash.c \
	sha256crypt.c \
	sha512crypt.c \
	unbase64.c
