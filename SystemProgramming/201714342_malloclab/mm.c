/*
 * Below is the structure of heap which I mean to implement.
 * It is special in that it maintains information of heap inside itself,
 * which means the segregated free list is saved in dynamic memory area.
 * Also it has a prologue block which consists of segregated free list,
 * but it does not have a epilogue block, and has HEAP_T macro instead.
 * HEAP_T always points end of heap by calculation with HEAP_SIZE.
 * 
 * Main methods used are
 *   - segregated free list
 *   - explicit free list
 *   - immediate coalesce
 *
 *     31 30 29 28 27 26 25 24 ... 07 06 05 04 03 02 01 00
 *     ------------------prologue block-------------------
 * <= HEAP_H
 *  00 padding(0)
 *  01 [HDR] size of prologue block                    | 1
 * <= heap_h
 *  02 size of overall heap
 *  03 SEG(i) will be pointer of a block.
 *  04 SEG(i) will be pointer of a block.
 * ... 
 *   N SEG(i) will be pointer of a block.
 * N+1 [FTR] size of prologue block                    | 1
 *     ------------------prologue block-------------------
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/***************************************************************
 * constants and macros from textbook
 **************************************************************/

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - (2 * WSIZE))

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - 2 * WSIZE)))

/***************************************************************
 * original constants and macros
 **************************************************************/

/* segregated free list is devided in 2^k form */
#define MAX_SEGS 20
#define SEG(i) *((void**)(heap_h + ((unsigned int) (i + 1) * sizeof(void*))))

/* overall size of heap will be shown by footer of prologue block */ 
#define HEAP_SIZE GET(heap_h)

/* first address and last address of heap */
#define HEAP_H heap_h - DSIZE
#define HEAP_T heap_h - WSIZE + HEAP_SIZE

/* increase heap size */
#define HEAP_INCR(size) PUT(heap_h, PACK(size, 0)) 

/* get or set predecessor, successor in seglist */
#define SET_PRED(ptr, pred) PUT(((void**)(ptr)), pred)
#define GET_PRED(ptr) (*(void**)(ptr))
#define SET_SUCC(ptr, succ) PUT(((void**)(ptr + WSIZE)), succ)
#define GET_SUCC(ptr) (*(void**)(ptr + WSIZE))

/***************************************************************
 * global variables
 **************************************************************/

void* heap_h; /* this points first address in heap */

/***************************************************************
 * function declaration
 **************************************************************/
 
static void print_heap();
static void print_seglist();
int check_heap();
static void* coalesce(void*ptr);
static void add_free(void* ptr);
static void delete_free(void* ptr);
static void* find_fit(size_t asize);
static void place(void* ptr, size_t asize);
static void* extend_heap(size_t asize);

/***************************************************************
 * functions for debug
 * be careful in use since they cause bad performance(throughput)
 **************************************************************/

static void print_heap()
{
	printf("\nPrinting overall heap structure...\n");
	for(void* ptr1 = heap_h; ptr1 < HEAP_T; ptr1 = NEXT_BLKP(ptr1))
	{
		for(void* ptr2 = HDRP(ptr1); ptr2 <= FTRP(ptr1); ptr2 += WSIZE)
		{
			if(ptr2 == HDRP(ptr1))
			{
				printf("[%x][HDR][size = %d][a = %d]\n", ptr2, GET_SIZE(ptr2), GET_ALLOC(ptr2));
			}
			else if(ptr2 == FTRP(ptr1))
			{
				printf("[%x][FTR][size = %d][a = %d]\n", ptr2, GET_SIZE(ptr2), GET_ALLOC(ptr2));
			}
		}
	}
	
	return;
}

static void print_seglist()
{
	printf("\nPrinting overall seglist structure...\n");
	for(int i = 0; i < MAX_SEGS; i++)
	{
		printf("[%d]\n", i);
		for(void* ptr = SEG(i); ptr != NULL; ptr = GET_PRED(ptr))
		{
			printf("\t[%x][%d]\n", ptr, GET_SIZE(HDRP(ptr)));
		}
	}
	
	return;
}

int mm_check(void)
{
	/* loop in seglist checks...
	 *  - is every block in the free list marked as free?
	 *  - are there any contiguous free blocks
	 *    that somehow escaped coalescing?
	 *  - do the pointers in the free list point to valid free blocks?
	 */ 
	for(int i = 0; i < MAX_SEGS; i++)
	{
		for(void* ptr = SEG(i); ptr != NULL; ptr = GET_PRED(ptr))
		{
			if(GET_ALLOC(HDRP(ptr)) || GET_ALLOC(FTRP(ptr)))
			{
				printf("heap is not consistent in [seglist] area.\n");
				return 0;
			}
			if(!GET_ALLOC(HDRP(PREV_BLKP(ptr))) || HDRP(NEXT_BLKP(ptr)) < HEAP_T && !GET_ALLOC(HDRP(NEXT_BLKP(ptr))))
			{
				printf("heap is not consistent in [seglist] area.\n");
				return 0;
			}
		}
	}
	/* loop in heaplist checks...
	 *  - do any allocated blocks overlap?
	 *  - do the pointers in a heap block point to valid heap addresses?
	 */ 
	for(void* ptr1 = heap_h; ptr1 < HEAP_T; ptr1 = NEXT_BLKP(ptr1))
	{
		if(FTRP(ptr1) > HDRP(NEXT_BLKP(ptr1)))
		{
			printf("heap is not consistent in [heaplist] area.\n");
			return 0;
		}
	}

	return 1;
}

