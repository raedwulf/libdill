/*

  Copyright (c) 2015 Martin Sustrik

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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "../libdill.h"

static int64_t start = 0;
static int64_t stop = 0;

static coroutine void worker(int64_t nw, int i, int count) {
    if(i == 0) {
	    msleep(nw + 999);
	    start = now();
    } else if(i == count - 1) {
	    msleep(nw + 1001);
	    stop = now();
    } else msleep(nw + 1000);
    printf("%d\n", i);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("usage: go <coroutines>\n");
        return 1;
    }
    long count = atol(argv[1]);

    long i;
    int64_t nw = now();
    for(i = 0; i != count; ++i) {
        int h = go(worker(nw, i, count));
    }
    sleep(1);
    //msleep(nw + 1002);
    yield();
    yield();

    long duration = (long)(stop - start);
    long ms = duration / count;

    printf("executed %ld coroutines in %f seconds\n",
        (long)(count), ((float)duration) / 1000);
    printf("duration of timers in queue: %ld ms\n", ms);

    return 0;
}

