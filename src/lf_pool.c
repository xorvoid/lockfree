#include "lf_pool.h"
#include "lf_util.h"

struct __attribute__((aligned(CACHE_LINE_SZ))) lf_pool
{
  size_t   num_elts;
  size_t   elt_sz;
  u64      tag_next;
  u64      _pad1;
  lf_ref_t head;
  char     _pad2[CACHE_LINE_SZ - 2*sizeof(size_t) - 2*sizeof(u64) - sizeof(lf_ref_t)];

  char   mem[];
};
static_assert(sizeof(lf_pool_t) == CACHE_LINE_SZ, "");
static_assert(alignof(lf_pool_t) == CACHE_LINE_SZ, "");

lf_pool_t * lf_pool_new(size_t num_elts, size_t elt_sz)
{
  size_t mem_sz;
  lf_pool_footprint(num_elts, elt_sz, &mem_sz, NULL);

  void * mem = malloc(mem_sz);;
  if (!mem) return NULL;

  return lf_pool_mem_init(mem, num_elts, elt_sz);
}

void lf_pool_delete(lf_pool_t *pool)
{
  free(pool);
}

void *lf_pool_acquire(lf_pool_t *pool)
{
  while (1) {
    lf_ref_t cur = pool->head;
    if (LF_REF_IS_NULL(cur)) return NULL;

    void *   elt  = (void*)cur.val;
    lf_ref_t next = *(lf_ref_t*)elt;

    if (!LF_REF_CAS(&pool->head, cur, next)) {
      LF_PAUSE();
      continue;
    }
    return elt;
  }
}

void lf_pool_release(lf_pool_t *pool, void *elt)
{
  u64      tag  = LF_ATOMIC_INC(&pool->tag_next);
  lf_ref_t next = LF_REF_MAKE(tag, (u64)elt);

  while (1) {
    lf_ref_t cur = pool->head;
    *(lf_ref_t*)elt = cur;

    if (!LF_REF_CAS(&pool->head, cur, next)) {
      LF_PAUSE();
      continue;
    }
    return;
  }
}

void lf_pool_footprint(size_t num_elts, size_t elt_sz, size_t *_size, size_t *_align)
{
  elt_sz = LF_ALIGN_UP(elt_sz, alignof(u128));

  if (_size)  *_size  = sizeof(lf_pool_t) + elt_sz * num_elts;
  if (_align) *_align = alignof(lf_pool_t);
}

lf_pool_t * lf_pool_mem_init(void *mem, size_t num_elts, size_t elt_sz)
{
  if (elt_sz == 0) return NULL;
  elt_sz = LF_ALIGN_UP(elt_sz, alignof(u128));

  lf_pool_t *lf_pool = (lf_pool_t *)mem;
  lf_pool->num_elts = num_elts;
  lf_pool->elt_sz   = elt_sz;
  lf_pool->tag_next = 0;
  lf_pool->head     = LF_REF_NULL;

  char *ptr = lf_pool->mem + num_elts * elt_sz;
  for (size_t i = num_elts; i > 0; i--) {
    ptr -= elt_sz;
    lf_pool_release(lf_pool, ptr);
  }

  return lf_pool;
}

lf_pool_t * lf_pool_mem_join(void *mem, size_t num_elts, size_t elt_sz)
{
  lf_pool_t *lf_pool = (lf_pool_t *)mem;
  if (num_elts != lf_pool->num_elts) return NULL;
  if (elt_sz != lf_pool->elt_sz) return NULL;
  return lf_pool;
}

void lf_pool_mem_leave(lf_pool_t *lf_pool)
{
  /* no-op at the moment */
}