/***************************************************************
 * helper static functions
 **************************************************************/

/* 
 * coalesce - it manages out of boundary issue
 *            with comparing block pointer and HEAP_H, HEAP_T
 *            although comparison with HEAP_H is completely useless.
 */
static void* coalesce(void* ptr)
{
	size_t prev_alloc;
	size_t next_alloc;
	size_t size = GET_SIZE(HDRP(ptr));
	
	if(PREV_BLKP(ptr) < HEAP_H) prev_alloc = 1;
	else prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	if(NEXT_BLKP(ptr) > HEAP_T) next_alloc = 1;
	else next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	if(prev_alloc && next_alloc)
	{
		/* A - F - A */
		return ptr;
	}
	else if(prev_alloc && !next_alloc)
	{
		/* A - F - F */
		delete_free(ptr);
		delete_free(NEXT_BLKP(ptr));
		size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
		add_free(ptr);
		return ptr;
	}
	else if(!prev_alloc && next_alloc)
	{
		/* F - F - A */
		delete_free(PREV_BLKP(ptr));
		delete_free(ptr);
		size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		PUT(FTRP(PREV_BLKP(ptr)), PACK(size, 0));
		add_free(PREV_BLKP(ptr));
		return PREV_BLKP(ptr);
	}
	else
	{
		/* F - F - F */
		delete_free(PREV_BLKP(ptr));
		delete_free(ptr);
		delete_free(NEXT_BLKP(ptr));
		size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		PUT(FTRP(PREV_BLKP(ptr)), PACK(size, 0));
		add_free(PREV_BLKP(ptr));
		return PREV_BLKP(ptr);
	}
}

/* 
 * find_seg - use >> operation to caculate 2^k size.
 *            segregated list can be optimized more if i starts from -1.
 */
static int find_seg(size_t size)
{
	int i;
	
	for(i = 0; size > 1 && i < 19; i++)
	{
		size = size >> 1;
	}
	
	return i;
}

/* 
 * add_free - add block in free list,
 *            size and alloc bit in hdr, ftr should be changed already.
 */
static void add_free(void* ptr)
{
	int i = find_seg(GET_SIZE(HDRP(ptr)));
	
	/* BLOCK -> PRED  */
	/* SUCC  <- BLOCK */
	if(SEG(i) == NULL)
	{
		/* no block in this seglist */
		SET_PRED(ptr, NULL);
		SET_SUCC(ptr, NULL);
		SEG(i) = ptr;
	}
	else
	{
		/* yes block in this seglist */
		SET_PRED(ptr, SEG(i));
		SET_SUCC(ptr, NULL);
		SET_SUCC(SEG(i), ptr);
		SEG(i) = ptr;
	}
}

/* 
 * add_free - deete block in free list,
 *            size and alloc bit in hdr, ftr should not be changed now.
 */
static void delete_free(void* ptr)
{
	int i = find_seg(GET_SIZE(HDRP(ptr)));
	
	/* BLOCK -> PRED  */
	/* SUCC  <- BLOCK */
	if(SEG(i) == ptr)
	{
		if(GET_PRED(ptr) == NULL)
		{
			/* [B] */
			SEG(i) = NULL;
			SET_PRED(ptr, NULL);
			SET_SUCC(ptr, NULL);
		}
		else
		{
			/* [B] - ... */
			SET_SUCC(GET_PRED(ptr), NULL);
			SEG(i) = GET_PRED(ptr);
			SET_PRED(ptr, NULL);
			SET_SUCC(ptr, NULL);
		}
	}
	else
	{
		if(GET_PRED(ptr) == NULL)
		{
			/* ... - [B] */
			SET_PRED(GET_SUCC(ptr), NULL);
			SET_PRED(ptr, NULL);
			SET_SUCC(ptr, NULL);
		}
		else
		{
			/* ... - [B] - ... */
			SET_SUCC(GET_PRED(ptr), GET_SUCC(ptr));
			SET_PRED(GET_SUCC(ptr), GET_PRED(ptr));
			SET_PRED(ptr, NULL);
			SET_SUCC(ptr, NULL);
		}
	}
	
	return;
}

/* 
 * place - splits the free block when there will be allocated block,
 *         but do not split if block left behind is too small. 
 */
