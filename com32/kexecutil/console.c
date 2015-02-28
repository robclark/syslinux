#include <time.h>
#include <stdio.h>
#include <getkey.h>
#include <sys/ioctl.h>

void drain_keyboard(void)
{
#if 0 /* TODO */
    /* Prevent "ghost typing" and keyboard buffer snooping */
    volatile char junk;
    int rv;

    do {
	rv = read(0, (char *)&junk, 1);
    } while (rv > 0);

    junk = 0;

    cli();
    *(volatile uint8_t *)0x419 = 0;	/* Alt-XXX keyboard area */
    *(volatile uint16_t *)0x41a = 0x1e;	/* Keyboard buffer empty */
    *(volatile uint16_t *)0x41c = 0x1e;
    memset((void *)0x41e, 0, 32);	/* Clear the actual keyboard buffer */
    sti();
#endif
}

int shift_is_held(void)
{
    return 0;
}
int ctrl_is_held(void)
{
    return 0;
}

int getscreensize(int fd, int *rows, int *cols)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    fprintf(stdout, "lines %d\n", w.ws_row);
    fprintf(stdout, "columns %d\n", w.ws_col);

    *rows = w.ws_row;
    *cols = w.ws_col;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/* seems to use some custom escape codes, so hook into syslinux ansi stuff: */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "../lib/sys/ansi.h"

static FILE *logfd;

static void ansicon_erase(const struct term_state *st,
	int x0, int y0, int x1, int y1);
static void ansicon_showcursor(const struct term_state *st);
static void ansicon_set_cursor(int x, int y, bool visible);
static void ansicon_write_char(int x, int y, uint8_t ch,
	const struct term_state *st);
static void ansicon_scroll_up(const struct term_state *st);
static void ansicon_beep(void);

static struct term_state ts;
struct ansi_ops __ansicon_ops = {
    .erase = ansicon_erase,
    .write_char = ansicon_write_char,
    .showcursor = ansicon_showcursor,
    .set_cursor = ansicon_set_cursor,
    .scroll_up = ansicon_scroll_up,
    .beep = ansicon_beep,
};

static struct term_info ti = {
    .disabled = 0,
    .ts = &ts,
    .op = &__ansicon_ops
};


static void set_state(const struct term_state *st, int x, int y)
{
    fprintf(stdout, "\033[0;3%u;4%u", st->fg, st->bg);
    if (st->underline)
	fprintf(stdout, "4;");
    if (st->blink)
	fprintf(stdout, "5;");
    if (st->reverse)
	fprintf(stdout, "7;");
    fprintf(stdout, "m");
    fprintf(stdout, "\033[?7%c", st->autowrap ? 'h' : 'l');
    fprintf(stdout, "\033[20%c", st->autocr ? 'h' : 'l');
    fprintf(stdout, "\033[%d;%dH", y+1, x+1);

}

/* Erase a region of the screen */
static void ansicon_erase(const struct term_state *st,
	int x0, int y0, int x1, int y1)
{
    fprintf(logfd, "ERASE: {%d,%d-%d,%d}\n", x0, y0, x1, y1);
    if ((x1 == ti.cols - 1) && (y1 = ti.rows - 1)) {
	/* Clear screen from cursor down */
	set_state(st, x0, y0);
	fprintf(stdout, "\033[0J");
    } else if ((x0 == 0) && (y0 == 0)) {
	/* Clear screen from cursor down */
	set_state(st, x1, y1);
	fprintf(stdout, "\033[1J");
    } else if ((y0 == y1) && (x1 == ti.cols -1)) {
	/* Clear line from cursor right */
	set_state(st, x0, y0);
	fprintf(stdout, "\033[0K");
    } else if ((y0 == y1) && (x0 == 0)) {
	/* Clear line from cursor left */
	set_state(st, x1, y0);
	fprintf(stdout, "\033[1K");
    } else {
	fprintf(logfd, "ERROR!\n");
    }
}

/* Show or hide the cursor */
static void ansicon_showcursor(const struct term_state *st)
{
}

static void ansicon_set_cursor(int x, int y, bool visible)
{
}

#define ENTRY(code, val) [(code) - 0x80] = (val)
#if 0
static const const char *extended_utf8[0x80] = {
	ENTRY(179, "\342\224\202"),
	ENTRY(191, "\342\224\220"),
	ENTRY(192, "\342\224\224"),
	ENTRY(193, "\342\224\264"),
	ENTRY(194, "\342\224\254"),
	ENTRY(195, "\342\224\234"),
	ENTRY(196, "\342\224\200"),
	ENTRY(197, "\342\224\274"),
	ENTRY(217, "\342\224\230"),
	ENTRY(218, "\342\224\214"),
};
#endif

