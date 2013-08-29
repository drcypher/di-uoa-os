/**
 * item_array.c
 *
 * An array of items. This is supposed to be maintained only by the writer (and producer), but
 * it's shared so that the reader threads may use the itemArrayIsValueCorrect() method for
 * assertions.
 *
 * If there was no requirement for readers to assert correctness, then the item array would be
 * a local structure inside the memory of the writer thread.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#include <stdlib.h>
#include "main.h"

//
// Global (shared) variables
//

// The actual data that need to be exchanged. This should be visible only
// to the writer thread, but it's defined global to allow *assertions* after task executions.
static int protectedItemArray[ITEM_COUNT];

/**
 * @return Size of item array.
 */
int itemArraySize()
{
	return ITEM_COUNT;
}

/**
 * Retrieve value of item at given index position.
 *
 * @param itemId Index of search item.
 * @return Value of searched item
 */
int itemArrayReadValue(int itemId)
{
	return protectedItemArray[itemId];
}

/**
 * Assert item value at position itemId equals testItemValue.
 *
 * @param itemId Position of item to test
 * @param testItemValue Test value to assert
 * @return 1 if testItemValue is correct, otherwise 0
 */
int itemArrayIsValueCorrect(int itemId, int testItemValue)
{
	return (testItemValue == protectedItemArray[itemId]);
}

/**
 * Initialize array with random items.
 *
 * @param threadName Name of threading calling the initialization.
 */
void itemArrayInit(const char *threadName)
{
	int i;

	// Fill in buffer with data
	printf("%s Initializing shared values:\n", threadName);
	for (i = 0; i < ITEM_COUNT; i++) {
		protectedItemArray[i] = (int) ((double) rand() / RAND_MAX * MAX_ITEM_VALUE);
		printf("%s %3d. %d\n", threadName, i, protectedItemArray[i]);
	}
}
