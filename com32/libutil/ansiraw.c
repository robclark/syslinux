/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2004-2008 H. Peter Anvin - All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rights to use,
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall
 *   be included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------- */

/*
 * ansiraw.c
 *
 * Configures the console for ANSI output in raw mode; versions
 * for COM32 and Linux support.
 */

#ifdef __COM32__

#include <stdio.h>
#include <unistd.h>
#include <console.h>

void console_ansi_raw(void)
{
    openconsole(&dev_rawcon_r, &dev_ansiserial_w);
}

#else

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static struct termios original_termios_settings;

static void __attribute__ ((constructor)) console_init(void)
{
    tcgetattr(0, &original_termios_settings);
}

static void __attribute__ ((destructor)) console_cleanup(void)
{
    tcsetattr(0, TCSANOW, &original_termios_settings);
}

void console_ansi_raw(void)
{
    struct termios tio;

    /* Disable stdio buffering */
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* Set the termios flag so we behave the same as libcom32 */
    tcgetattr(0, &tio);
    tio.c_iflag &= ~ICRNL;
    tio.c_iflag |= IGNCR;
    tio.c_lflag &= ~(ISIG | ICANON | ECHO);
    if (!tio.c_oflag & OPOST)
	tio.c_oflag = 0;
    tio.c_oflag |= OPOST | ONLCR;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;	/* Don't 100% busy-wait in Linux */
    tcsetattr(0, TCSAFLUSH, &tio);
}

#if 1
#include <time.h>
#include <sys/time.h>
static int throttle(int fd)
{
	struct timeval timeout;
	fd_set read_fds, write_fds, except_fds;
	int rv;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(fd, &read_fds);

	/* set timeout to 0.1 seconds: */
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	return select(fd+1, &read_fds, &write_fds, &except_fds, &timeout);
}
#else
static int throttle(int fd) { return 0; }
#endif

int raw_read(int fd, void *buf, size_t count)
{
	struct termios tio, rtio;
	int rv;

	tcgetattr(fd, &tio);

	cfmakeraw(&rtio);
	tcsetattr(fd, 0, &rtio);

	rv = throttle(fd);
	if (rv <= 0)
		goto out;

	rv = read(fd, buf, count);

out:
	/* Restore settings */
	tcsetattr(fd, 0, &tio);

	return rv;
}

#endif
