/*

  Copyright (c) 2016 Martin Sustrik

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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "libdill.h"
#include "utils.h"

/* Returns smallest value greater than val that is a multiple of unit. */
#define dill_align(val, unit) ((val) % (unit) ?\
    (val) + (unit) - (val) % (unit) : (val))

#if __STDC_VERSION__ >= 201112L
#define dill_memalign(o, s) aligned_alloc(o->pgsz, s)
#elif _POSIX_C_SOURCE >= 200112L
#define dill_memalign(o, s) ({\
    void *m = NULL;\
    int rc = posix_memalign(&m, o->pgsz, s);\
    if(dill_slow(rc != 0)) {\
        errno = rc;\
        m = NULL;\
    }\
    m;\
})
#elif defined HAVE_MALLOC_H
#include <malloc.h>
#if defined HAVE_VALLOC
#define dill_memalign(o, s) valloc(o->pgsz, s)
#elif HAVE_MEMALIGN
#define dill_memalign(o, s) memalign(o->pgsz, s)
#elif defined HAVE_PVALLOC
#define dill_memalign(o, s) pvalloc(o->pgsz, s)
#else
#endif
#endif

#ifndef dill_memalign
#error "Target system does not have posix_memalign compatible function!"
#endif

dill_unique_id(apage_type);

struct apage_alloc {
    struct hvfs hvfs;
    struct alloc_vfs avfs;
    int flags;
    size_t sz, pgsz;
};

#define DEFAULT_SIZE (256 * 1024)

static inline void *apage_hquery(struct hvfs *hvfs, const void *type);
static void apage_hclose(struct hvfs *hvfs);
static void *apage_alloc(struct alloc_vfs *avfs, size_t *sz);
static int apage_free(struct alloc_vfs *avfs, void *p);
static ssize_t apage_size(struct alloc_vfs *avfs);
static int apage_caps(struct alloc_vfs *avfs);

/* Get memory page size. The query is done once only. The value is cached. */
static size_t dill_page_size(void) {
    static long pgsz = 0;
    if(dill_fast(pgsz))
        return (size_t)pgsz;
    pgsz = sysconf(_SC_PAGE_SIZE);
    dill_assert(pgsz > 0);
    return (size_t)pgsz;
}

static inline void *apage_hquery(struct hvfs *hvfs, const void *type) {
    struct apage_alloc *obj = (struct apage_alloc *)hvfs;
    if(type == alloc_type) return &obj->avfs;
    if(type == apage_type) return obj;
    errno = ENOTSUP;
    return NULL;
}

int apage(int flags, size_t sz) {
#if !HAVE_MPROTECT
    if(flags & DILL_ALLOC_FLAGS_GUARD) {errno = ENOTSUP; return -1;}
#endif
    if(flags & DILL_ALLOC_FLAGS_HUGE) {errno = ENOTSUP; return -1;}
    struct apage_alloc *obj = malloc(sizeof(struct apage_alloc));
    if(dill_slow(!obj)) {errno = ENOMEM; return -1;}
    obj->hvfs.query = apage_hquery;
    obj->hvfs.close = apage_hclose;
    obj->avfs.alloc = apage_alloc;
    obj->avfs.free = apage_free;
    obj->avfs.size = apage_size;
    obj->avfs.caps = apage_caps;
    obj->flags = flags;
    obj->pgsz = sysconf(_SC_PAGESIZE);
    obj->sz = sz ? sz : DEFAULT_SIZE;
    obj->sz = dill_align(obj->sz, obj->pgsz);
    /* Create the handle. */
    int h = hcreate(&obj->hvfs);
    if(dill_slow(h < 0)) {
        int err = errno;
        free(obj);
        errno = err;
        return -1;
    }
    return h;
}

static void *apage_alloc(struct alloc_vfs *avfs, size_t *sz) {
    struct apage_alloc *obj =
        dill_cont(avfs, struct apage_alloc, avfs);
    /* Try use *sz first (if valid), if not use default & return through *sz */
    size_t size = sz ? (*sz ? dill_align(*sz, obj->pgsz) : (*sz = obj->sz)) : obj->sz;
    if(sz) *sz = size;
    /* Allocate the stack so that it's memory-page-aligned.
       Add one page as stack overflow guard. */
#if HAVE_MPROTECT
    /* With the stack guard, this function will always add more memory
       than requested.  Always use this value to calculate the top of the
       stack, if this is used directly instead of pool or a cache wrappers. */
    if(obj->flags & DILL_ALLOC_FLAGS_GUARD) {
        size += obj->pgsz;
        if(sz) *sz += obj->pgsz;
    }
#endif
    void *m = dill_memalign(obj, size);
#if HAVE_MPROTECT
    /* The bottom page is used as a stack guard. This way stack overflow will
       cause segfault rather than randomly overwrite the heap. */
    if(obj->flags & DILL_ALLOC_FLAGS_GUARD) {
        int rc = mprotect(m, obj->pgsz, PROT_NONE);
        if(dill_slow(rc != 0)) {
            free(m);
            errno = rc;
            return NULL;
        }
    }
#endif
    if(dill_slow(!m)) return NULL;
    if(obj->flags & DILL_ALLOC_FLAGS_ZERO) memset(m, 0, size);
    return m;
}

static int apage_free(struct alloc_vfs *avfs, void *p) {
    free(p);
    return 0;
}

static ssize_t apage_size(struct alloc_vfs *avfs) {
    struct apage_alloc *obj =
        dill_cont(avfs, struct apage_alloc, avfs);
    return obj->sz;
}

static int apage_caps(struct alloc_vfs *avfs) {
    struct apage_alloc *obj =
        dill_cont(avfs, struct apage_alloc, avfs);
    int caps = DILL_ALLOC_CAPS_RESIZE | DILL_ALLOC_CAPS_ALIGNED;
    if(obj->flags & (DILL_ALLOC_FLAGS_ZERO)) caps |= DILL_ALLOC_CAPS_ZERO;
    return caps;
}

static void apage_hclose(struct hvfs *hvfs) {
    struct apage_alloc *obj = (struct apage_alloc *)hvfs;
    /* This won't free memory that is allocated because it does not
       expose DILL_CAPS_BOOKKEEP capability */
    free(obj);
}
