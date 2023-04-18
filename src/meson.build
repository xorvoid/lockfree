
SRC = [
  'lf_pool.c',
]

liblockfree = static_library('lockfree', SRC)

executable('test_pool_stress', 'test_pool_stress.c', link_with: liblockfree)
