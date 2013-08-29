/**
 * Consumers & Producers with Swappable Shared Buffers.
 * 
 * 
 * Description:
 * --------------------------------
 * This program maintains two thread groups and two respective integer buffers of equal size:
 * 1. A group of producer threads writing into the produce buffer
 * 2. A group of consumer threads reading from the consume buffer
 * 
 * Initially, the two buffers (buffer 0 and buffer 1) are empty. Buffer 0 is considered the
 * consumption buffer (by setting sharedConsumeBufferId to 0) and buffer 1 is considered the
 * production buffer (sharedProduceBufferId to 1).
 * 
 * Since both buffers start empty, only producers are allowed to do work, i.e. produce random
 * numbers and put them in the produce buffer (see safeProduce function).
 * 
 * As soon as the produce buffer gets full, it's swapped with the (initially empty) consume buffer.
 * This is as easy as swapping the values of sharedConsumeBufferId and sharedProduceBufferId (see
 * safeSwapBuffers function).
 * 
 * Now consumers can also start working, consuming data elements from the consume buffer (see
 * safeConsume function).
 * 
 * Three semaphores are used for synchronization and protection of shared memory:
 * 1. sharedSemMutex: It makes sure only on thread (consumer or producer) messes with the buffers
 *    and their positions at any time to avoid race conditions.
 *
 * 2. sharedSemMayConsume: Used for notifying consumers of being able to consume data from the
 *    consume buffer, thus keeping them on wait while the consume buffer remains empty.
 * 
 * 3. sharedSemMayProduce: Used for notifying producers of being able to produce data into the
 *    produce buffer, thus keeping them on wait while the produce buffer remains full.
 *
 * Notice that variables defined in the global scope (outside any function) are shared, and their
 * names are purposely prefixed with the word "shared" for readability, so confusion with local
 * (thread-specific) variables is avoided while reading the algorithms.
 * 
 * Threads were used for concurrent executions since it does not require further shared memory
 * manipulation (shm_* functions). Of course you could always rewrite this using concurrent *processes*
 * (instead of threads) exchanging data through other shared memory mechanisms.
 * 
 * 
 * 
 * Disclaimer:
 * --------------------------------
 * Although the program seems to serve its purpose correctly, no proper set of tests supports
 * this claim, so it's always possible I've missed something.
 * 
 * Also, this implementation of the consumer-producer paradigm with swappable buffers might not
 * be optimal, thus reducing concurrency while a better algorithm wouldn't. It's meant to demonstrate
 * the paradigm rather than provide an optimized solution to any problem.
 * 
 * Given the above, comments, corrections and design optimizations are always welcome :)
 * 
 * 
 * Dependencies:
 * --------------------------------
 * 1. POSIX Threads
 * 2. POSIX Semaphores
 * 
 * 
 * Compilation command:
 * --------------------------------
 * gcc -pthread swap_conprod.c -o swap_conprod
 * (at least on my kubuntu workstation)
 * 
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//
// Configurable constants. Feel free to try different combinations. Avoid 0 values :)
//

// Number of producer threads to create
#define PRODUCERS_COUNT 3

// Number of consumer threads to create
#define CONSUMERS_COUNT 3

// Size of each buffer
#define BUFFER_SIZE 10

// Max value of produced integer items
#define MAX_ITEM_VALUE 300

//
// Global (shared) variables
//

// Id (0 or 1) of buffer currently used for consuming/producing respectively
int sharedConsumeBufferId;
int sharedProduceBufferId;

// Current seek position for next consume/produce action in consume/produce buffers respectively
int sharedConsumePos;
int sharedProducePos;

// The two swappable buffers keeping the integer item values
int sharedBuffers[2][BUFFER_SIZE];

// Mutex regulating exclusive access to buffer manipulation sections (critical sections)
sem_t sharedSemMutex;

// Signaling semaphore telling consumers if they can proceed with consuming items
sem_t sharedSemMayProduce;

// Signaling semaphore telling producers if they can proceed with producing items
sem_t sharedSemMayConsume;

/**
 * Swaps the ids of consume/produce buffer.
 * 
 * Also updates the respective seek positions.
 * 
 * This function should only be called from within a critical section protected
 * by the mutex, otherwise race conditions will occur.
 * 
 */
void safeSwapBuffers(const char *threadName)
{
	// Swap buffers
	int swapBufferId = sharedConsumeBufferId;
	sharedConsumeBufferId = sharedProduceBufferId;
	sharedProduceBufferId = swapBufferId;

	fprintf(stderr, "\t%s Swapping buffers: consume -> %d, produce -> %d\n",
		threadName, sharedConsumeBufferId, sharedProduceBufferId);

	// Fix positions
	sharedConsumePos = 0;
	sharedProducePos = 0;

	fprintf(stderr, "\t%s Signaling consumers and producers\n",
		threadName, sharedConsumeBufferId, sharedProduceBufferId);

	// Signal both consumers and producers
	sem_post(&sharedSemMayConsume);
	sem_post(&sharedSemMayProduce);
}

/**
 * Safely produce a new data item into the produce buffer.
 */
