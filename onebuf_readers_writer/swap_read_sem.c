/**
 * swap_read_sem.c
 *
 * Protocol implementation with only two read semaphores being swapped (taking turns). This can only
 * work given there's an incrementing itemId driving the process and taking all consecutive values
 * from 0 up to the size of the itemArray.
 *
 * It's an optimized variation of the per_item_read_sem.c protocol.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#include <semaphore.h>
#include "main.h"

//
// Global (shared) variables
//

// Id of reader currently reading data (varies between 0 and READERS_MAX)
static int sharedFinishedReaderCount;

// Signaling semaphore telling writer he can proceed to writing next item in shared buffer
static sem_t sharedSemMayWrite;

// Signaling semaphore telling readers they can start reading the item with index equal
// to the semaphore index (aka there's one semaphore per data item)
static sem_t sharedSemMayReadSwap[2];

/**
 * Safely read a value from the sharedSingleItem variable.
 *
 * This functions follows the swap read semaphore protocol.
 *
 * @param threadName Name of reader thread reading a value
 * @param itemId The position of the item in the initial buffer
 * @return Read value
 */
int protocolReadValue(const char *threadName, int itemId)
{
	int itemValue;
	int readSemaphoreId = itemId % 2;

	fprintf(stderr, "%s Waiting on semaphore %d to read item with id=%d from the shared buffer\n", threadName, readSemaphoreId, itemId);

	// Wait until the writer has written out the value of itemId in the shared buffer
	sem_wait(&sharedSemMayReadSwap[readSemaphoreId]);

	// Read value from exchange buffer
	itemValue = exchangeBufferReadValue(threadName, itemId);

	// Update the number of readers that have read the currently shared value
	sharedFinishedReaderCount++;

	if (sharedFinishedReaderCount == READERS_COUNT) {
		// We were the last reader

		// Reset number of readers for the next round
		sharedFinishedReaderCount = 0;

		// Signal the writer to write the next value, but not the readers, as they might
		// have the chance to read the old value in the shared buffer, before even the
		// writer manages to write the new value
		sem_post(&sharedSemMayWrite);
	} else {
		// Signal the next reader who's waiting to read this item
		sem_post(&sharedSemMayReadSwap[readSemaphoreId]);
	}

	return itemValue;
}

/**
 * Safely write a value to the sharedSingleItem variable so it's read by readers.
 *
 * @param threadName Name of writer thread writing out the item value
 * @param itemId The position of the item in the initial buffer
 * @param itemValue The value of the item being exchanged with the readers
 */
void protocolWriteValue(const char *threadName, int itemId, int itemValue)
{
	int readSemaphoreId = itemId % 2;

	fprintf(stderr, "%s Waiting for readers to complete reading\n", threadName);

	// Wait until all readers are done reading
	sem_wait(&sharedSemMayWrite);

	// Write the item value to the shared variable
	exchangeBufferWriteValue(threadName, itemId, itemValue);

	fprintf(stderr, "%s Signaling readers to resume reading on item with id=%d on readSemaphoreId=%d\n",
			threadName, itemId, readSemaphoreId);

	// Signal readers so they start reading
	sem_post(&sharedSemMayReadSwap[readSemaphoreId]);
}


/**
 * Initializes shared variables and semaphores.
 */
void protocolInit()
{
	// No readers are initially active
	sharedFinishedReaderCount = 0;

	// Writer may start doing work right away
	sem_init(&sharedSemMayWrite, 0, 1);

	// Initialize swap read semaphore to 0 (not usable yet)
	sem_init(&sharedSemMayReadSwap[0], 0, 0);
	sem_init(&sharedSemMayReadSwap[1], 0, 0);
}
