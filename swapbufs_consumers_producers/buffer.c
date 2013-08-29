/**
 * buffer.c
 * 
 * Non thread-safe implementation of the swappable double-buffer data structure.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */
#include "main.h"

//
// Global (shared) variables
//

// Id (0 or 1) of buffer currently used for consuming/producing respectively
static int sharedConsumeBufferId;
static int sharedProduceBufferId;

// Current seek position for next consume/produce action in consume/produce buffers respectively
static int sharedBufferPos[2];

// The two swappable buffers keeping the integer item values
static int sharedBuffers[2][BUFFER_SIZE];

/**
 * Swaps the ids of consume/produce buffer.
 *
 * Also updates the respective seek positions.
 *
 * @param threadName Name of thread executing the buffer swap
 */
void bufferSwap(const char *threadName)
{
	// Swap buffers
	int swapBufferId = sharedConsumeBufferId;
	sharedConsumeBufferId = sharedProduceBufferId;
	sharedProduceBufferId = swapBufferId;

	fprintf(stderr, "\t%s Swapping buffers: consume -> %d, produce -> %d\n",
		threadName, sharedConsumeBufferId, sharedProduceBufferId);

	// Fix positions
	sharedBufferPos[sharedConsumeBufferId] = 0;
	sharedBufferPos[sharedProduceBufferId] = 0;
}


/**
 * Produce a new data item into the produce buffer.
 *
 * @param threadName Name of thread producing data
 * @param data Data produced
 */
void bufferProduceData(const char *threadName, int data)
{
	int seekPos = sharedBufferPos[sharedProduceBufferId];

	// Push data to buffer
	sharedBuffers[sharedProduceBufferId][seekPos] = data;

	fprintf(stderr, "\t%s Wrote new item value %d to buffer[%d][%d] (%d items left in buffer)\n",
		threadName, data, sharedProduceBufferId, seekPos, BUFFER_SIZE - seekPos - 1);

	// Update produce position
	sharedBufferPos[sharedProduceBufferId]++;
}

/**
 * Consume a data item from the consume buffer.
 *
 * @param threadName Name of thread consuming data.
 * @return Consumed data
 */
int bufferConsumeData(const char *threadName)
{
	int data;
	int seekPos = sharedBufferPos[sharedConsumeBufferId];

	data = sharedBuffers[sharedConsumeBufferId][seekPos];

	fprintf(stderr, "\t%s Read item value %d from buffer[%d][%d] (%d items left in buffer)\n",
		threadName, data, sharedConsumeBufferId, seekPos, BUFFER_SIZE - seekPos - 1);

	// Update consume position
	sharedBufferPos[sharedConsumeBufferId]++;

	return data;
}

/**
 * Is consume buffer full?
 *
 * @return 1 if produce buffer is full, otherwise 0
 */
int bufferConsumeIsExhausted()
{
	return (sharedBufferPos[sharedConsumeBufferId] == BUFFER_SIZE);
}

/**
 * Is produce buffer full?
 *
 * @return 1 if produce buffer is full, otherwise 0
 */
int bufferProduceIsExhausted()
{
	return (sharedBufferPos[sharedProduceBufferId] == BUFFER_SIZE);
}

/**
 * Initialize data structure.
 */
void bufferInit()
{
	// Initial ids of consume and produce buffers
	sharedConsumeBufferId = 0;				// First is consume buffer
	sharedProduceBufferId = 1;				// Second is produce buffer

	// Current seek position in consume & produce buffer
	sharedBufferPos[sharedConsumeBufferId] = BUFFER_SIZE;		// Consume buffer starts "full"
	sharedBufferPos[sharedProduceBufferId] = 0;					// Produce buffer starts "empty"
}
