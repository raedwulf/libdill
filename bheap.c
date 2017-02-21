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

#include "bheap.h"
#include "utils.h"

#define dill_bheap_isnil(x) ((x) == &dill_bheap_nil)
#define dill_bheap_isroot(x) ((x)->up->up == &dill_bheap_nil)

static struct dill_bheap dill_bheap_nil = { 0, 0 };

void dill_bheap_init(struct dill_bheap *self) {
    self->root = NULL;
    self->count = 0;
}

static int dill_bheap_traverse_next(struct dill_bheap *self, struct dill_bheap_item **parent) {
    if(dill_slow(!self->count)) return -1;
    int height = 31 - __builtin_clz(self->count + 1);
    int dir = (self->count + 1) << (32 - height);
    *parent = self->root;
    for(int i = 0; i < height - 1; i++, dir <<= 1)
        *parent = (dir & 0x80000000) ? (*parent)->right : (*parent)->left;
    return dir & 0x80000000;
}

static struct dill_bheap_item *dill_bheap_traverse_last(struct dill_bheap *self) {
    if(dill_slow(!self->count)) return NULL;
    int height = 31 - __builtin_clz(self->count);
    int dir = self->count << (32 - height);
    struct dill_bheap_item *node = self->root;
    for(int i = 0; i < height; i++, dir <<= 1)
        node = (dir & 0x80000000) ? node->right : node->left;
    return node;
}

static void dill_bheap_swap_left(struct dill_bheap_item *item) {
    struct dill_bheap_item *up, *left, *right;
    up = item->up;
    left = item->left;
    right = item->right;

    item->up = left;
    item->left = left->left;
    item->right = left->right;
    if(item->left) item->left->up = item;
    if(item->right) item->right->up = item;

    left->up = up;
    left->left = item;
    left->right = right;
    if(right) right->up = left;
    if(left->right) left->right->up = left;

    if(up) {
        if(up->left == item)
            up->left = left;
        else
            up->right = left;
    }
}

static void dill_bheap_swap_right(struct dill_bheap_item *item) {
    struct dill_bheap_item *up, *left, *right;
    up = item->up;
    left = item->left;
    right = item->right;

    item->up = right;
    item->left = right->left;
    item->right = right->right;
    if(item->left) item->left->up = item;
    if(item->right) item->right->up = item;

    right->up = up;
    right->left = left;
    right->right = item;
    if(left) left->up = right;
    if(right->left) right->left->up = right;

    if(up) {
        if(up->left == item)
            up->left = right;
        else
            up->right = right;
    }
}

static struct dill_bheap_item *dill_bheap_swap(struct dill_bheap_item *item) {
    if(item->up->left == item)
        dill_bheap_swap_left(item->up);
    else
        dill_bheap_swap_right(item->up);
}

static void dill_bheap_sift_up(struct dill_bheap *self, struct dill_bheap_item *item) {
    /* If item has no parent, there is nothing to do. */
    if(dill_slow(!item->up)) return;
    while(item->up && item->up->val > item->val)
        dill_bheap_swap(item);
    if(item->up == NULL) {self->root = item; return;}
}

static void dill_bheap_sift_down(struct dill_bheap *self, struct dill_bheap_item *item) {
    struct dill_bheap_item *next;
    /* If item has no children, there is nothing to do. */
    if(!item->left) return;
    while(1) {
        if(item->left && (item->left->val < item->val)) {
            next = item->left;
            dill_bheap_swap_left(item);
        } else if(item->right && (item->right->val < item->val)) {
            next = item->right;
            dill_bheap_swap_right(item);
        } else {
            if(item->left == NULL) {
                item->left = item->right;
                item->right = NULL;
            }
            break;
        }
        if(next->up == NULL) self->root = next;
    }
}

void dill_bheap_insert(struct dill_bheap *self, int64_t val,
    struct dill_bheap_item *item) {
    item->left = item->right = item->up = NULL;
    /* If this is the first item; add to root. */
    if(!self->root) {
        self->root = item;
        self->count = 1;
        item->val = val;
        return;
    }
    /* Get parent of new item. */
    struct dill_bheap_item *parent;
    int dir = dill_bheap_traverse_next(self, &parent);
    dill_assert(dir != -1);
    dill_assert(parent != NULL);
    if(dir) /* right */
        parent->right = item;
    else /* left */
        parent->left = item;
    /* Finalise new item; it's now a leaf with p as parent. */
    item->up = parent;
    item->val = val;
    self->count++;
    /* Sift new item up. */
    dill_bheap_sift_up(self, item);
}

void dill_bheap_erase(struct dill_bheap *self, struct dill_bheap_item *item) {
    /* Pop item at the bottom of the heap in list of leaves. */
    struct dill_bheap_item *p = dill_bheap_traverse_last(self);
    dill_assert(p != NULL);
    self->count--;
    /* Last item. */
    if(!self->count) {
        self->root = NULL;
        return;
    }

    if(item == p) {
        if(p->up) {
            if(p->up->left == p) {
                dill_assert(p->up->right == NULL);
                p->up->left = NULL;
            } else
                p->up->right = NULL;
        }
    } else if(item->left == p) {
        dill_bheap_swap_left(item);
        p->left = p->right;
        p->right = NULL;
    } else if(item->right == p) {
        dill_assert(item->left != NULL);
        dill_bheap_swap_right(item);
        p->right = NULL;
    } else {
        /* Remove p's parents' links to p. */
        if(p->up) {
            if(p->up->left == p) {
                p->up->left = NULL;
                dill_assert(p->up->right == NULL);
            } else {
                dill_assert(p->up->right == p);
                p->up->right = NULL;
            }
        }
        /* If popped item is the item to be erase; do nothing more. */
        if(p == item) {
            /* Update the root if changed. */
            if(p == self->root)
                self->root = NULL;
            return;
        }
        /* Insert the new item under the parent of item. */
        if(item->up) {
            if(item->up->left == item) {
                item->up->left = p;
            } else {
                dill_assert(item->up->right == item);
                item->up->right = p;
            }
        }
        /* Move internal links of item to p. */
        if(item->left) {
            item->left->up = p;
            if(item->right) item->right->up = p;
        } else {
            dill_assert(item->right == NULL);
        }
        /* Make p point to item's links. */
        p->up = item->up;
        p->left = item->left;
        p->right = item->right;
    }
    /* Clear pointers from deleted node. */
    item->up = item->left = item->right = NULL;
    /* Maintain heap invariant through sifting. */
    if(!p->up) self->root = p;
    if(p->up && p->val < p->up->val)
        dill_bheap_sift_up(self, p);
    else
        dill_bheap_sift_down(self, p);
}
