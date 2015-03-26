---
title: nROP converters
layout: base
---

[1]: http://lcamtuf.coredump.cx/afl/
[2]: https://github.com/0vercl0k/rp

# Static analysis

The `src/Makefile` handles `gcc` and `clang` as compilers. You can switch from one to another by commenting out `C` and `CXX` variables:

    CC=gcc
    CXX=g++
    #CC=clang --analyze -x c
    #CC=clang 
    #CXX=clang++

to

    #CC=gcc
    #CXX=g++
    #CC=clang --analyze -x c
    CC=clang 
    CXX=clang++

# Dynamic analysis

## ASAN

A fuzzing session with [afl][1] and [rp][2] showed a nice crash, in `examples/execve-badstrtabsize`, with a valid ELF and a huge strtab size. While our project handles this case (and not rp++ v1 :), ASAN will stop the process with :

    ==10359==WARNING: AddressSanitizer failed to allocate 0x800000000108 bytes
    ==10359==AddressSanitizer's allocator is terminating the process instead of returning 0
    ==10359==If you don't like this behavior set allocator_may_return_null=1
    ==10359==AddressSanitizer CHECK failed: /build/llvm/src/llvm-3.6.0.src/projects/compiler-rt/lib/sanitizer_common/sanitizer_allocator.cc:146 "((0)) != (0)" (0x0, 0x0)
    #0 0x64c834 in __asan::AsanCheckFailed(char const*, int, char const*, unsigned long long, unsigned long long) (/home/dad/Outils/nrop/nrop/packer+0x64c834)
    #1 0x6531b1 in __sanitizer::CheckFailed(char const*, int, char const*, unsigned long long, unsigned long long) (/home/dad/Outils/nrop/nrop/packer+0x6531b1)
    #2 0x651ca3 in __sanitizer::ReportAllocatorCannotReturnNull() (/home/dad/Outils/nrop/nrop/packer+0x651ca3)
    #3 0x5bf706 in __asan::Allocator::Allocate(unsigned long, unsigned long, __sanitizer::BufferedStackTrace*, __asan::AllocType, bool) (/home/dad/Outils/nrop/nrop/packer+0x5bf706)
    #4 0x644d6c in __interceptor_malloc (/home/dad/Outils/nrop/nrop/packer+0x644d6c)
    #5 0x6735eb in create_section (/home/dad/Outils/nrop/nrop/packer+0x6735eb)
    #6 0x674a32 in symbols_elf64 (/home/dad/Outils/nrop/nrop/packer+0x674a32)
    #7 0x674581 in create_elf (/home/dad/Outils/nrop/nrop/packer+0x674581)
    #8 0x676e4e in parse (/home/dad/Outils/nrop/nrop/packer+0x676e4e)
    #9 0x6650b0 in main (/home/dad/Outils/nrop/nrop/packer+0x6650b0)
    #10 0x7f53237567ff in __libc_start_main (/usr/lib/libc.so.6+0x207ff)
    #11 0x5bdce8 in _start (/home/dad/Outils/nrop/nrop/packer+0x5bdce8)


You can allow this behavior with

    export ASAN_OPTIONS="allocator_may_return_null=1"

And run the program again.
