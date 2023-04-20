# Lockfree

A collection of some lockfree datastructures

## Datastructures

- Pool: A simple lockfree memory/object pool based on the Treiber Stack
- Queue: A simple lockfree MPMC queue based very roughly on Michael&Scott queues
- Bcast: A simple lockfree MPMC "broadcast-y" fan-out pub-sub queue based even more roughly on Michael&Scott queues

## Properties

- Lockfree
- Non-allocating (after init)
- Fixed-size
- Cache-efficent flat structures
- Agnostic to in-process memory or shared-memory
- Crash-safe (kill -9 cannot cause deadlocks, livelocks, corruption, etc)
