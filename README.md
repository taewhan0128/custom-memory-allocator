# custom-memory-allocator
Custom C memory allocator implementing malloc, free, calloc, and 
realloc using sbrk and a linked-list heap structure

## Overview

This project implements a custom dynamic memory allocator in C that
replaces the standard library functions:

- malloc
- free
- calloc
- realloc

The allocator manages heap memory manually using `sbrk()` and tracks
memory blocks using a singly linked list of metadata headers.

The design demonstrates key concepts used in real allocators, including:

- block splitting
- block coalescing
- free block reuse
- heap growth via system calls
- thread safety using `pthread_mutex`

The allocator manages heap memory manually using `sbrk()` and maintains
a linked list of memory blocks with metadata headers.

Thread safety is ensured using `pthread_mutex`.

## Features

- Custom implementation of `malloc`, `free`, `calloc`, `realloc`, and `coalesce`
- Heap expansion using `sbrk()`
- Metadata header stored before each allocated block
- Linked list to track allocated and free blocks
- Reuse of freed memory blocks and resized, if possible
- Thread-safe allocator using `pthread_mutex`

## Memory Layout

Each allocated block contains a metadata header followed by the user
memory region returned by `malloc`.

| header_t | user data |

The header stores metadata required for managing heap blocks.

## Allocation Strategy

1. Traverse the linked list to locate a free block large enough for the request.
2. If a suitable block is found:
   - If the block is significantly larger than the requested size,
     it is split into two blocks:
       [allocated block] [remaining free block]
   - The first block is marked as used and returned to the user.
3. If no suitable free block exists:
   - The heap is extended using `sbrk()`
   - A new block is created and appended to the linked list.

## Block Coalescing

To reduce heap fragmentation, adjacent free blocks are merged
whenever `free()` is called.

The coalescing algorithm:

1. Traverse the linked list from the head.
2. If two neighboring blocks are both marked free,
   they are merged into a single larger block.

This implementation performs a full list traversal and therefore
runs in **O(n)** time.

## Freeing Memory

When `free()` is called:

1. The allocator retrieves the block header located before the user pointer.
2. If the block is at the end of the heap:
   - the heap is shrunk using `sbrk()`
3. Otherwise:
   - the block is marked as free
   - combine free adjacent block with coalesce_free_blocks function for bigger memory block

## Example Heap Layout

Before allocation:

[used 128] -> [free 512] -> [used 64]

malloc(100)

After allocation and split:

[used 128] -> [used 100] -> [free 396] -> [used 64]

## Build
On MacOS:

Compile the allocator:
`gcc -shared -fPIC slow_malloc.c -o slow_malloc.dylib -pthread`

Compile the test program:
`gcc test.c -o test`

`gcc test.c slow_malloc.c -o test -pthread`

`./test`

## Future Work

- Best-fit / first-fit allocation strategies
- Memory alignment improvements
- Replace `sbrk()` with `mmap`