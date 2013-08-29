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
#define PRODUCERS_COUNT 3

// Number of consumer threads to create
#define CONSUMERS_COUNT 3

// Size of each buffer
#define BUFFER_SIZE 10

// Max value of produced integer items
#define MAX_ITEM_VALUE 300

//
// Buffer functions
//
void bufferSwap(const char *threadName);

void bufferProduceData(const char *threadName, int data);

int bufferConsumeData(const char *threadName);

int bufferConsumeIsExhausted();

int bufferProduceIsExhausted();

void bufferInit();

//
// Protocol functions
//

// Read value function
int protocolConsumeData(const char *threadName);

// Produce data function
void protocolProduceData(const char *threadName, int data);

// One-time initialization function
void protocolInit();

#endif  /* MAIN_H */
