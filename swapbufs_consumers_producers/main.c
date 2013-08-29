/**
 * main.c
 *
 * Simple workflow skeleton managing:
 * 1. Thread management.
 * 2. Data structure initialization.
 * 3. Protocol initialization.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "main.h"

/**
 * Producer thread task.
 *
 * Infinitely produces numbers and tries to safely push them onto the produce buffer.
 *
 * @param threadId Id assigned to thread. Used to create a unique name for it
 * @return
 */
void *producerThreadTask(void *threadId)
{
	int data;
	char threadName[255];

	// Compile thread name
	sprintf(threadName, "[prod %3ld]", (long)threadId);

	while (1) {
		// Produce number
		data = (int) ((double) rand() / RAND_MAX * MAX_ITEM_VALUE);

		// Add it in the produce buffer
		protocolProduceData(threadName, data);
	}

	return 0;
}

/**
 * Consumer thread task.
 * 
 * Infinitely tries to safely consume numbers from the consume buffer.
 * 
 * @param threadId Id assigned to thread. Used to create a unique name for it
 * @return
 */
void *consumerThreadTask(void *threadId)
{
	int data;
	char threadName[255];

	// Compile thread name
	sprintf(threadName, "[cons %3ld]", (long)threadId);

	while (1) {
		// Consume number
		data = protocolConsumeData(threadName);
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
	bufferInit();
	protocolInit();

	//
	// Create producer and consumer threads and let them start work.
	// Use 'i' as the id of the created threads
	//
	long i;
	pthread_t producerThread[PRODUCERS_COUNT];
	pthread_t consumerThread[CONSUMERS_COUNT];

	for (i = 0; i < PRODUCERS_COUNT; i++) {
		pthread_create(&producerThread[i], NULL, producerThreadTask, ((void *)i));
	}

	for (i = 0; i < CONSUMERS_COUNT; i++) {
		pthread_create(&consumerThread[i], NULL, consumerThreadTask, ((void *)i));
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
