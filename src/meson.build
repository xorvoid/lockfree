
SRC = [
  'lf_pool.c',
  'lf_queue.c',
  'lf_bcast.c',
  'lf_shm.c',
]

liblockfree = static_library('lockfree', SRC)

executable('test_pool_stress', 'test_pool_stress.c', link_with: liblockfree)
executable('test_queue_functional', 'test_queue_functional.c', link_with: liblockfree)
#executable('test_queue_stress', 'test_queue_stress.c', link_with: liblockfree)

executable('test_bcast_functional', 'test_bcast_functional.c', link_with: liblockfree)
executable('test_bcast_stress',     'test_bcast_stress.c',     link_with: liblockfree)
executable('test_bcast_shm_create', 'test_bcast_shm_create.c', link_with: liblockfree)
executable('test_bcast_shm_run',    'test_bcast_shm_run.c',    link_with: liblockfree)
