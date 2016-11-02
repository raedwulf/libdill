/*

  Copyright (c) 2016 Martin Sustrik
  Copyright (c) 2016 Tai Chi Minh Ralph Eastwood

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
#include <stdio.h>
#include <unistd.h>

#include "../libdill.h"

int main() {
    int ac, am, ap, ao;
    size_t sz;
    void *p, *pa[8];
    int rc;
    size_t pgsz = sysconf(_SC_PAGE_SIZE);

    am = amalloc(512, DILL_ALLOC_FLAGS_DEFAULT);
    assert(am != -1);
    assert(asize(am) == 512);
    assert(!(acaps(am) & DILL_ALLOC_CAPS_ZERO));
    assert(!(acaps(am) & DILL_ALLOC_CAPS_ALIGNED));
    assert(!(acaps(am) & DILL_ALLOC_CAPS_BOOKKEEP));
    assert(acaps(am) & DILL_ALLOC_CAPS_RESIZE);
    sz = 512;
    p = aalloc(am, &sz);
    assert(sz == 512);
    assert(p != NULL);
    rc = afree(am, p);
    assert(rc == 0);
    sz = 768;
    p = aalloc(am, &sz);
    assert(sz == 768);
    assert(p != NULL);
    rc = afree(am, p);
    assert(rc == 0);
    hclose(am);
    am = amalloc(512, DILL_ALLOC_FLAGS_HUGE);
    assert(am == -1);
    assert(errno == ENOTSUP);
    errno = 0;
    am = amalloc(512, DILL_ALLOC_FLAGS_GUARD);
    assert(am == -1);
    assert(errno == ENOTSUP);
    am = amalloc(512, DILL_ALLOC_FLAGS_DEFAULT);
    hclose(am);

    ap = apage(1, DILL_ALLOC_FLAGS_DEFAULT);
    assert(ap != -1);
    assert(asize(ap) == pgsz);
    assert(!(acaps(ap) & DILL_ALLOC_CAPS_ZERO));
    assert(acaps(ap) & DILL_ALLOC_CAPS_ALIGNED);
    assert(!(acaps(ap) & DILL_ALLOC_CAPS_BOOKKEEP));
    assert(acaps(ap) & DILL_ALLOC_CAPS_RESIZE);
    sz = 512;
    p = aalloc(ap, &sz);
    assert(sz == pgsz);
    assert(p != NULL);
    rc = afree(ap, p);
    assert(rc == 0);
    sz = pgsz + 42;
    p = aalloc(ap, &sz);
    assert(sz == pgsz * 2);
    assert(p != NULL);
    rc = afree(ap, p);
    assert(rc == 0);
    sz = pgsz * 4;
    p = aalloc(ap, &sz);
    assert(sz == pgsz * 4);
    assert(p != NULL);
    rc = afree(ap, p);
    assert(rc == 0);
    hclose(ap);
    ap = apage(512, DILL_ALLOC_FLAGS_HUGE);
    assert(ap == -1);
    assert(errno == ENOTSUP);
    errno = 0;
#if HAVE_MPROTECT
    ap = apage(512, DILL_ALLOC_FLAGS_GUARD);
    assert(ap == 0);
    hclose(ap);
#else
    ap = apage(512, DILL_ALLOC_FLAGS_GUARD);
    assert(ap == -1);
#endif

    am = amalloc(4096, DILL_ALLOC_FLAGS_ZERO);
    assert(am != -1);
    assert(asize(am) == 4096);
    assert(acaps(am) & DILL_ALLOC_CAPS_ZERO);
    assert(!(acaps(am) & DILL_ALLOC_CAPS_ALIGNED));
    assert(!(acaps(am) & DILL_ALLOC_CAPS_BOOKKEEP));
    assert(acaps(am) & DILL_ALLOC_CAPS_RESIZE);
    p = aalloc(am, &sz);
    while(sz)
        assert(((char *)p)[sz--] == 0);
    afree(am, p);
    ao = apool(am, DILL_ALLOC_FLAGS_DEFAULT, 8, 2);
    assert(ao != -1);
    assert(asize(ao) == 8);
    assert(!(acaps(ao) & DILL_ALLOC_CAPS_ZERO));
    assert(!(acaps(ao) & DILL_ALLOC_CAPS_ALIGNED));
    assert(!(acaps(ao) & DILL_ALLOC_CAPS_BOOKKEEP));
    assert(!(acaps(ao) & DILL_ALLOC_CAPS_RESIZE));
    sz = 8;
    p = aalloc(ao, &sz);
    assert(sz == 8);
    assert(p != NULL);
    rc = afree(ao, p);
    assert(rc == 0);
    sz = 15;
    p = aalloc(ao, &sz);
    assert(p == NULL);
    sz = 8;
    p = aalloc(ao, &sz);
    assert(sz == 8);
    assert(p != NULL);
    rc = afree(ao, p);
    assert(rc == 0);
    sz = 8;
    p = aalloc(ao, &sz);
    assert(sz == 8);
    assert(p != NULL);
    rc = afree(ao, p);
    assert(rc == 0);
    hclose(ao);
    hclose(am);

    return 0;
}

