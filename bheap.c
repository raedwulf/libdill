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

#define dill_bheap_traverse(h,n,d)\
    switch((h)) {\
        case 31: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 30: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 29: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 28: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 27: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 26: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 25: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 24: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 23: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 22: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 21: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 20: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 19: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 18: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 17: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 16: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 15: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 14: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 13: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 12: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 11: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case 10: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  9: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  8: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  7: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  6: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  5: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  4: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  3: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  2: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
        case  1: n = (n)->child[!!((d) & 0x80000000)]; d <<= 1;\
    }

void dill_bheap_init(struct dill_bheap *self) {
    self->root = NULL;
    self->count = 0;
}

static int dill_bheap_traverse_next(struct dill_bheap *self, struct dill_bheap_item **parent) {
    if(dill_slow(!self->count)) return -1;
    int height = 31 - __builtin_clz(self->count + 1);
    int dir = (self->count + 1) << (32 - height);
    *parent = self->root;
    dill_bheap_traverse(height - 1, *parent, dir);
    return !!(dir & 0x80000000);
}

static struct dill_bheap_item *dill_bheap_traverse_last(struct dill_bheap *self) {
    if(dill_slow(!self->count)) return NULL;
    int height = 31 - __builtin_clz(self->count);
    int dir = self->count << (32 - height);
    struct dill_bheap_item *node = self->root;
    dill_bheap_traverse(height, node, dir);
    return node;
}

static struct dill_bheap_item *dill_bheap_swap_child(struct dill_bheap_item *item, int right) {
    struct dill_bheap_item *up, *c0, *c1;
    up = item->up;
    c0 = item->child[right];
    c1 = item->child[!right];

    item->up = c0;
    item->left = c0->left;
    item->right = c0->right;
    if(item->left) item->left->up = item;
    if(item->right) item->right->up = item;

    c0->up = up;
    c0->child[right] = item;
    c0->child[!right] = c1;
    if(c1) c1->up = c0;
    if(c0->child[!right]) c0->child[!right]->up = c0;

    if(up) up->child[up->left != item] = c0;
    return c0;
}

static void dill_bheap_sift_up(struct dill_bheap *self, struct dill_bheap_item *item) {
    /* If item has no parent, there is nothing to do. */
    if(dill_slow(!item->up)) return;
    while(item->up && item->up->val >= item->val)
        dill_bheap_swap_child(item->up, item->up->left != item);
    if(item->up == NULL) self->root = item;
}

static void dill_bheap_sift_down(struct dill_bheap *self, struct dill_bheap_item *item) {
    struct dill_bheap_item *next;
    /* If item has no children, there is nothing to do. */
    if(dill_slow(!item->left)) return;
    int l = (item->left->val < item->val);
    int r = item->right && (item->right->val < item->val);
    struct dill_bheap_item *child = NULL;
    if(l || r) {
        child = dill_bheap_swap_child(item, !l);
        if(!child->up) self->root = child;
    } else return;
    while(1) {
        l = item->left && (item->left->val < item->val);
        r = item->right && (item->right->val < item->val);
        if(l || r) dill_bheap_swap_child(item, !l); else break;
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
    //dill_assert(dir != -1);
    //dill_assert(parent != NULL);
    parent->child[dir] = item;
    /* Finalise new item; it's now a leaf with p as parent. */
    item->up = parent;
    item->val = val;
    self->count++;
    /* Sift new item up. */
    dill_bheap_sift_up(self, item);
}

void dill_bheap_erase(struct dill_bheap *self, struct dill_bheap_item *item) {
    /* Get last item in heap. */
    struct dill_bheap_item *p = dill_bheap_traverse_last(self);
    dill_assert(p != NULL);
    self->count--;
    /* Empty heap. */
    if(dill_slow(!self->count)) {
        dill_assert(p == self->root);
        self->root = NULL;
        return;
    }
    /* p cannot be root. */
    dill_assert(p->up != NULL);
    /* Removing last item. */
    if(item == p) {
        p->up->child[p->up->left != p] = NULL;
        return;
    }
    int l = item->left == p;
    int r = item->right == p;
    if(l || r) {
        dill_bheap_swap_child(item, !l);
        if(l) p->left = p->right;
        p->right = NULL;
    } else {
        /* Remove p's parents' links to p. */
        p->up->child[p->up->left != p] = NULL;
        /* Insert the new item under the parent of item. */
        if(item->up) item->up->child[item->up->left != item] = p;
        /* Move internal links of item to p. */
        if(item->left) item->left->up = p;
        if(item->right) item->right->up = p;
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
