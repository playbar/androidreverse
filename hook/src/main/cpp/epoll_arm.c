/*
 *  Collin's Binary Instrumentation Tool/Framework for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *  http://www.mulliner.org/android/
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>

extern int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
extern int gettimeofday_new(struct timeval *tv, struct timezone *tz);
extern int clock_gettime_new(clockid_t clk_id, struct timespec *tp);

int my_epoll_wait_arm(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return my_epoll_wait(epfd, events, maxevents, timeout);
}

int gettimeofday_new_arm(struct timeval *tv, struct timezone *tz) {
	return gettimeofday_new(tv, tz);
}

//int clock_gettime_new_arm(clockid_t clk_id, struct timespec *tp) {
//	return clock_gettime_new(clk_id, tp);
//}
