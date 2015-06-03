#ifndef REALTIME_H
#define REALTIME_H
#include <stdint.h>
#include"stdio.h"
#include <unistd.h>	/* POSIX flags */
#include <time.h>	/* clock_gettime(), time() */
#include <sys/time.h>	/* gethrtime(), gettimeofday() */

/* get current time in second */
inline uint32_t getSTime() {
   	return time(0);
}

/* get current time in millisecond*/
inline uint64_t getMTime() {
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return (uint64_t) tm.tv_sec*1000 + (uint64_t) tm.tv_usec/1000;
}

/* get current time in usecond */
inline uint64_t getUTime() {
    struct timeval tm;
    gettimeofday(&tm, NULL);
    return (uint64_t) tm.tv_sec * 1000000 + tm.tv_usec;
}

/* convert time to string */
/* vietnamese: offset = 7 */
inline void VNStringTime(time_t time, char* stringTime, int offset) {
    time += offset * 60 * 60;
    struct tm tmT = *gmtime(&time);
    sprintf(stringTime, "%3.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tmT.tm_year + 1900, tmT.tm_mon + 1, tmT.tm_mday, tmT.tm_hour, tmT.tm_min, tmT.tm_sec);
}

#endif