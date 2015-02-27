/*
 * sys/times.h
 */

#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

struct tms {
    /* Empty */
};

#define HZ		1000
#define CLK_TCK		HZ

static inline clock_t times(struct tms *buf)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (1000 * tv.tv_sec) + (tv.tv_usec / 1000);
}

#endif /* _SYS_TIMES_H */
