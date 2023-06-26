/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20191630",
    /* Your full name*/
    "JinYong Lee",
    /* Your email address */
    "ljy2855@sogang.ac.kr",
};
static char *heap_head = 0;
static char *heap_tail = 0;
static char *free_head = 0;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define SET_ALLOC(p) PUT(p, (GET(p) | 0x1))
#define CLR_ALLOC(p) PUT(p, (GET(p) & ~0x1))
#define SET_TAG(p) PUT(p, (GET(p) | 0x2))
#define GET_TAG(p) (GET(p) & 0x2)
#define CLR_TAG(p) PUT(p, (GET(p) & ~0x2))
#define SET_SIZE(p, size) PUT((p), (size | (GET(p) & 0x7)))

#define HDRP(ptr) ((char *)(ptr)-WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)
#define BLOCK_SIZE(p) (GET_SIZE(HDRP(p)))

#define NEXT_BLOCK(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr)-WSIZE))
#define PREV_BLOCK(ptr) ((char *)(ptr)-GET_SIZE((char *)(ptr)-DSIZE))

#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

#define PRED(ptr) (char *)(free_head + GET(PRED_PTR(ptr)))
#define SUCC(ptr) (char *)(free_head + GET(SUCC_PTR(ptr)))
#define OFFSET(ptr) ((char *)(ptr)-free_head)

#define SET_PRED(self, ptr) PUT(PRED_PTR(self), OFFSET(ptr))
#define SET_SUCC(self, ptr) PUT(SUCC_PTR(self), OFFSET(ptr))

#define CONNECT(ptr1, ptr2) \
    SET_SUCC(ptr1, ptr2);   \
    SET_PRED(ptr2, ptr1);

#define INSERT_LIST(new, target) \
    CONNECT(new, SUCC(target));  \
    CONNECT(target, new);
#define ERASE(ptr) CONNECT(PRED(ptr), SUCC(ptr))


