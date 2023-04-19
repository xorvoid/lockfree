#pragma once
#include <stdbool.h>
#include <stdlib.h>

bool   lf_shm_create(const char *name, size_t size);
void   lf_shm_remove(const char *name);
void * lf_shm_open(const char *name, size_t * _opt_size);
void   lf_shm_close(void *shm, size_t size);
