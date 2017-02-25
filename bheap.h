/*

  Copyright (c) 2017 Tai Chi Minh Ralph Eastwood

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

#ifndef DILL_BHEAP_INCLUDED
#define DILL_BHEAP_INCLUDED

#include <stdint.h>

struct dill_bheap_item {
    union {
        struct {
            struct dill_bheap_item *left;
            struct dill_bheap_item *right;
        };
        struct dill_bheap_item *child[2];
    };
    struct dill_bheap_item *up;
    int64_t val;

};

struct dill_bheap {
    struct dill_bheap_item *root;
    int count;
};

/* Initialize the binary heap. O(1) */
void dill_bheap_init(struct dill_bheap *self);

/* Returns 1 if there are no items in the binary heap. 0 otherwise. O(1) */
static int dill_bheap_empty(struct dill_bheap *self) {
    return !self->root;
}

/* Insert item into the binary heap. Set its value to 'val'.  Returns item index. O(log N) */
void dill_bheap_insert(struct dill_bheap *self, int64_t val, struct dill_bheap_item *item);

/* Remove an item from a heap. O(log N) */
void dill_bheap_erase(struct dill_bheap *self, struct dill_bheap_item *item);

/* Return item with the lowest value. If there are no items in the tree NULL
   is returned. O(1)*/
static struct dill_bheap_item *dill_bheap_first(struct dill_bheap *self) {
    return self->root;
}

#endif
