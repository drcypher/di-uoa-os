/**
 * three_sem.c
 *
 * Thread-safe wrapper of buffer.c using three semaphores:
 * 1. mutex: Protects critical section (buffer manipulation)
 * 2. mayProduce: Signals producers to proceed
 * 3. mayConsume: Signals consumers to proceed
 * 
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#include <semaphore.h>
#include "main.h"

//
// Global (shared) variables
//

// Mutex regulating exclusive access to buffer manipulation sections (critical sections)
static sem_t sharedSemMutex;

// Signaling semaphore telling consumers if they can proceed with consuming items
static sem_t sharedSemMayProduce;

// Signaling semaphore telling producers if they can proceed with producing items
static sem_t sharedSemMayConsume;

/**
 * Safely produce a new data item into the produce buffer.
 *
 * @param threadName Name of thread producing data
 * @param data Data produced
 */
void protocolProduceData(const char *threadName, int data)
{
	fprintf(stderr, "%s Waiting on produce semaphore\n", threadName);

	// Wait until there's room for producing
	sem_wait(&sharedSemMayProduce);

	fprintf(stderr, "%s Waiting on mutex\n", threadName);

	// Found some room, get exclusive access to shared buffer variables
	sem_wait(&sharedSemMutex);

	fprintf(stderr, "%s Acquired mutex\n", threadName);

	// Push data to buffer
	bufferProduceData(threadName, data);

	// Check if produce buffer got full
	if (bufferProduceIsExhausted()) {
		fprintf(stderr, "\t%s Produce buffer exhausted\n", threadName);

		// Produce buffer is indeed full, check what's the deal with the consume buffer
		if (bufferConsumeIsExhausted()) {
			fprintf(stderr, "\t%s Consume buffer also exhausted\n", threadName);

			// Consume buffer is also exhausted, time for swap
			bufferSwap(threadName);

			fprintf(stderr, "\t%s Signaling consumers and producers\n", threadName);

			// Signal both consumers and producers
			sem_post(&sharedSemMayConsume);
			sem_post(&sharedSemMayProduce);
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
 *
 * @param threadName Name of thread consuming data.
 * @return
 */
int protocolConsumeData(const char *threadName)
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
	data = bufferConsumeData(threadName);

	// Check if consume buffer got exhausted
	if (bufferConsumeIsExhausted()) {
		fprintf(stderr, "\t%s Consume buffer exhausted\n", threadName);

		// Consume buffer is indeed exhausted, check what's the deal with the produce buffer
		if (bufferProduceIsExhausted()) {
			fprintf(stderr, "\t%s Produce buffer also exhausted\n", threadName);

			// Consume buffer is also full, time for swap
			bufferSwap(threadName);

			fprintf(stderr, "\t%s Signaling consumers and producers\n", threadName);

			// Signal both consumers and producers
			sem_post(&sharedSemMayConsume);
			sem_post(&sharedSemMayProduce);
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
 * Initializes shared variables and semaphores.
 */
void protocolInit()
{
	// Initialize semaphores for consumers and producers
	sem_init(&sharedSemMutex, 0, 1);		// Nobody holds the mutex at the beginning
	sem_init(&sharedSemMayProduce, 0, 1);	// Since produce buffer is empty, a producer should freely produce
	sem_init(&sharedSemMayConsume, 0, 0);	// Consumers should hold until the consume buffer becomes empty
}
