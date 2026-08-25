#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "dbc.h"

int main(int argc, char *argv[])
{
    // Allocate memory for the array of messages
    struct CAN_DBC_Message *message = calloc(10, sizeof(struct CAN_DBC_Message));
    struct Array dbcArray = {0, 10, message};

    if (argc > 1) {
        dbcParser(argv[1], &dbcArray);
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