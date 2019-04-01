/**
 * Malloc Lab
 * CS 241 - Spring 2019
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef struct meta_st
{
   void* ptr;
   size_t size;           
   int free;              //1 for free, 0 for not free
   struct meta_st *next;  
   struct meta_st *prev;
   struct meta_st *nextfree;
   struct meta_st *prevfree;
}meta_st;

static meta_st *last = NULL;
static meta_st *base = NULL;

// |meta_st| + size  +  |meta_st| + size  |
// base      ........... last

meta_st* split_block(meta_st* chosen, size_t size);
meta_st* extend_heap(meta_st* chosen, size_t size);
void add(meta_st* node);
void move(meta_st* node);


void add(meta_st* node){
    if(base)
        base->prevfree = node;
    node -> nextfree = base;
    node -> prevfree = NULL;
    base = node;
}

void move(meta_st* node){
    if(node -> prevfree != NULL)
        node -> prevfree -> nextfree = node -> nextfree;
    if(node -> nextfree != NULL)
        node -> nextfree -> prevfree = node -> prevfree;
    if(node == base)
        base = node -> nextfree;
    node -> prevfree = NULL;
    node -> nextfree = NULL;
}
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    void *p = malloc(num*size);
    if(p){
        memset(p, 0, num*size);
    }
    return p;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    //meta_st *la = last;
    meta_st *ba = base;
    meta_st *chosen = NULL;

    while(ba!= NULL){
        // if (la->free && la->size >= size) {
        //     chosen = la;
        //     break;
        // }
        // if (la <= ba) {break;}
        if (ba->free && ba->size >= size) {
            chosen = ba;
            break;
        }        
        //la = la->prev;
        ba = ba->nextfree;
    }
    
    if (chosen) {
        move(chosen);
        chosen->free = 0; //not a free block anymore
        if (chosen->size > size + 150)
            chosen = split_block(chosen, size);
        return chosen->ptr;
    }
    
    chosen = extend_heap(chosen, size);
    return chosen->ptr;
}

meta_st* split_block(meta_st* chosen, size_t size){
    meta_st* newnode = (meta_st*) (chosen->ptr + size);
    newnode->size = chosen->size - size - sizeof(meta_st);
    chosen->size = size;
    newnode->prev = chosen;
    newnode->next = chosen->next;
    chosen->next = newnode;
    if(newnode->next)
        newnode->next->prev = newnode;
    newnode->ptr = newnode + 1;
    newnode->free = 1;
    if(chosen == last)
        last = newnode;
    add(newnode);
    return chosen;
}

meta_st* extend_heap(meta_st* chosen, size_t size){
    chosen = sbrk(sizeof(meta_st) + size);
    if(chosen == (void*)-1){
        return NULL;
    }
    chosen->ptr = chosen + 1;
    chosen->size = size;
    chosen->free = 0;
    chosen->prev = last;
    chosen->next = NULL;
    if(last) {
        last->next = chosen;
    }
    //if(base == NULL)  base = chosen;
    last = chosen;
    return chosen;
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (!ptr)  return;
    meta_st* temp = (meta_st*) ptr - 1;
    temp->free = 1;
    add(temp);

    if(temp->prev != NULL){      //combine??
        if(temp->prev->free){
            if(temp == last)  last = temp->prev;
            move(temp);
            temp->prev->size = temp->prev->size + sizeof(meta_st) + temp->size;
            temp->prev->next = temp->next;
            if(temp->next)
                temp->next->prev = temp->prev;
            temp=temp->prev; 
        }
    }

    if(temp->next != NULL){      //combine??
        if(temp->next->free){
            if(temp->next == last)  last = temp;
            move(temp->next);
            temp->size = temp->size + sizeof(meta_st) + temp->next->size;
            if(temp->next->next)    
                temp->next->next->prev = temp;   
            temp->next = temp->next->next;
        }
    }
    //to_free_list(temp);
    // meta_st* la = last;
    // meta_st* ba = base;
    // while(la && ba){
    //     if (la->ptr == ptr) {
    //         la->free = 1;
    //         break;
    //     }
    //     if(la <= ba) break;  //extra protection?
    //     if (ba->ptr == ptr) {
    //         ba->free = 1;
    //         break;
    //     }
    //     la = la->next;
    //     ba = ba->prev;
    // }

    return;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if(ptr == NULL) return malloc(size);
    if(size == 0) {
        free(ptr);
        return NULL;
    }

    meta_st* p = (meta_st*)ptr - 1;
    size_t oldsize = p->size; 

    /*------reduce the size-------*/
    if(oldsize >= size){
        //split???
        // if(oldsize > 2*size && (oldsize - size == 1024)){
        //     meta_st* entry = p + size;    //????????????????????
        //     entry->ptr = entry + 1;
        //     entry->free = 1;
        //     entry->size = oldsize - size - sizeof(meta_st);
        //     entry->next = p->next;
        //     entry->prev = ptr;
        //     p->next = entry;
        // }
         return ptr;
        //not enough to split, then do nothing.
    }

    /*------expand the size-------*/
    else{
        //fusion the one after??

        //malloc new
        void* result = malloc(size);
        if(!result) return NULL;            /************** check in the freelist ?????? **********/
        memcpy(result, ptr, oldsize);
        free(ptr);
        return result;
    }
   // return ptr;
}



// void to_free_list(meta_st* ptr){
    
// }