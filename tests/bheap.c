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

#include "assert.h"
#include "../bheap.c"

int main(void) {
    struct dill_bheap heap;
    dill_bheap_init(&heap);

    int rc = dill_bheap_empty(&heap);
    assert(rc == 1);

    struct dill_bheap_item items[16];
    int i;
    for(i = 0; i != 10; ++i) {
        dill_bheap_insert(&heap, i, &items[i]);
    }
    for(i = 0; i != 10; ++i) {
        int64_t l = items[i].left ? items[i].left->val : -1; 
        int64_t r = items[i].right ? items[i].right->val : -1; 
        int64_t u = items[i].up ? items[i].up->val : -1; 
        printf("%ld: [l]%ld [r]%ld [u]%ld\n", items[i].val, l, r, u);
    }

    dill_bheap_erase(&heap, &items[0]);
    dill_bheap_erase(&heap, &items[4]);
    dill_bheap_erase(&heap, &items[5]);
    dill_bheap_erase(&heap, &items[9]);
    for(i = 10; i != 16; ++i) {
        dill_bheap_insert(&heap, i, &items[i]);
    }

    printf("***\n");
    printf("root: %ld\n", heap.root->val);
    for(i = 0; i != 16; ++i) {
        int64_t l = items[i].left ? items[i].left->val : -1; 
        int64_t r = items[i].right ? items[i].right->val : -1; 
        int64_t u = items[i].up ? items[i].up->val : -1; 
        printf("%ld: [l]%ld [r]%ld [u]%ld\n", items[i].val, l, r, u);
    }

    //dill_bheap_insert(&heap, 0, &items[0]);
    //dill_bheap_insert(&heap, 4, &items[4]);
    //dill_bheap_insert(&heap, 5, &items[5]);
    //dill_bheap_insert(&heap, 9, &items[9]);

    //rc = dill_bheap_empty(&heap);
    //assert(rc == 0);

    return 0;
}

