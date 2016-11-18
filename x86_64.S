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

.global dill_trampoline_void
.type dill_trampoline_void,@function
dill_trampoline_void:
.global dill_trampoline_vargs
.type dill_trampoline_vargs,@function
dill_trampoline_vargs:

    add     $8, %rsp
    mov     dill_tr_info@GOTPCREL(%rip), %r11
    mov     8(%r11), %r10
    mov     %rbx, (%r10)
    mov     %rbp, 8(%r10)
    mov     %r12, 16(%r10)
    mov     %r13, 24(%r10)
    mov     -8(%rsp), %rbx
    mov     %r14, 32(%r10)
    mov     %r15, 40(%r10)
    mov     %rsp, 48(%r10)
    mov     %rbx, 56(%r10)
    mov     16(%r11), %rsp
    call    *(%r11)
#if defined DILL_STATIC
    jmp     dill_epilogue
#else
    jmp     *dill_epilogue@GOTPCREL(%rip)
#endif
