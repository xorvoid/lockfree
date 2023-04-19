#include "lf_bcast.h"
#include "lf_pool.h"
#include "lf_util.h"

#define ESTIMATED_PUBLISHERS 16

struct __attribute__((aligned(CACHE_LINE_SZ))) lf_bcast
{
  u64         depth_mask;
  u64         head_idx;
  u64         tail_idx;
  lf_pool_t * pool; // TODO REMOVE ME SO WE CAN LIVE IN SHM
  char        _pad_2[CACHE_LINE_SZ - 3*sizeof(u64) - sizeof(lf_pool_t*)];

  lf_ref_t slots[];
};
static_assert(sizeof(lf_bcast_t) == CACHE_LINE_SZ, "");
static_assert(alignof(lf_bcast_t) == CACHE_LINE_SZ, "");

lf_bcast_t * lf_bcast_new(size_t depth, size_t max_msg_sz)
{
  if (!LF_IS_POW2(depth)) return NULL;

  size_t mem_sz = sizeof(lf_bcast_t) + depth * sizeof(lf_ref_t);

  lf_bcast_t *b = (lf_bcast_t*)malloc(mem_sz);
  b->depth_mask = depth-1;
  b->head_idx = 1; /* Start from 1 because we use 0 to mean "unused" */
  b->tail_idx = 1; /* Start from 1 because we use 0 to mean "unused" */
  b->pool = lf_pool_new(depth+ESTIMATED_PUBLISHERS, max_msg_sz+sizeof(size_t));

  memset(b->slots, 0, depth * sizeof(lf_ref_t));

  return b;
}

void lf_bcast_delete(lf_bcast_t *bcast)
{
  free(bcast);
}

static void try_drop_head(lf_bcast_t *b, u64 head_idx)
{
  // TODO AUDIT
  lf_ref_t head_cur = b->slots[head_idx & b->depth_mask];
  if (!LF_U64_CAS(&b->head_idx, head_idx, head_idx+1)) return;

  // FIXME: THIS IS WHERE WE MIGHT LOSE SHARED RESOURCES
  void *elt = (void*)head_cur.val;
  lf_pool_release(b->pool, elt);
}

bool lf_bcast_pub(lf_bcast_t *b, void * msg, size_t msg_sz)
{
  char *elt = lf_pool_acquire(b->pool);
  if (!elt) return false; // out of elements

  memcpy(elt, &msg_sz, sizeof(msg_sz));
  memcpy(elt+sizeof(msg_sz), msg, msg_sz);

  // TODO APPEND ELT

  while (1) {
    u64        head_idx = b->head_idx;
    u64        tail_idx = b->tail_idx;
    lf_ref_t * tail_ptr = &b->slots[tail_idx & b->depth_mask];
    lf_ref_t   tail_cur = *tail_ptr;
    LF_BARRIER_ACQUIRE();

    // Stale tail pointer? Try to advance it..
    if (tail_cur.tag == tail_idx) {
      LF_U64_CAS(&b->tail_idx, tail_idx, tail_idx+1);
      LF_PAUSE();
      continue;
    }

    // Stale tail_idx? Try again..
    if (tail_cur.tag >= tail_idx) {
      LF_PAUSE();
      continue;
    }

    // Slot currently used.. full.. roll off the head
    if (head_idx <= tail_cur.tag) {
      assert(head_idx == tail_cur.tag);
      try_drop_head(b, head_idx);
      LF_PAUSE();
      continue;
    }

    // Otherwise, try to append the tail
    lf_ref_t tail_next = LF_REF_MAKE(tail_idx, (u64)elt);
    if (!LF_REF_CAS(tail_ptr, tail_cur, tail_next)) {
      LF_PAUSE();
      continue;
    }

    // Success, try to update the tail. If we fail, it's okay.
    LF_U64_CAS(&b->tail_idx, tail_idx, tail_idx+1);
    return true;
  }
}

typedef struct sub_impl sub_impl_t;
struct __attribute__((aligned(16))) sub_impl
{
  lf_bcast_t * bcast;
  u64          idx;
  char         _extra[16];
};
static_assert(sizeof(sub_impl_t) == sizeof(lf_bcast_sub_t), "");
static_assert(alignof(sub_impl_t) == alignof(lf_bcast_sub_t), "");

void lf_bcast_sub_begin(lf_bcast_sub_t *_sub, lf_bcast_t *b)
{
  sub_impl_t *sub = (sub_impl_t*)_sub;
  sub->bcast = b;
  sub->idx = b->head_idx;
}

bool lf_bcast_sub_next(lf_bcast_sub_t *_sub, void * msg_buf, size_t * _out_msg_sz, size_t *_out_drops)
{
  sub_impl_t *sub   = (sub_impl_t*)_sub;
  lf_bcast_t *b     = sub->bcast;
  size_t      drops = 0;

  while (1) {
    if (sub->idx == b->tail_idx) return false;

    lf_ref_t * ref_ptr = &b->slots[sub->idx & b->depth_mask];
    lf_ref_t   ref     = *ref_ptr;

    LF_BARRIER_ACQUIRE();

    if (ref.tag != sub->idx) { // We've fallen behind and the message we wanted was dropped?
      sub->idx++;
      drops++;
      LF_PAUSE();
      continue;
    }
    char * elt    = (char*)ref.val;
    size_t msg_sz = *(size_t*)elt;
    memcpy(msg_buf, elt+sizeof(size_t), msg_sz);

    LF_BARRIER_ACQUIRE();

    lf_ref_t ref2 = *ref_ptr;
    if (!LF_REF_EQUAL(ref, ref2)) { // Data changed while reading? Drop it.
      sub->idx++;
      drops++;
      LF_PAUSE();
      continue;
    }

    sub->idx++;
    *_out_msg_sz = msg_sz;
    *_out_drops  = drops;
    return true;
  }
}
