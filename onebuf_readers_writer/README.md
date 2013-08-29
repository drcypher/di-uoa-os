# Writer and Readers with single-value exchange buffer

## Workflow

The program consists of a single writer, owning an `itemArray` with `ITEM_COUNT` elements. There
are also many readers who should sequentially get all items from `itemArray` through a designated
`exchangeBuffer` which may host only a single item value at a time.

The involved structures/entities are the following:

1. The item array (`item_array.c`). This should be owned only by the writer thread, but for the
sake of testing we've made it globally available. Still, readers only access it in a controlled
manner, i.e. using `itemArrayIsValueCorrect()`.

2. The single-place exchange buffer (`exchange_buffer.c`). This allows the exchange of data
among writer and readers. It's not free of race conditions, so it must be wrapped in a synchronization
protocol.

3. A wrapper implementing synchronization using one semaphore for each item in the array (`per_item_read_sem.c`)

4. An optimized variation of the above wrapper taking advantag of the sequential retrieval
of array items, thus utilizing a single pair of swapping read semaphores (`swap_item_sem.c`)


## Dependencies

1. POSIX Threads
2. POSIX Semaphores


## Compilation

If you want to compile against the swapping semaphore implementation, give

```
gcc -pthread main.c exchange_buffer.c item_array.c swap_read_sem.c
```

If you want to test the per-item semaphore implementation, change the above to

```
gcc -pthread main.c exchange_buffer.c item_array.c per_item_read_sem.c
```

If you ever add your own protocol implementation, just replace `per_item_read_sem.c` with your own
implementation.


## License

This code is public domain. I hope I helped you dealing with your Operating Systems course a bit :)
