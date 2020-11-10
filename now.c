/*

  Copyright (c) 2017 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "ctx.h"

int64_t dill_mnow(void) {

/* Implementation using Mach timers. */
#if defined __APPLE__
    static mach_timebase_info_data_t dill_mtid = {0};
    if (dill_slow(!dill_mtid.denom))
        mach_timebase_info(&dill_mtid);
    uint64_t ticks = mach_absolute_time();
    return (int64_t)(ticks * dill_mtid.numer / dill_mtid.denom / 1000000);
#else

/* Implementation using clock_gettime(). */
#if defined CLOCK_MONOTONIC_COARSE
    clock_t id = CLOCK_MONOTONIC_COARSE;
#elif defined CLOCK_MONOTONIC_FAST
    clock_t id = CLOCK_MONOTONIC_FAST;
#elif defined CLOCK_MONOTONIC
    clock_t id = CLOCK_MONOTONIC;
#else
#define DILL_dill_now_FALLBACK
#endif
#if !defined DILL_dill_now_FALLBACK
    struct timespec ts;
    int rc = clock_gettime(id, &ts);
    dill_assert (rc == 0);
    return ((int64_t)ts.tv_sec) * 1000 + (((int64_t)ts.tv_nsec) / 1000000);

/* Implementation using gettimeofday(). This is slow and error-prone
   (the time can jump backwards!), but it's just a last resort option. */
#else
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    dill_assert (rc == 0);
    return ((int64_t)tv.tv_sec) * 1000 + (((int64_t)tv.tv_usec) / 1000);
#endif

#endif
}

int64_t dill_now(void) {
    return dill_ctx_now(&dill_getctx->now);
}

int dill_ctx_now_init(struct dill_ctx_now *ctx) {
#if defined __APPLE__
    mach_timebase_info(&ctx->mtid);
#endif
#if defined(__x86_64__) || defined(__i386__)
    ctx->last_time = dill_mnow();
    ctx->last_tsc = __rdtsc();
#endif
    return 0;
}

void dill_ctx_now_term(struct dill_ctx_now *ctx) {
}