static inline void *extend_heap(size_t words);
static inline void *coalesce(void *bp);
static inline void *find_fit_at_list(size_t size);
static inline void *place(char *, size_t);
static inline int mm_check();
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_head = mem_sbrk(6 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    free_head = heap_head + WSIZE; /* Init head ptr */
    PUT(heap_head, 0);             /* Alignment padding */
    PUT(heap_head + WSIZE, 0);     /* free list head ptr*/
    PUT(heap_head + (2 * WSIZE), 0);
    PUT(heap_head + (3 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_head + (4 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_head + (5 * WSIZE), PACK(0, 3));     /* Epilogue header */
    heap_head += (4 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
        return -1;
    }
    SET_TAG(HDRP(NEXT_BLOCK(heap_head)));
    INSERT_LIST(NEXT_BLOCK(heap_head), free_head);
#ifdef DEBUG
    // printf("[after init]\n");
    assert(!mm_check());
#endif
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    size_t alloc_size, extendsize;
    char *new;
    if (size == 0)
    {
        return NULL;
    }

    alloc_size = MAX(DSIZE * 2, (ALIGN(size) - size >= WSIZE) ? ALIGN(size) : (ALIGN(size) + DSIZE));
#ifdef DEBUG
    // printf("[malloc] size : %u\n", alloc_size);

#endif
    if ((new = find_fit_at_list(alloc_size)) != NULL)
    {
        return place(new, alloc_size);
    }
    extendsize = MAX(alloc_size, CHUNKSIZE);
    if (!GET_TAG(heap_tail))
    {
#ifdef DEBUG
        // printf("[heap_tail not full]\n");

#endif
        extendsize -= GET_SIZE(heap_tail - WSIZE);
    }
    if ((new = extend_heap(extendsize / WSIZE)) == NULL)
    {
        return NULL;
    }

    return place(new, alloc_size);
}
static inline void *find_fit_at_list(size_t size)
{
    char *cur = free_head;
    char *temp = NULL;

    while ((cur = SUCC(cur)) != free_head)
    {
   
        if (BLOCK_SIZE(cur) >= size)
        {
            if (temp == NULL)
            {
                temp = cur;
            }
            else
            {
                if (BLOCK_SIZE(cur) < BLOCK_SIZE(temp))
                {
                    temp = cur;
                }
            }
        }
    }
    if (temp == NULL)
        return NULL;
    ERASE(temp);
    return temp;
}

static void inline *place(char *bp, size_t asize)
{
    size_t next_size;
    size_t size = BLOCK_SIZE(bp);
    
    char *next;
    
    SET_ALLOC(HDRP(bp));
    if (size != asize && size - asize >= 2 * DSIZE)
    {
        next_size = size - asize;
        
        if (asize < 12 * DSIZE || next_size < 4 * DSIZE)
        {
            // alloc current block
            SET_SIZE(HDRP(bp), asize);
            next = NEXT_BLOCK(bp);
            PUT(HDRP(next), PACK(next_size, 0));
            PUT(FTRP(next), PACK(next_size, 0));
        }
        else
        {
            // alloc next to next block
            next = bp;
            SET_SIZE(HDRP(next), next_size);
            PUT(FTRP(next), PACK(next_size, 0));
            // swap bp and next pointer
            bp = NEXT_BLOCK(next);
            SET_ALLOC(HDRP(bp));
            SET_SIZE(HDRP(bp), asize);
        }
        SET_TAG(HDRP(NEXT_BLOCK(bp)));
        next = coalesce(next);
        SET_TAG(HDRP(next));
        CLR_ALLOC(HDRP(next));
        CLR_TAG(HDRP(NEXT_BLOCK(next)));

        INSERT_LIST(next, free_head);
    }
    SET_TAG(HDRP(NEXT_BLOCK(bp)));
#ifdef DEBUG
    assert(!mm_check());
#endif
    return bp;
}

static inline void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
#ifdef DEBUG
    // printf("[extend_heap] before heap_pro : %p heap_epilogue : %p ....\n", heap_head, heap_tail);
#endif
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }
#ifdef DEBUG
    // printf("[extend_heap sbrk] last bp : %p extend size : %u\n", bp, size);
#endif
    /* Initialize free block header/footer and the epilogue header */
    SET_SIZE(HDRP(bp), size); /* Free block header */
    CLR_ALLOC(HDRP(bp));
    PUT(FTRP(bp), PACK(size, 0));          /* Free block footer */
    PUT(HDRP(NEXT_BLOCK(bp)), PACK(0, 1)); /* New epilogue header */
    heap_tail = HDRP(NEXT_BLOCK(bp));
#ifdef DEBUG
    // printf("[extend_heap] after heap_pro : %p heap_epilogue : %p ....\n\n", heap_head, heap_tail);
    assert(!mm_check());
#endif
    /* Coalesce if the previous block was free */

    return coalesce(bp);
}

