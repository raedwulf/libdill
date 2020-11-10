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

#ifndef DILL_NOW_INCLUDED
#define DILL_NOW_INCLUDED

#include <stdint.h>

#if defined __APPLE__
#include <mach/mach_time.h>
#endif

#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#endif

struct dill_ctx_now {
#if defined __APPLE__
    mach_timebase_info_data_t mtid;
#endif
#if defined(__x86_64__) || defined(__i386__)
    int64_t last_time;
    uint64_t last_tsc;
#endif
};

int dill_ctx_now_init(struct dill_ctx_now *ctx);
void dill_ctx_now_term(struct dill_ctx_now *ctx);

/* Same as dill_now() except that it doesn't use the context.
   I.e. it can be called before calling dill_ctx_now_init(). */
int64_t dill_mnow(void);

/* Same as dill_now() except that context is passed as a parameter */
static int64_t dill_ctx_now(struct dill_ctx_now *ctx) {
#if defined(__x86_64__) || defined(__i386__)
    /* On x86 platforms, rdtsc instruction can be used to quickly check time
       in form of CPU cycles. If less than 1M cycles have elapsed since the
       last dill_now_() call we assume it's still the same millisecond and return
       cached time. This optimization can give a huge speedup with old systems.
       1M number is chosen is such a way that it results in getting time every
       millisecond on 1GHz processors. On faster processors we'll query time
       somewhat more often but the number of queries should still be
       statistically insignificant. On slower processors we'll start losing
       precision, e.g. on 500MHz processor we can diverge by 1ms. */
    uint64_t tsc = __rdtsc();
    int64_t diff = tsc - ctx->last_tsc;
    if(diff < 0) diff = -diff;
    if(dill_fast(diff < 1000000ULL)) return ctx->last_time;
    ctx->last_tsc = tsc;
    ctx->last_time = dill_mnow();
    return ctx->last_time;
#else
    return dill_mnow();
#endif
}

#endif
