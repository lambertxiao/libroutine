#ifndef LIBROUTINE_COMMOM_H_
#define LIBROUTINE_COMMOM_H_

#include <stdint.h>
#include <sys/time.h>

static uint64_t get_time_ms() {
	struct timeval now = { 0 };
	gettimeofday( &now, nullptr );
	unsigned long long u = now.tv_sec;
	u *= 1000;
	u += now.tv_usec / 1000;
	return u;
}

#endif

