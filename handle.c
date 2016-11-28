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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cr.h"
#include "libdill.h"
#include "utils.h"
#include "context.h"

struct dill_handle {
    /* Table of virtual functions. */
    struct hvfs *vfs;
    /* Number of references to this handle. */
    int refcount;
    /* Index of the next handle in the linked list of unused handles. -1 means
       'end of the list'. -2 means 'active handle'. */
    int next;
};

#define CHECKHANDLE(h, err) \
    if(dill_slow((h) < 0 || (h) >= ctx->nhandles ||\
          ctx->handles[(h)].next != -2)) {\
        errno = EBADF; return (err);}\
    struct dill_handle *hndl = &ctx->handles[(h)];

struct dill_ctx_handle {
    struct dill_handle *handles;
    int nhandles;
    int unused;
};

struct dill_ctx_handle dill_ctx_handle_defaults = {NULL};
struct dill_ctx_handle dill_ctx_handle_main_data = {NULL};
struct dill_ctx_handle *dill_ctx_handle_main = &dill_ctx_handle_main_data;

#if defined DILL_VALGRIND

static void dill_handle_atexit(void) {
    struct dill_ctx_handle *ctx = dill_context.handle;
    if(ctx->handles)
        free(ctx->handles);
}

#endif

/* Intialisation function for handle context. */
int dill_inithandle(void) {
    struct dill_ctx_handle *ctx = malloc(sizeof(struct dill_ctx_handle));
    if(dill_slow(!ctx)) return -1;
    memcpy(ctx, &dill_ctx_handle_defaults, sizeof(struct dill_ctx_handle));
    dill_context.handle = ctx;
    return 0;
}

/* Termination function for handle context. */
void dill_termhandle(void) {
    /* Ensure that we are not in the main thread. */
    if(dill_context.handle && dill_context.handle != dill_ctx_handle_main) {
        free(dill_context.handle);
        dill_context.handle = NULL;
    }
}

int hcreate(struct hvfs *vfs) {
    struct dill_ctx_handle *ctx = dill_context.handle;
    if(dill_slow(!vfs || !vfs->query || !vfs->close)) {
        errno = EINVAL; return -1;}
    /* Return ECANCELED if shutting down. */
    int rc = dill_canblock();
    if(dill_slow(rc < 0)) return -1;
    /* If there's no space for the new handle expand the array. */
    if(dill_slow(ctx->unused == 0)) {
        /* Start with 256 handles, double the size when needed. */
        int sz = ctx->nhandles ? ctx->nhandles * 2 : 256;
        struct dill_handle *hndls =
            realloc(ctx->handles, sz * sizeof(struct dill_handle));
        if(dill_slow(!hndls)) {errno = ENOMEM; return -1;}
#if defined DILL_VALGRIND
        /* Clean-up function to delete the array at exit. It is not strictly
           necessary but valgrind will be happy about it. */
        static int initialized = 0;
        if(dill_slow(!initialized)) {
            int rc = atexit(dill_handle_atexit);
            dill_assert(rc == 0);
            initialized = 1;
        }
#endif
        /* Add newly allocated handles to the list of unused handles. */
        int i;
        for(i = ctx->nhandles; i != sz - 1; ++i)
            hndls[i].next = i + 1;
        hndls[sz - 1].next = -1;
        ctx->unused = ctx->nhandles + 1;
        /* Adjust the array. */
        ctx->handles = hndls;
        ctx->nhandles = sz;
    }
    /* Return first handle from the list of unused hadles. */
    int h = ctx->unused - 1;
    ctx->unused = ctx->handles[h].next + 1;
    ctx->handles[h].vfs = vfs;
    ctx->handles[h].refcount = 1;
    ctx->handles[h].next = -2;
    return h;
}

int hdup(int h) {
    struct dill_ctx_handle *ctx = dill_context.handle;
    CHECKHANDLE(h, -1);
    ++hndl->refcount;
    return h;
}

void *hquery(int h, const void *type) {
    struct dill_ctx_handle *ctx = dill_context.handle;
    CHECKHANDLE(h, NULL);
    return hndl->vfs->query(hndl->vfs, type);
}

int hclose(int h) {
    struct dill_ctx_handle *ctx = dill_context.handle;
    CHECKHANDLE(h, -1);
    /* If there are multiple duplicates of this handle just remove one
       reference. */
    if(hndl->refcount > 1) {
        --hndl->refcount;
        return 0;
    }
    /* This will guarantee that blocking functions cannot be called anywhere
       inside the context of the close. */
    int old = dill_no_blocking2(1);
    /* Send stop signal to the handle. */
    hndl->vfs->close(hndl->vfs);
    /* Restore the previous state. */
    dill_no_blocking2(old);
    /* Return the handle to the shared pool. */
    hndl->next = ctx->unused - 1;
    ctx->unused = h + 1;
    return 0;
}