/* fbcon kinda sucks, so just translate special box chars into
 * closest ascii equiv's.. if we had kmscon we could do something
 * more clever with unicode chars instead.. :-/
 */
static const char *extended[0x80] = {
	ENTRY(179, "|"),
	ENTRY(180, "+"),
	ENTRY(191, "+"),
	ENTRY(192, "+"),
	ENTRY(195, "+"),
	ENTRY(196, "-"),
	ENTRY(217, "+"),
	ENTRY(218, "+"),
};

static void ansicon_write_char(int x, int y, uint8_t ch,
	const struct term_state *st)
{
    fprintf(logfd, "WRITE: {%d,%d: %c (%u)}\n", x, y, ch, ch);
    set_state(st, x, y);
    if (ch >= 0x80) {
	int idx = ch - 0x80;
	if (extended[idx])
	    fprintf(stdout, "%s", extended[idx]);
	else
	    fprintf(stdout, "?");
    } else {
	fprintf(stdout, "%c", ch);
    }
}

static void ansicon_scroll_up(const struct term_state *st)
{
}

static void ansicon_beep(void)
{
    fprintf(stdout, "\a");
}

int console_putchar(int c)
{
    __ansi_putchar(&ti, c);
    return 1;
}

int console_printf(const char *format, ...)
{
    char buf[4096];
    va_list ap;
    int i, rv;

    va_start(ap, format);
    rv = vsprintf(buf, format, ap);
    va_end(ap);

    for (i = 0; i < rv; i++)
	__ansi_putchar(&ti, buf[i]);

    return rv;
}

#undef fputs
int console_fputs(const char *s, FILE *file)
{
    if (file == stdout) {
	int i, len = strlen(s);
	for (i = 0; i < len; i++)
	    __ansi_putchar(&ti, s[i]);
	return len;
    } else {
	return fputs(s, file);
    }
}

static void __attribute__ ((constructor)) console_init(void)
{
    logfd = fopen("log.txt", "w");
    getscreensize(1, &ti.rows, &ti.cols);
    fprintf(logfd, "SCREEN: %dx%d\n", ti.rows, ti.cols);
    __ansi_init(&ti);
}

static void __attribute__ ((destructor)) console_cleanup(void)
{
    fclose(logfd);
}
//////////////////////////////////////////////////////////////////////////////

// TODO move?
char ConfigName[FILENAME_MAX];
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

int copy_string(char *dst, const char *src)
{
    int n = 0;
    while (src && (*src != '\0') && (*src != ' ')) {
	*(dst++) = *(src++);
	n++;
    }
    *dst = '\0';
    return n;
}

void execute(const char *cmdline, uint32_t type, bool sysappend)
{
    char kernel[512] = "";
    char initrd[512] = "";
    char fdtdir[512] = "";
    char append[2048] = "", *appendp = append;
    char buf[4096];
    char *dst = kernel;
    char *p = buf;
    int ret;
    /* TODO don't hard code these two: */
    const char *bootpath = "/boot";
    const char *fdtname = "qcom-apq8064-ifc6410";

    strncpy(buf, cmdline, sizeof(buf));

    p += copy_string(kernel, p);

    while (*p != '\0') {
	if (*p == ' ') {
	    const char *peek = (p + 1);
	    dst = appendp;
	    if (!peek) {
		*appendp = '\0';
	    } else if (strncmp(peek, "initrd=", 7) == 0) {
		p += copy_string(initrd, peek + 7) + 7;
		*(p++) = '\0';
		continue;
	    } else if (strncmp(peek, "fdtdir=", 7) == 0) {
		p += copy_string(fdtdir, peek + 7) + 7;
		*(p++) = '\0';
		continue;
	    }
	}
	appendp++;
	*(dst++) = *(p++);
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "cmdline:   %s\n", cmdline);
    fprintf(stdout, "type:      %u\n", type);
    fprintf(stdout, "sysappend: %u\n", sysappend);

    fprintf(stdout, "\n");
    fprintf(stdout, "kernel:    %s\n", kernel);
    fprintf(stdout, "initrd:    %s\n", initrd);
    fprintf(stdout, "fdtdir:    %s\n", fdtdir);
    fprintf(stdout, "append:    %s\n", append);

    p  = buf;
    p += sprintf(p, "kexec -l %s/%s", bootpath, kernel);
    p += sprintf(p, " --dtb=%s/%s/%s.dtb", bootpath, fdtdir, fdtname);
    p += sprintf(p, " --ramdisk=%s/%s", bootpath, initrd);
    p += sprintf(p, " --append=\"%s\"", append);

    fprintf(stdout, "%s\n", buf);

    ret = system(buf);
    fprintf(stdout, "ret=%d\n", ret);
    if (!ret)
	ret = system("kexec -e");
    fprintf(stdout, "2 ret=%d\n", ret);

    exit(0);
}
