/**
 * main.c
 *
 *
 *
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include "main.h"

/**
 * Writer thread task.
 *
 * Tries to sequentially write contents of protectedBuffer out to the readers using the shared
 * variable "sharedSingleItem".
 *
 * Due to assertion requirements explained in "readerThreadTask", the proper values are read
 * from the protectedBuffer which is actually shared. If no assertions were required then the
 * actualy values exchanged would be produced in a local buffer of the writer thread, probably
 * within this function.
 *
 * Note that this is a skeleton function controlling the flow of execution. The interesting
 * stuff is actually implemented in the perItemSemaphoreWriteValue and unsafelyWriteValue functions.
 * 
 * @param threadId Id assigned to thread. Used to create a unique name for it
 * @return
 */
void *writerThreadTask(void *threadId)
{
	int i;
	char threadName[255];

	// Compile thread name
	sprintf(threadName, "[writer]");

	for (i = 0; i < ITEM_COUNT; i++) {
		protocolWriteValue(threadName, i, itemArrayReadValue(i));
	}

	return 0;
}

/**
 * Reader thread task.
 *
 * Tries to read ITEM_COUNT items given out by the writer, using the shared variable
 * "sharedSingleItem".
 *
 * The function used to read the values is defined in READ_VALUE_FUNCTION so you can easily
 * changed it.
 *
 * This function cheats by looking into the proper values so that it can assert whether everything
 * went ok or not. If it wasn't for the assertion, then no access to the protectedBuffer would
 * be necessary.
 *
 * Note that this is a skeleton function controlling the flow of execution. The interesting
 * stuff is actually implemented in the perItemSemaphoreReadValue and unsafelyReadValue functions.
 *
 * @param threadId Id assigned to thread. Used to create a unique name for it
 * @return
 */
void *readerThreadTask(void *threadId)
{
	int i;
	int wrongReadCount = 0;
	char threadName[255];
	int localValues[ITEM_COUNT];
	const char *readResultString;

	// Compile thread name
	sprintf(threadName, "[reader %3ld]", (long)threadId);

	for (i = 0; i < ITEM_COUNT; i++) {

		// Read the value of item "i" using the selected READ_VALUE_FUNCTION
		localValues[i] = protocolReadValue(threadName, i);

		// Check if read value is correct. Here we're cheating by looking into
		// the real values from the protected buffer, but there's no other way anyway :)
		if (itemArrayIsValueCorrect(i, localValues[i])) {
			readResultString = "";
		} else {
			wrongReadCount++;
			readResultString = "***** FAIL ***** ";
		}

		fprintf(stderr, "%s%s Read item with id=%d, copied value = %d\n",
				readResultString, threadName, i, localValues[i]);
	}


	if (wrongReadCount == 0) {
		fprintf(stderr, "%s succeeded\n", threadName);
	} else {
		fprintf(stderr, "***** %s FAILED to read %d out of %d items correctly.\n",
				threadName, wrongReadCount, ITEM_COUNT);
	}
	return 0;
}

/**
 * Initializes shared variables and semaphores.
 *
 * Then fires up consumer & producer threads.
 */
int main()
{
	int i;

	//
	// Initialize buffers and related shared variables and semaphores
	//
	exchangeBufferInit();
	itemArrayInit("main");
	protocolInit();

	//
	// Create writer and reader threads and let them start work.
	//
	pthread_t writerThread;
	pthread_t readerThread[READERS_COUNT];

	pthread_create(&writerThread, NULL, writerThreadTask, ((void *) -1));
	for (i = 0; i < READERS_COUNT; i++) {
		pthread_create(&readerThread[i], NULL, readerThreadTask, ((void *) i));
	}

	//
	// Eventually join all threads
	//
	pthread_join(writerThread, NULL);
	for (i = 0; i < READERS_COUNT; i++) {
		pthread_join(readerThread[i], NULL);
	}

	return 0;
}
