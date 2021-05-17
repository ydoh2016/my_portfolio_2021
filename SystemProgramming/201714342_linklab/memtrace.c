//------------------------------------------------------------------------------
//
// memtrace
//
// trace calls to the dynamic memory manager
//
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memlog.h>
#include <memlist.h>

//
// function pointers to stdlib's memory management functions
//
static void *(*mallocp)(size_t size) = NULL;
static void (*freep)(void *ptr) = NULL;
static void *(*callocp)(size_t nmemb, size_t size);
static void *(*reallocp)(void *ptr, size_t size);

//
// statistics & other global variables
//
static unsigned long n_malloc  = 0;
static unsigned long n_calloc  = 0;
static unsigned long n_realloc = 0;
static unsigned long n_allocb  = 0;
static unsigned long n_freeb   = 0;
static item *list = NULL;

//
// init - this function is called once when the shared library is loaded
//
__attribute__((constructor))
void init(void)
{
  char *error;

  LOG_START();

  // initialize a new list to keep track of all memory (de-)allocations
  // (not needed for part 1)
  list = new_list();
}

//
// fini - this function is called once when the shared library is unloaded
//
__attribute__((destructor))
void fini(void)
{
  LOG_STATISTICS(n_allocb, n_allocb / (n_malloc + n_calloc + n_realloc), n_freeb);
  if(n_allocb > n_freeb)
  {
    LOG_NONFREED_START();
    item *i = list -> next;
    while(i != NULL)
    {
      if(i -> cnt)
      {
        LOG_BLOCK(i -> ptr, i -> size, i -> cnt);
      }
      i = i -> next;
    }
  }
  LOG_STOP();

  // free list (not needed for part 1)
  free_list(list);
}

//
// wrapper for malloc
//
void *malloc (size_t size)
{
  char *error;
  void *ret;

  if(!mallocp)
  {
    mallocp = dlsym(RTLD_NEXT, "malloc");
    if((error = dlerror()) != NULL)
    {
      fputs(error, stderr);
      exit(1);
    }
  }
  ret = mallocp(size);
  LOG_MALLOC(size, ret);
  n_malloc++;
  n_allocb += alloc(list, ret, size) -> size;
  return ret;
}

//
// wrapper for free
//
void free (void *ptr)
{
  char *error;

  if(!freep)
  {
    freep = dlsym(RTLD_NEXT, "free");
    if((error = dlerror()) != NULL)
    {
      fputs(error, stderr);
      exit(1);
    }
  }
  if(find(list, ptr) == NULL) // deal with case : ILLEGAL FREE
  {
    LOG_FREE(ptr);
    LOG_ILL_FREE();
    return;
  }
  if(find(list, ptr) -> cnt == 0) // deal with case : DOUBLE FREE
  {
    LOG_FREE(ptr);
    LOG_DOUBLE_FREE();
    return;
  }
  freep(ptr);
  LOG_FREE(ptr);
  n_freeb += dealloc(list, ptr) -> size;
  return;
}  

//
// wrapper for calloc
//
void *calloc (size_t nmemb, size_t size)
{
  char *error;
  void *ret;

  if(!callocp)
  {
    callocp = dlsym(RTLD_NEXT, "calloc");
    if((error = dlerror()) != NULL)
    {
      fputs(error, stderr);
      exit(1);
    }
  }
  ret = callocp(nmemb, size);
  LOG_CALLOC(nmemb, size, ret);
  n_calloc++;
  n_allocb += alloc(list, ret, nmemb * size) -> size;
  return ret;
}

//
// wrapper for realloc
//
void *realloc (void *ptr, size_t size)
{
  char *error;
  void *ret;

  if(!reallocp)
  {
    reallocp = dlsym(RTLD_NEXT, "realloc");
    if((error = dlerror()) != NULL)
    {
      fputs(error, stderr);
      exit(1);
    }
  }
  if(find(list, ptr) == NULL) // deal with case : ILLEGAL REALLOC
  {
    ret = reallocp(NULL, size);
    LOG_REALLOC(ptr, size, ret);
    LOG_ILL_FREE();
    n_realloc++;
    n_allocb += alloc(list, ret, size) -> size;
    return ret;
  }
  if(find(list, ptr) -> cnt == 0) // deal with case : DOUBLE REALLOC
  {
    ret = reallocp(NULL, size);
    LOG_REALLOC(ptr, size, ret);
    LOG_DOUBLE_FREE();
    n_realloc++;
    n_allocb += alloc(list, ret, size) -> size;
    return ret;
  }
  if(find(list, ptr) -> size >= size) // Deal with case: new size <= old size
  {
    ret = ptr;
    LOG_REALLOC(ptr, size, ret);
    return ret;
  }
  ret = reallocp(ptr, size);
  LOG_REALLOC(ptr, size, ret);
  n_realloc++;
  n_freeb += dealloc(list, ptr) -> size;
  n_allocb += alloc(list, ret, size) -> size;
  return ret;
}
