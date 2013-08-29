/**
 * exchange_buffer.c
 *
 * Implementation of a single-place exchange buffer. It's able to exchange a single value
 * among different threads. Its methods must be wrapped into some protocol function set in order
 * to make it suitable for concurrent applications.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */
#include "main.h"

//
// Global (shared) variables
//

// Shared variable allowing the exchange of a single item
static int sharedExchangeBufferValue;

/**
 * Read the value of sharedExchangeBufferValue variable.
 *
 * @param threadName Name of reader thread reading a value
 * @param itemId The position of the item in the initial buffer
 * @return Read value
 */
int exchangeBufferReadValue(const char *threadName, int itemId)
{
	fprintf(stderr, "%s Reading item with id=%d from the shared exchange buffer\n", threadName, itemId);

	// Just read whatever is in the shared buffer
	return sharedExchangeBufferValue;
}

/**
 * Write a value to the sharedExchangeBufferValue variable so it's read by readers.
 *
 * @param threadName Name of writer thread writing out the item value
 * @param itemId The position of the item in the initial buffer
 * @param itemValue The value of the item being exchanged with the readers
 */
void exchangeBufferWriteValue(const char *threadName, int itemId, int itemValue)
{
	fprintf(stderr, "%s Writing item with id=%d and value=%d to the shared exchange buffer\n", threadName, itemId, itemValue);
	sharedExchangeBufferValue = itemValue;
}

/**
 * Initialize shared buffer to some invalid value.
 */
void exchangeBufferInit()
{
	sharedExchangeBufferValue = -1;
}
