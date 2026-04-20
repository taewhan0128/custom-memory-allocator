# custom-memory-allocator
Custom C memory allocator implementing malloc, free, calloc, and realloc using sbrk and a linked-list heap structure

## Overview

This project implements a custom dynamic memory allocator in C that provides
replacements for the standard library functions:

- malloc
- free
- calloc
- realloc

The allocator manages heap memory manually using `sbrk()` and maintains
a linked list of memory blocks with metadata headers.

Thread safety is ensured using `pthread_mutex`.

## Features

- Custom implementation of `malloc`, `free`, `calloc`, and `realloc`
- Heap expansion using `sbrk()`
- Metadata header stored before each allocated block
- Linked list to track allocated and free blocks
- Reuse of freed memory blocks
- Thread-safe allocator using `pthread_mutex`

## Memory Layout

Each allocated block contains a metadata header followed by the user data.

| header_t | user memory |

header_t structure:

size_t size      → size of user allocation  
unsigned is_free → whether block is free  
header_t* next   → next block in the linked list


## Allocation Strategy

1. Search the linked list for a free block large enough.
2. If found:
   - mark block as used
   - return pointer to user memory
3. If not found:
   - extend heap using `sbrk()`
   - create a new block
   - append it to the linked list

## Freeing Memory

When `free()` is called:

1. The allocator retrieves the block header located before the user pointer.
2. If the block is at the end of the heap:
   - the heap is shrunk using `sbrk()`
3. Otherwise:
   - the block is marked as free
   - it can be reused by future allocations

## Build

Compile the allocator:

gcc -shared -fPIC slow_malloc.c -o slow_malloc.dylib -pthread

Compile the test program:

gcc test.c -o test

gcc test.c yonsei_malloc.c -o test -pthread 

./test

## Future Work

- Block splitting
- Block coalescing
- Best-fit / first-fit allocation strategies
- Memory alignment improvements
- Replace `sbrk()` with `mmap`
