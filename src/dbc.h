#ifndef DBC_H
#define DBC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern struct Array {
    unsigned short size;
    unsigned short capacity;
    struct CAN_DBC_Message *messages;
} Array;

//struct to hold the message data
extern struct CAN_DBC_Message {
    unsigned int id;
    unsigned int length;
    unsigned char frame[8];
    char name[64];
} CAN_DBC_Message;

void appendDBC_array(struct Array *array, struct CAN_DBC_Message new_message);
void freeDBC_array(struct Array *array);
int getCANdataFromPhysical(int physicalValue, double factor, double offset);
void insertSignalIntoMessage(unsigned char *message, unsigned int start_bit, unsigned int signal_length, int signal_value, const unsigned short endian);
void dbcParser(const char* filename, struct Array *array);
void printDBC_array(struct Array *array);


#endif