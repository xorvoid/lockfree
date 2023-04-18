#include <assert.h>
#include <limits.h>
#include <stdlib.h>

// Machine is byte-addressable
static_assert(CHAR_BIT == 8, "");

// Sizes / Offsets / Alignments are 64-bit
static_assert(sizeof(size_t) == 8, "");

// Assuming we have 64-bit pointers
static_assert(sizeof(void*) == 8, "");
