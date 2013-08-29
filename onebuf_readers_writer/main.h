/**
 * main.h
 * 
 * Common declarations and definitions.
 *
 * @author Konstantinos Filios <konfilios@gmail.com>
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

//
// Configurable constants. Feel free to try different combinations. Avoid 0 values :)
//

// Number of producer threads to create
#define READERS_COUNT 10

// Size of buffer
#define ITEM_COUNT 20

// Max value of produced integer items
#define MAX_ITEM_VALUE 300

//
// Exchange buffer functions
//
int exchangeBufferReadValue(const char *threadName, int itemId);

void exchangeBufferWriteValue(const char *threadName, int itemId, int itemValue);

void exchangeBufferInit();

//
// Item array functions
//
int itemArraySize();

int itemArrayReadValue(int itemId);

int itemArrayIsValueCorrect(int itemId, int testItemValue);

void itemArrayInit(const char *threadName);


//
// Protocol functions
//

// Read value function
int protocolReadValue(const char *threadName, int itemId);

// Write value function
void protocolWriteValue(const char *threadName, int itemId, int itemValue);

// One-time initialization function
void protocolInit();

#endif  /* MAIN_H */
