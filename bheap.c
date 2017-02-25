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
    switch(height - 1) {
        case 31: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 30: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 29: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 28: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 27: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 26: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 25: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 24: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 23: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 22: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 21: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 20: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 19: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 18: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 17: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 16: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 15: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 14: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 13: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 12: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 11: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 10: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  9: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  8: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  7: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  6: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  5: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  4: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  3: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  2: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  1: *parent = (*parent)->child[!!(dir & 0x80000000)]; dir <<= 1;
    }
    return !!(dir & 0x80000000);
}

static struct dill_bheap_item *dill_bheap_traverse_last(struct dill_bheap *self) {
    if(dill_slow(!self->count)) return NULL;
    int height = 31 - __builtin_clz(self->count);
    int dir = self->count << (32 - height);
    struct dill_bheap_item *node = self->root;
    switch(height) {
        case 31: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 30: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 29: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 28: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 27: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 26: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 25: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 24: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 23: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 22: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 21: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 20: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 19: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 18: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 17: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 16: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 15: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 14: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 13: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 12: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 11: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case 10: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  9: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  8: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  7: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  6: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  5: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  4: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  3: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  2: node = node->child[!!(dir & 0x80000000)]; dir <<= 1;
        case  1: node = node->child[!!(dir & 0x80000000)];
    }
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
    while(item->up && item->up->val > item->val)
        dill_bheap_swap_child(item->up, item->up->left != item);
    if(item->up == NULL) self->root = item;
}

static void dill_bheap_sift_down(struct dill_bheap *self, struct dill_bheap_item *item) {
    struct dill_bheap_item *next;
    /* If item has no children, there is nothing to do. */
    if(!item->left) return;
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
    if(!self->count) {
        self->root = NULL;
        return;
    }
    /* Removing last item. */
    if(item == p) {
        if(p->up) p->up->child[p->up->left != p] = NULL;
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
        if(p->up) p->up->child[p->up->left != p] = NULL;
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
