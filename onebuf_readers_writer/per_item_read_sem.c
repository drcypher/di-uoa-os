/**
 * per_item_read_sem.c
 * 
 * Protocol implementation with one read semaphore per item in the itemArray.
 *
 * For an optimized variation using a constant number of two read semaphores see
 * swap_read_sem.c protocol.
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
static sem_t sharedSemMayReadPerItem[ITEM_COUNT];

/**
 * Safely read a value from the sharedSingleItem variable.
 *
 * This functions follows the per item read semaphore protocol.
 *
 * @param threadName Name of reader thread reading a value
 * @param itemId The position of the item in the initial buffer
 * @return Read value
 */
int protocolReadValue(const char *threadName, int itemId)
{
	int itemValue;

	fprintf(stderr, "%s Waiting to read item with id=%d from the shared buffer\n", threadName, itemId);

	// Wait until the writer has written out the value of itemId in the shared buffer
	sem_wait(&sharedSemMayReadPerItem[itemId]);

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
		sem_post(&sharedSemMayReadPerItem[itemId]);
	}

	return itemValue;
}

/**
 * Safely write a value to the sharedSingleItem variable so it's read by readers.
 *
 * Using this function guarantee synchronization and protection as long as the readers also use
 * perItemSemaphoreReadValue.
 *
 * @param threadName Name of writer thread writing out the item value
 * @param itemId The position of the item in the initial buffer
 * @param itemValue The value of the item being exchanged with the readers
 */
void protocolWriteValue(const char *threadName, int itemId, int itemValue)
{
	fprintf(stderr, "%s Waiting for readers to complete reading\n", threadName);

	// Wait until all readers are done reading
	sem_wait(&sharedSemMayWrite);

	// Write the item value to the shared variable
	exchangeBufferWriteValue(threadName, itemId, itemValue);

	fprintf(stderr, "%s Signaling readers to resume reading on item with id=%d\n", threadName, itemId);

	// Signal readers so they start reading
	sem_post(&sharedSemMayReadPerItem[itemId]);
}

/**
 * Initializes shared variables and semaphores.
 */
void protocolInit()
{
	int i;

	// No readers are initially active
	sharedFinishedReaderCount = 0;

	// Writer may start doing work right away
	sem_init(&sharedSemMayWrite, 0, 1);

	// Initialize per item semaphores to 0 (not usable yet)
	for (i = 0; i < ITEM_COUNT; i++) {
		sem_init(&sharedSemMayReadPerItem[i], 0, 0);
	}
}