void safeProduce(const char *threadName, int data)
{
	fprintf(stderr, "%s Waiting on produce semaphore\n", threadName);

	// Wait until there's room for producing
	sem_wait(&sharedSemMayProduce);

	fprintf(stderr, "%s Waiting on mutex\n", threadName);

	// Found some room, get exclusive access to shared buffer variables
	sem_wait(&sharedSemMutex);

	fprintf(stderr, "%s Acquired mutex\n", threadName);

	// Push data to buffer
	sharedBuffers[sharedProduceBufferId][sharedProducePos] = data;

	fprintf(stderr, "\t%s Wrote new item value %d to buffer[%d][%d] (%d items left in buffer)\n",
		threadName, data, sharedProduceBufferId, sharedProducePos, BUFFER_SIZE - sharedProducePos - 1);

	// Update produce position
	sharedProducePos++;

	// Check if produce buffer got full
	if (sharedProducePos == BUFFER_SIZE) {
		fprintf(stderr, "\t%s Produce buffer exhausted\n", threadName);

		// Produce buffer is indeed full, check what's the deal with the consume buffer
		if (sharedConsumePos == BUFFER_SIZE) {
			fprintf(stderr, "\t%s Consume buffer also exhausted\n", threadName);

			// Consume buffer is also exhausted, time for swap
			safeSwapBuffers(threadName);
		} else {
			fprintf(stderr, "\t%s Consume buffer still active, producers will have to wait\n", threadName);
		}
	} else {
		fprintf(stderr, "\t%s Still room for producing, signaling producers\n", threadName);

		// There's still room for producing, allow other producers to proceed
		sem_post(&sharedSemMayProduce);
	}

	// Release mutex
	sem_post(&sharedSemMutex);

	fprintf(stderr, "%s Released mutex\n", threadName);
}

/**
 * Safely consume a data item from the consume buffer.
 */
int safeConsume(const char *threadName)
{
	int data;

	fprintf(stderr, "%s Waiting on consume semaphore\n", threadName);

	// Wait until there's room for consuming
	sem_wait(&sharedSemMayConsume);

	fprintf(stderr, "%s Waiting on mutex\n", threadName);

	// Found some room, get exclusive access to shared buffer variables
	sem_wait(&sharedSemMutex);

	fprintf(stderr, "%s Acquired mutex\n", threadName);

	// Pop data from buffer
	data = sharedBuffers[sharedConsumeBufferId][sharedConsumePos];

	fprintf(stderr, "\t%s Read item value %d from buffer[%d][%d] (%d items left in buffer)\n",
		threadName, data, sharedConsumeBufferId, sharedConsumePos, BUFFER_SIZE - sharedConsumePos - 1);

	// Update consume position
	sharedConsumePos++;

	// Check if consume buffer got exhausted
	if (sharedConsumePos == BUFFER_SIZE) {
		fprintf(stderr, "\t%s Consume buffer exhausted\n", threadName);

		// Consume buffer is indeed exhausted, check what's the deal with the produce buffer
		if (sharedProducePos == BUFFER_SIZE) {
			fprintf(stderr, "\t%s Produce buffer also exhausted\n", threadName);

			// Consume buffer is also full, time for swap
			safeSwapBuffers(threadName);
		} else {
			fprintf(stderr, "\t%s Produce buffer still active, consumers will have to wait\n", threadName);
		}

	} else {
		fprintf(stderr, "\t%s Still room for consuming, signaling consumers\n", threadName);

		// There's still room for consuming, allow other consumers to proceed
		sem_post(&sharedSemMayConsume);
	}

	// Release mutex
	sem_post(&sharedSemMutex);

	fprintf(stderr, "%s Released mutex\n", threadName);

	return data;
}

/**
 * Producer thread task.
 * 
 * Infinitely produces numbers and tries to safely push them onto the produce buffer.
 * 
 */
void *producerTask(void *threadId)
{
	char threadName[255];
	int data;

	// Compile thread name
	sprintf(threadName, "[prod %ld]", (long)threadId);

	while (1) {
		// Produce number
		data = (int) ((double) rand() / RAND_MAX * MAX_ITEM_VALUE);

		// Add it in the produce buffer
		safeProduce(threadName, data);
	}

	return 0;
}

/**
 * Consumer thread task.
 * 
 * Infinitely tries to safely consume numbers from the consume buffer.
 * 
 */
void *consumerTask(void *threadId)
{
	char threadName[255];
	int data;

	// Compile thread name
	sprintf(threadName, "[cons %ld]", (long)threadId);

	while (1) {
		// Consume number
		data = safeConsume(threadName);
	}

	return 0;
}

/**
 * Initializes shared variables and semaphores. Then fires up consumer & producer threads.
 * 
 */
int main()
{
	//
	// Initialize buffers and related shared variables and semaphores
	//

	// Initial ids of consume and produce buffers
	sharedConsumeBufferId = 0;			// First is consume buffer
	sharedProduceBufferId = 1;			// Second is produce buffer

	// Current seek position in consume & produce buffer
	sharedConsumePos = BUFFER_SIZE;			// Consume buffer starts "full"
	sharedProducePos = 0;				// Produce buffer starts "empty"

	// Initialize semaphores for consumers and producers
	sem_init(&sharedSemMutex, 0, 1);		// Nobody holds the mutex at the beginning
	sem_init(&sharedSemMayProduce, 0, 1);		// Since produce buffer is empty, a producer should freely produce
	sem_init(&sharedSemMayConsume, 0, 0);		// Consumers should hold until the consume buffer becomes empty

	//
	// Create producer and consumer threads and let them start work.
	// Use 'i' as the id of the created threads
	//
	long i;
	pthread_t producerThread[PRODUCERS_COUNT];
	pthread_t consumerThread[CONSUMERS_COUNT];

	for (i = 0; i < PRODUCERS_COUNT; i++) {
		pthread_create(&producerThread[i], NULL, producerTask, ((void *)i));
	}

	for (i = 0; i < CONSUMERS_COUNT; i++) {
		pthread_create(&consumerThread[i], NULL, consumerTask, ((void *)i));
	}

	//
	// Eventually join all threads
	//
	for (i = 0; i < PRODUCERS_COUNT; i++) {
		pthread_join(producerThread[i], NULL);
	}

	for (i = 0; i < CONSUMERS_COUNT; i++) {
		pthread_join(consumerThread[i], NULL);
	}

	return 0;
}
