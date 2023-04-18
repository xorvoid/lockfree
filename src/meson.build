
SRC = [
  'lf_pool.c',
  'lf_queue.c',
]

liblockfree = static_library('lockfree', SRC)

executable('test_pool_stress', 'test_pool_stress.c', link_with: liblockfree)
executable('test_queue_functional', 'test_queue_functional.c', link_with: liblockfree)
#executable('test_queue_stress', 'test_queue_stress.c', link_with: liblockfree)
