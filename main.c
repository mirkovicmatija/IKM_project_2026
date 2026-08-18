#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

struct Array {
    unsigned short size;
    unsigned short capacity;
    struct CAN_DBC_Message *messages;
} Array;


struct CAN_DBC_Message {
    unsigned int id;
    unsigned int length;
    unsigned char frame[8];
    char name[64];
} CAN_DBC_Message;



void appendDBC_array(struct Array *array, struct CAN_DBC_Message new_message) {
    if (array->size >= array->capacity) {
        // Resize the array
        array->capacity *= 2;
        array->messages = (struct CAN_DBC_Message*)realloc(array->messages, array->capacity * sizeof(struct CAN_DBC_Message));
    }
    array->messages[array->size] = new_message;
    array->size++;
}

void freeDBC_array(struct Array *array) {
    free(array->messages);
    array->messages = NULL;
    array->size = 0;
    array->capacity = 0;
}


unsigned int getCANdataFromPhysical(int physicalValue, double factor, double offset) {
    return (unsigned int)((physicalValue - offset)/ factor);
}

void insertSignalIntoMessage(unsigned char *message, unsigned int start_bit, unsigned int signal_length, unsigned int signal_value, const unsigned short endian) {
    // Calculate the byte and bit position
    unsigned int byte_index = start_bit / 8;
    unsigned int bit_index = start_bit % 8;
    
    for (unsigned int i = 0; i < signal_length; i++) {
            unsigned int bit_value = (signal_value >> i) & 0x01;
            message[byte_index + ((endian)? 0 : ((signal_length / 8) - 1))] |= (bit_value << bit_index);

            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                byte_index = byte_index + ((endian)? 1 : - 1);
            }        
}
    //return message;
}

void dbcParser(const char* filename, struct Array *array, const int value) {

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }
    
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "BO_ ", 4) == 0) {
            // Process Message Header Line and extract ID, name of process, and Length
            struct CAN_DBC_Message message = {0,0,{0},""};


            sscanf(line, "BO_ %u %s %u ", &message.id, message.name, &message.length);
          
            appendDBC_array(array, message);
            
        } else if ((strncmp(line, " SG_ ", 5) == 0) || (strncmp(line, "  SG_ ", 6) == 0) || (strncmp(line, "   SG_ ", 7) == 0)) {
            // Process Signal Specific Line
            char is_signed, sg_line[64];
            unsigned int start_bit;
            unsigned int signal_length;
            unsigned int min, max;
            unsigned short is_endiad;
            double factor, offset;

            sscanf(line, "   SG_ %s : %u|%u@%hu%c (%lf,%lf) [%u|%u]", sg_line, &start_bit, &signal_length ,&is_endiad, &is_signed, &factor, &offset, &min, &max);
            
            
            unsigned int canValue = getCANdataFromPhysical(value, factor, offset);
    
            insertSignalIntoMessage(array->messages[array->size-1].frame, start_bit, signal_length, canValue, is_endiad);
          
        }

    }
    fclose(file);
}

void printDBC_array(struct Array *array) {
    for (unsigned short i = 0; i < array->size; i++) {
        printf("Message ID: %u, Name: %s, Length: %u\n", array->messages[i].id, array->messages[i].name, array->messages[i].length);
        for (unsigned int j = 0; j < array->messages[i].length; j++) {
            printf("Frame[%u]: %02X\n", j, array->messages[i].frame[j]);
        }
    }
}


int main(int argc, char *argv[])
{
    // Your code here
    struct CAN_DBC_Message *message = calloc(10, sizeof(struct CAN_DBC_Message));
    struct Array dbcArray = {0, 10, message};

    if (argc > 1) {
        dbcParser(argv[1], &dbcArray, atoi(argv[2]));
    }
    printDBC_array(&dbcArray);

    struct ifreq ifr;			/* CAN interface info struct */
	struct sockaddr_can addr;
/* CAN adddress info struct */
	struct can_frame frame;		/* CAN frame struct */
	int s;						/* SocketCAN handle */

	memset(&ifr, 0, sizeof(ifr));
	memset(&addr, 0, sizeof(addr));
	memset(&frame, 0, sizeof(frame));
	
	// TODO: Open a socket here
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	
	/* Convert interface string "can0" to index */
	strcpy(ifr.ifr_name, "can0");
	ioctl(s, SIOCGIFINDEX, &ifr);
	
	/* Setup address for binding */
	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;
/* Disable reception filter on this RAW socket */
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
	// TODO: Bind socket to can0 interface
	bind(s, (struct sockaddr *)&addr, sizeof(addr));

    for (unsigned short i = 0; i < dbcArray.size; i++) {
        frame.can_id = dbcArray.messages[i].id;
        frame.can_dlc = dbcArray.messages[i].length;
        
        for(unsigned short j = 0; j < dbcArray.messages[i].length; j++){
            frame.data[j] = dbcArray.messages[i].frame[j];
        }

        write(s, &frame, sizeof(frame));
    }

	
	close(s);

    freeDBC_array(&dbcArray);
    return 0;
}