static inline void *coalesce(void *bp)
{
#ifdef DEBUG
    // printf("[coalesce before] addr = %p, size = %u prev alloc : %d next alloc : %d\n",
    //        bp, GET_SIZE(HDRP(bp)), GET_TAG(HDRP(bp)), GET_ALLOC(HDRP(NEXT_BLOCK(bp))));
#endif

    size_t prev_alloc = GET_TAG(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLOCK(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { /* Case 1 */
    }
    else if (prev_alloc && !next_alloc)
    { /* Case 2 */
        ERASE(NEXT_BLOCK(bp));
        size += GET_SIZE(HDRP(NEXT_BLOCK(bp)));
        SET_SIZE(HDRP(bp), size);
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    { /* Case 3 */
        ERASE(PREV_BLOCK(bp));
        size += GET_SIZE(HDRP(PREV_BLOCK(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        SET_SIZE(HDRP(PREV_BLOCK(bp)), size);
        bp = PREV_BLOCK(bp);
    }
    else
    { /* Case 4 */
        ERASE(PREV_BLOCK(bp));
        ERASE(NEXT_BLOCK(bp));
        size += GET_SIZE(HDRP(PREV_BLOCK(bp))) + GET_SIZE(HDRP(NEXT_BLOCK(bp)));
        SET_SIZE(HDRP(PREV_BLOCK(bp)), size);
        PUT(FTRP(NEXT_BLOCK(bp)), PACK(size, 0));
        bp = PREV_BLOCK(bp);
    }
#ifdef DEBUG
    // printf("[coalesce finish] addr = %p, size = %u \n\n",bp, GET_SIZE(HDRP(bp)));
    assert(!mm_check());
#endif
    return bp;
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

#ifdef DEBUG
    // printf("[free start] addr = %p, size = %u\n", ptr, GET_SIZE(HDRP(ptr)));
#endif

    size_t block_size;
    block_size = GET_SIZE(HDRP(ptr));
    CLR_ALLOC(HDRP(ptr));
    SET_SIZE(FTRP(ptr), block_size);

    CLR_TAG(HDRP(NEXT_BLOCK(ptr)));

    ptr = coalesce(ptr);
    CLR_TAG(HDRP(NEXT_BLOCK(ptr)));
#ifdef DEBUG
    assert(GET_TAG(HDRP(ptr)));
    assert(!GET_TAG(HDRP(NEXT_BLOCK(ptr))));
#endif
    INSERT_LIST(ptr, free_head);
#ifdef DEBUG
    // printf("[free finish] \n\n");
    assert(!mm_check());
#endif
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    void *oldptr = ptr;
    void *newptr;
    size_t copySize, alloc_size;
    size_t prev_size, next_size, sum_size;
    prev_size = 0;
    next_size = 0;
    copySize = BLOCK_SIZE(ptr) - DSIZE;

    if (ptr == NULL)
    {
        return mm_malloc(size);
    }
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    alloc_size = MAX(DSIZE * 2, (ALIGN(size) - size >= WSIZE) ? ALIGN(size) : (ALIGN(size) + DSIZE));

    if (alloc_size <= BLOCK_SIZE(oldptr))
    {

        return ptr;
    }
    if (FTRP(oldptr) + WSIZE == heap_tail)
    {
        SET_TAG(HDRP(NEXT_BLOCK(oldptr)));
        if ((extend_heap((alloc_size - BLOCK_SIZE(oldptr)) / WSIZE)) == NULL)
        {
            return NULL;
        }
        SET_SIZE(HDRP(ptr), alloc_size);
        SET_SIZE(FTRP(ptr), alloc_size);
        SET_TAG(HDRP(NEXT_BLOCK(ptr)));
        return oldptr;
    }

    if (!GET_ALLOC(HDRP(NEXT_BLOCK(ptr))))
        next_size = BLOCK_SIZE(NEXT_BLOCK(ptr));
    if (!next_size && HDRP(NEXT_BLOCK(NEXT_BLOCK(oldptr))) == heap_tail)
    {

        if (alloc_size > next_size + BLOCK_SIZE(oldptr))
        {
            ERASE(NEXT_BLOCK(oldptr));
            if ((extend_heap((alloc_size - BLOCK_SIZE(oldptr) - next_size) / WSIZE)) == NULL)
            {
                return NULL;
            }
            SET_SIZE(HDRP(ptr), alloc_size);
            SET_SIZE(FTRP(ptr), alloc_size);
            SET_TAG(HDRP(NEXT_BLOCK(ptr)));
            return oldptr;
        }
    }
#ifdef DEBUG
    // printf("[realloc] new size : %u, old size : %u \n", alloc_size, copySize + DSIZE);
    // assert(!memcmp(newptr,oldptr,copySize));
#endif
    // TODO if append its size
    if (!GET_ALLOC(HDRP(PREV_BLOCK(ptr))))
        prev_size = BLOCK_SIZE(PREV_BLOCK(ptr));
    sum_size = BLOCK_SIZE(ptr) + prev_size + next_size;
    if (alloc_size <= sum_size)
    {
        CLR_ALLOC(HDRP(ptr));
        CLR_TAG(HDRP(NEXT_BLOCK(ptr)));
#ifdef DEBUG
        // printf("[realloc extend] new size : %u, old size : %u prev : %u, next %u\n", alloc_size, copySize + DSIZE, prev_size, next_size);
        // assert(!memcmp(newptr,oldptr,copySize));
#endif
        newptr = coalesce(oldptr);
        memcpy(newptr, oldptr, copySize);

        if (sum_size >= alloc_size + 2 * DSIZE)
        {
            char *next;
            SET_SIZE(HDRP(newptr), alloc_size);
            SET_SIZE(FTRP(newptr), alloc_size);
            SET_ALLOC(HDRP(newptr));
            PUT(HDRP(NEXT_BLOCK(newptr)), PACK((sum_size - alloc_size), 0));
            PUT(FTRP(NEXT_BLOCK(newptr)), PACK((sum_size - alloc_size), 0));
            SET_TAG(HDRP(NEXT_BLOCK(newptr)));
            next = coalesce(NEXT_BLOCK(newptr));
            INSERT_LIST(next, free_head);
            CLR_TAG(HDRP(NEXT_BLOCK(next)));
        }
        else
        {
            SET_SIZE(HDRP(newptr), sum_size);
            SET_SIZE(FTRP(newptr), sum_size);
            SET_ALLOC(HDRP(newptr));
            SET_TAG(HDRP(NEXT_BLOCK(newptr)));
        }
    }
    else
    {

        newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;

        memcpy(newptr, oldptr, copySize + WSIZE);
#ifdef DEBUG
        // printf("[realloc new malloc] \n");
        assert(!memcmp(newptr, oldptr, copySize + WSIZE));
#endif

        mm_free(oldptr);
    }
#ifdef DEBUG
    // printf("[realloc finish] new ptr : %p, size : %u\n", newptr, BLOCK_SIZE(newptr));
    assert(!mm_check());
#endif
    return newptr;
}

static int mm_check()
{

    // Is every block in the free list marked as free?
    void *next;
    for (next = SUCC(free_head); next != free_head; next = SUCC(next))
    {
        if (GET_ALLOC(HDRP(next)))
        {
            printf("Consistency error: block %p in free list but marked allocated!\n", next);
            return 1;
        }
        if (GET_TAG(HDRP(NEXT_BLOCK(next))))
        {
            printf("Consistency error: tag not set block %p\n", NEXT_BLOCK(next));
            return 1;
        }
    }

    // Are there any contiguous free blocks that escaped coalescing?
    for (next = SUCC(free_head); next != free_head; next = SUCC(next))
    {

        char *prev = PRED(next);
        if (prev != NULL && HDRP(next) - FTRP(prev) == DSIZE)
        {
            printf("Consistency error: block %p missed coalescing!\n", next);
            return 1;
        }
    }

    // Do the pointers in the free list point to valid free blocks?
    for (next = SUCC(free_head); next != free_head; next = SUCC(next))
    {
        if (next < mem_heap_lo() || next > mem_heap_hi())
        {
            printf("Consistency error: free block %p invalid\n", next);
            return 1;
        }
    }

    // Do the pointers in a heap block point to a valid heap address?
    for (next = heap_head; HDRP(NEXT_BLOCK(next)) != heap_tail; next = NEXT_BLOCK(next))
    {

        if (HDRP(next) < (char *)mem_heap_lo() || FTRP(next) > (char *)mem_heap_hi())
        {
            printf("Consistency error: block %p outside designated heap space hi : %p\n", next, mem_heap_hi());
            return 1;
        }
        //   if (GET_ALLOC(HDRP(next)))
        //   if(GET_SIZE(HDRP(next)) != GET_SIZE(FTRP(next))){
        //     printf("Consistency error: block %p header footer size diff head size : %u foot size :%u\n", next,GET_SIZE(HDRP(next)),GET_SIZE(FTRP(next)));
        //     return 1;
        //   }
    }
    if (heap_tail > (char *)mem_heap_hi())
    {
        printf("Consistency error: heap_tail : %p heap_hi : %p\n", heap_tail, mem_heap_hi());
        return 1;
    }

    return 0;
}