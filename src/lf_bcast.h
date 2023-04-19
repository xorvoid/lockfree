#pragma once
#include <stdbool.h>
#include <stdlib.h>

/* Lockfree Bcast: multi-publisher broadcast to multi-consumer */

typedef struct lf_bcast     lf_bcast_t;
typedef struct lf_bcast_sub lf_bcast_sub_t;
struct __attribute__((aligned(16))) lf_bcast_sub { char _opaque[32]; };

/* Basic API */
lf_bcast_t * lf_bcast_new(size_t depth, size_t max_msg_sz);
void         lf_bcast_delete(lf_bcast_t *bcast);
bool         lf_bcast_pub(lf_bcast_t *b, void * msg, size_t msg_sz);
void         lf_bcast_sub_begin(lf_bcast_sub_t *sub, lf_bcast_t *b);
bool         lf_bcast_sub_next(lf_bcast_sub_t *sub, void * msg_buf, size_t * _out_msg_sz);
