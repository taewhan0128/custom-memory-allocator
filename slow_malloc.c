// Simple custom memory allocator implementing malloc/free/calloc/realloc
// Uses sbrk() to extend heap and a linked list of headers to track blocks
// Thread-safe using a global pthread mutex

// WARNING: sbrk() is deprecated on macOS but still functional for educational use

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

typedef char ALIGN[16];

union header{
    struct{
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    // force the header to be algined to 16bytes
    ALIGN stub;
    
};
typedef union header header_t;

header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;

header_t *get_free_block(size_t size){
    header_t *curr = head;
    while(curr){
        // check whether there is free block
        // that matches request size
        if(curr->s.is_free && curr->s.size >= size){
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

void coalesce_free_blocks(){
    header_t *curr = head;
    // traverse the linked list of heap blocks
    while (curr && curr->s.next){
         // if both current block and next block are free,
        // merge them into a single larger block
        if(curr->s.is_free && curr->s.next->s.is_free){
            write(2, "coalesece free block called\n", 27);              // debug line to check combing block
            curr->s.size += sizeof(header_t) + curr->s.next->s.size;    // combine size of block and header
            curr->s.next = curr->s.next->s.next;                        // traverse to next block
            if(curr->s.next == NULL){
                tail = curr;
            }
        }else{
            curr = curr->s.next;
        }
    }
}

void free(void *block){
    write(2, "free called\n", 12); // debug trace
    header_t *header, *tmp;
    void *programbreak;            // pointer to end of data segment
    if(!block){
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)block - 1; // move back from user pointer to access metadata header
    programbreak = sbrk(0);        // gives current end of data segement address

    // check if data segment is at the end of linked list.
    // if it's at the end of linked list, reduce the size of
    // heap and release the memory to OS. Else, we keep the
    // block and mark as free. 
    if((char*)block + header->s.size == programbreak){
        if(head == tail){
            head = tail = NULL;
        }else{
            tmp = head;
            while(tmp){
                if(tmp->s.next == tail){
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        /* sbrk with negative value decrease the
        heap size. thus we release the memroy to 
        OS. */
        sbrk(0 - header->s.size - sizeof(header_t));
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header->s.is_free = 1;
    coalesce_free_blocks();  // check and merge free that are beside
    pthread_mutex_unlock(&global_malloc_lock);
}

void *malloc(size_t size){
    write(2, "malloc called\n", 14);  // debug trace
    size_t total_size;
    void *block;
    header_t *header;
    if(!size){
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);

    // if free block of reqeust size is found,
    // return the address of the block.
    if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header+1);
    }

    // set total size as header size + requested block size
    total_size = sizeof(header_t) + size;
    // requeset memory of total size from os
    block = sbrk(total_size);
    if(block == (void*)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head){
        head = header;
    }
    if(tail){
        tail->s.next = header;
    }
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header+1);
}

void *calloc(size_t num, size_t nsize){
    write(2, "calloc called\n", 14);  // debug trace
    size_t size;
    void *block;
    if(!num || !nsize){
        return NULL;
    }
    size = num * nsize;
    // check mul overflow
    if(nsize != size / num){
        return NULL;
    }
    block = malloc(size);
    if(!block){
        return NULL;
    }
    memset(block, 0, size);
    return block;
}

void *realloc(void *block, size_t size){
    write(2, "realloc called\n", 15);  // debug trace
    header_t *header;
    void *ret;
    if(!block || !size){
        return malloc(size);
    }
    header = (header_t*)block - 1; // move back from user pointer to access metadata header
    if(header->s.size >= size){
        return block;
    }
    ret = malloc(size);
    if(ret){
        /* relocate the content to the 
        bigger size block*/
        memcpy(ret, block, header->s.size);
        // free the old block
        free(block);
    }
    return ret;
}

// this is debug function; 
// not required if using test file
void print_mem_list()
{
	header_t *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->s.size, curr->s.is_free, (void*)curr->s.next);
		curr = curr->s.next;
	}
}