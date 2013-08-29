# Consumers & Producers with Swappable Double Buffer

## Workflow

This program maintains a fixed-size swappable double buffer of integers used
concurrently by two thread groups:
1. A group of producer threads writing into the produce buffer
2. A group of consumer threads reading from the consume buffer

There are three rules, however:
1. Consumers read only from a single side of the buffer (`bufferProduceData()`)
2. Producers write only to a single side of the buffer (`bufferConsumeData()`)
3. When the producer's side has no space left (`bufferConsumeIsExhausted()`) and the consumer's side of
the buffer is exhausted (`bufferProduceIsExhausted()`), the buffer sides must be swapped (`bufferSwap()`)

## Concurrency

The module implementing the buffer data structure (`buffer*()` functions) encapsulates all
data interchange with the data structure but cannot be used concurrently without race conditions.

So there's a need for two wrapper functions which will handle synchronization and protection
consistently:
1. `protocolProduceData`
2. `protocolConsumeData`

There's also a `protocolInit` for optional internal initializations before concurrent execution begins.


## 3-semaphores protocol

In file `three_sem.c` a 3-semaphore protocol is implemented using the following semaphores:

1. `sharedSemMutex`: It makes sure only on thread (consumer or producer) messes with the buffers
   and their positions at any time to avoid race conditions.

2. `sharedSemMayConsume`: Used for notifying consumers of being able to consume data from the
   consume buffer, thus keeping them on wait while the consume buffer remains empty.

3. `sharedSemMayProduce`: Used for notifying producers of being able to produce data into the
   produce buffer, thus keeping them on wait while the produce buffer remains full.

## Other protocols

The skeleton of the application, i.e. `main.c` and `buffer.c` are written in such a way that
anybody implementing the three `protocol*` functions and linking them during compilation would
be able to try and error, without dealing with thread creation or data management details.

## Disclaimer

Although the program seems to serve its purpose correctly, no proper set of tests supports
this claim, so it's always possible I've missed something.

Also, this implementation of the consumer-producer paradigm with swappable buffers might not
be optimal, thus reducing concurrency while a better algorithm wouldn't. It's meant to demonstrate
the paradigm rather than provide an optimized solution to any problem.

Given the above, comments, corrections and design optimizations are always welcome :)


## Dependencies

1. POSIX Threads
2. POSIX Semaphores


## Compilation

```
gcc -pthread main.c buffer.c three_sem.c
```

If you ever add your own protocol implementation, just replace `three_sem.c` with your own
implementation.


## License

This code is public domain. I hope I helped you dealing with your Operating Systems course a bit :)