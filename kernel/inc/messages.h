#ifndef MESSAGES_H
#define MESSAGES_H

#define NUM_MESSAGES		500
#define MAX_MESSAGE_SIZE		64

// intertask message
typedef struct
{
	int from;			// source
	int to;				// destination
	char* msg;			// msg
} Message;

int getMessage(int from, int to, Message* msg);
int postMessage(int from, int to, char* msg);

#endif