static void place(void* ptr, size_t asize)
{
	size_t old = GET_SIZE(HDRP(ptr));
	
	delete_free(ptr);
	if(old - asize < 2 * DSIZE)
	{
		PUT(HDRP(ptr), PACK(old, 1));
		PUT(FTRP(ptr), PACK(old, 1));
	}
	else
	{
		PUT(HDRP(ptr), PACK(asize, 1));
		PUT(FTRP(ptr), PACK(asize, 1));
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(old - asize, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(old - asize, 0));
		add_free(NEXT_BLKP(ptr));
	}
	
	return;	
}

/* 
 * find_fit - returns pointer to a block if there is proper one in seglist.
 */
static void* find_fit(size_t asize)
{
	for(int i = find_seg(asize); &SEG(i) != FTRP(heap_h); i++)
	{
		if(SEG(i) != NULL)
		{
			for(void* ptr = SEG(i); ptr != NULL; ptr = GET_PRED(ptr))
			{
				if(GET_SIZE(HDRP(ptr)) >= asize)
				{
					return ptr;
				}
			}
		}
	}
	
	return NULL;
}

/* 
 * extend_heap - extend heap by asize,
 *               asize should be legal size.
 */
static void* extend_heap(size_t asize)
{
	void* ptr;

	if((ptr = mem_sbrk(asize)) == (void*) NULL) return (void*) -1;
	else
	{
		/* should set extended heap as a free block */
		PUT(HDRP(ptr), PACK(asize, 0));
		PUT(FTRP(ptr), PACK(asize, 0));
		add_free(ptr);
		/* add extended heap in overall heap size */
		HEAP_INCR(HEAP_SIZE + asize);
		/* coalesce after calculating size */
		ptr = coalesce(ptr);
	}
	
	return ptr;
}

/* 
 * mm_init - initialize the malloc package and seglist.
 */
int mm_init(void)
{
	size_t asize = ALIGN(MAX_SEGS * WSIZE + 2 * WSIZE + SIZE_T_SIZE);
	
	if((heap_h = mem_sbrk(asize)) == (void*)-1) return -1;
	else
	{
		PUT(heap_h, 0);
		PUT(HDRP(heap_h), PACK(asize, 1));
		PUT(FTRP(heap_h), PACK(asize, 1));
		HEAP_INCR(asize);
	}
	for(int i = 0; i < MAX_SEGS; i++)
	{
		SEG(i) = NULL;
	}
	
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void* mm_malloc(size_t size)
{
    int asize;
    void* ptr;
    size_t extendsize;
    
    if(size == 0) return NULL;
    asize = ALIGN(size + SIZE_T_SIZE);
    if((ptr = find_fit(asize)) == NULL || ptr >= HEAP_T)
    {
    	if(!GET_ALLOC(HEAP_T - WSIZE))
    	{
    		extendsize = asize - GET_SIZE(HEAP_T - WSIZE);
    	}
    	else
    	{
    		extendsize = asize;
    	}
    	if((ptr = extend_heap(extendsize)) == (void*)-1) return NULL;
    }
    place(ptr, asize);
    //printf("\n[malloc][%x][%d = %x]\n", ptr, asize, asize);
    //print_heap();
    //print_seglist();
    //if(!mm_check()) printf("There was an inconsistency in heap, please check\n");
    
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
	PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
	add_free(ptr);
	coalesce(ptr);
	//printf("\n[ free ][%x][%d = %x]\n\n", ptr, GET_SIZE(HDRP(ptr)), GET_SIZE(HDRP(ptr)));
    //print_heap();
    //print_seglist();
    //if(!mm_check()) printf("There was an inconsistency in heap, please check\n");
	
	return;
}

/*
 * mm_realloc - Implemented in cases.
 */
void* mm_realloc(void *ptr, size_t size)
{
	void* oldptr = ptr;
	void* newptr = ptr;
    size_t asize = ALIGN(size + SIZE_T_SIZE);
    
    if(ptr == NULL)
    {
    	newptr = mm_malloc(size);
    	return newptr;
    }
    if(size == 0)
    {
    	mm_free(ptr);
    	return NULL;
    }
    if(asize <= GET_SIZE(HDRP(oldptr))) /* block will remain as it was */
    {
    	return oldptr;
    }
    if(NEXT_BLKP(oldptr) < HEAP_T && !GET_ALLOC(HDRP(NEXT_BLKP(oldptr))) && GET_SIZE(HDRP(NEXT_BLKP(oldptr))) + GET_SIZE(HDRP(oldptr)) > size)
    {
    	place(NEXT_BLKP(oldptr), asize - GET_SIZE(HDRP(oldptr)));
    	PUT(HDRP(oldptr), PACK(asize, 1));
    	PUT(FTRP(oldptr), PACK(asize, 1));
    }
    else
    {
    	newptr = mm_malloc(size);
    	memcpy(newptr, oldptr, GET_SIZE(HDRP(oldptr)));
    	mm_free(oldptr);
    }
    return newptr;
}
