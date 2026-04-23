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
- memory alignment
- heap growth via system calls
- thread safety using `pthread_mutex`

The allocator manages heap memory manually using `sbrk()` and maintains
a linked list of memory blocks with metadata headers.

Thread safety is ensured using `pthread_mutex`.

## Features

- Custom implementation of `malloc`, `free`, `calloc`, and `realloc`
- Heap expansion using `sbrk()`
- Metadata header stored before each allocated block
- Singly linked list to track allocated and free blocks
- Linked list to track allocated and free blocks
- Reuse of freed memory blocks when possible
- Block splitting to minimize internal fragmentation
- Block coalescing to reduce heap fragmentation
- 16-byte memory alignment for safe CPU access
- Thread-safe allocator using a `global pthread_mutex`

## Memory Layout

Each allocated block contains a metadata header followed by the user
memory region returned by `malloc`.

| header_t | user data |

The header stores:
- block size
- free/used status
- pointer to the next block

This structure allows the allocator to traverse the heap and manage memory blocks efficiently.

## Memory Alignment

To ensure compatibility with CPU alignment requirements, all allocations are aligned to 16 bytes.
Before allocation, the requested size is rounded up.
This guarantees that returned pointers are safe for storing any primitive data type such as:
- double
- long
- struct
- SIMD data types

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
MacOS:
Compile the allocator as shared library:

`gcc -shared -fPIC slow_malloc.c -o slow_malloc.dylib -pthread`

Compile the test program:

`gcc test.c -o test`

`gcc test.c slow_malloc.c -o test -pthread`

`./test`

## Limitations

This allocator is designed for educational purposes and does not implement many advanced optimizations found in production allocators such as:

- segregated free lists
- per-thread arenas
- `mmap` for large allocations
- best-fit / size-class allocation strategies

## Future Work

- Best-fit / first-fit allocation strategies
- Memory alignment improvements
- Replace `sbrk()` with `mmap`