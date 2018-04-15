#include <stdio.h>
#include <errno.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <w3150.h>
#include <stdlib.h>

int main(void) {
    
    // Simple example of receiving raw ethernet frames from the W3150
    // Raw frames are printed as they arrive

    int i;
    uint8_t recv_buf[0xffff];
    uint16_t recv_len;

    uint8_t mac_address[6] = {0xde,0xad,0xbe,0xef,0xba,0x5e};
    uint8_t local_host[4]  = {192,168,10,123};
    uint8_t gateway[4]     = {192,168,50,1};
    uint8_t subnet[4]     =  {255,255,255,0};

    printf("--------Receive Example--------\n");
    w3150_init_networking(mac_address,local_host,gateway,subnet);

    w3150_ping_block();

    if (w3150_init_macraw() != 1){
        printf("macraw init failed\n");
        exit(0);
    }

    while(1){
        
        // wait for a response
        while(w3150_macraw_check_recv() != 1){
            usleep(1);
        }

        printf("Got a packet\n");
        recv_len = w3150_macraw_read(recv_buf);

        printf("Printing Packet\n");
        for (i = 0; i < recv_len; i++)
            printf("%x ", recv_buf[i]);

        printf("\n");

    }

    return 0;

}
