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

__attribute__((optimize("-O3")))
void dill_trampoline(struct dill_cr_info *info, ...) {
    asm("lea     LJMPRET%=(%%rip), %%r11\n\t"
        "mov     8(%%rdx), %%rcx\n\t"
        "mov     %%rbx, (%%rcx)\n\t"\
        "mov     %%rbp, 8(%%rcx)\n\t"\
        "mov     %%r12, 16(%%rcx)\n\t"\
        "mov     %%r13, 24(%%rcx)\n\t"\
        "mov     %%r14, 32(%%rcx)\n\t"\
        "mov     %%r15, 40(%%rcx)\n\t"\
        "mov     %%rsp, 48(%%rcx)\n\t"\
        "mov     %%r11, 56(%%rcx)\n\t"\
        "mov     16(%%rdx), %%rsp\n\t"
        "call    *(%%rdx)\n\t"
        "jmp     dill_epilogue\n\t"
        "LJMPRET%=:\n\t"
        : : "d" (info) : "rcx", "r11");
}
