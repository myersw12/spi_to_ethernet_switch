#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>

#include <errno.h>
#include <dirent.h> 

#include <w3150.h>

/*
 * This sets up a TAP interface tunnel on the 
 * Raspberry PI.  The W3150+ library uses the WIRINGPI library for 
 * the SPI interface.  Using other SPI libraries might result in 
 * greater speeds.
 *
 * Setup Tap Interface: sudo ip tuntap add mode tap
 * Configure Tap Interface: sudo ifconfig tap0 192.168.10.123
 * Run tap_test: ./tap_example tap0 -tap
 *
 */

//#define STDOUT
//#define DEBUG_NET

const char* TunTapDev = "/dev/net/tun";

int main(int argc, char** argv) {

    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {
            fprintf(stderr, "ttio - tun/tap to stdio proxy\n");
            fprintf(stderr, "Usage: %s INTF [-tap|-tun] [CLEN] [-pi]\n", argv[0]);
            fprintf(stderr, "  INTF:  the name of the network interface\n");
            fprintf(stderr, "  -tap:  tap style ethernet tunnel (default)\n");
            fprintf(stderr, "  -tun:  point-to-point IP tunnel\n");
            fprintf(stderr, "  CLEN:  capture size (should be the same as the mtu, default: %d)\n", ETH_FRAME_LEN);
            fprintf(stderr, "  -pi:   include packet information\n");
            fprintf(stderr, "Note that the arguments should be in exactly THIS order.\n");
            exit(0);
        }
    } else {
        // Application won't run without arguments
        fprintf(stderr, "Try: %s -h\n", argv[0]);
        exit(1);
    }

    // Initialize W3150 
    uint8_t w3150_recv_buf[0xffff];
    uint16_t w3150_recv_len;

    // set this to whatever you want, helps to match with the tap interface
    uint8_t mac_address[6] = {0xca,0x1f,0xfd,0xc9,0xb2,0xe7};
    
    // IP addresses don't really matter because we are dealing
    // with raw ethernet frames
    uint8_t local_host[4]  = {192,168,50,221};
    uint8_t gateway[4]     = {192,168,50,1};
    uint8_t subnet[4]     =  {255,255,255,0};

    w3150_init_networking(mac_address,local_host,gateway,subnet);

    // Ping is disabled so the W3150 hardware stack will not respond.
    w3150_ping_block();

    if (w3150_init_macraw() != 1){
        printf("macraw init failed\n");
        exit(1);
    }

    struct ifreq ifr;
    // Clear the ifreq structure
    memset(&ifr, 0, sizeof(ifr));

    char dev[IFNAMSIZ] = "tap0";	//Interface name
    // Copy the interface name from the commandline
    strcpy(dev, argv[1]);
    strcpy(ifr.ifr_name, dev);

    // Initial capture length
    int CaptureLen = ETH_FRAME_LEN;

    {
        // Default to TAP interface, change only if --tun option is detected
        short ifrflags = IFF_TAP;
        if (argc > 2 && strcmp(argv[2], "-tun") == 0) {
                ifrflags = IFF_TUN;
                CaptureLen = ETH_DATA_LEN;
        }
        
        // Different capture length?
        if (argc > 3) CaptureLen = atoi(argv[3]);
        
        if (CaptureLen < 1) {
            fprintf(stderr, "Capture length %d invalid!\n", CaptureLen);
            exit(1);
        }
        
        fprintf(stderr, "Max packet length: %d\n", CaptureLen);
        
        // Include packet info in output?
        if (!(argc > 4 && strcmp(argv[4], "-pi") == 0)) ifrflags = ifrflags | IFF_NO_PI;
            // Set ifr flags
            ifr.ifr_flags = ifrflags;
    }

    int TunFD; // Tun/tap stream

    // Try to open the tun/tap device file, exit on failure
    // Use the nonblocking flag
    fprintf(stderr, "Opening %s\n", TunTapDev);
    
    if ((TunFD = open(TunTapDev, O_RDWR | O_NONBLOCK)) < 0) {
        fprintf(stderr, "Failed to open %s: %d\n", TunTapDev, TunFD);
        exit(2);
    }

    // Request the interface and set its flags
    fprintf(stderr, "Requesting device: %s\n", dev);
    
    if (ioctl(TunFD, TUNSETIFF, (void *)&ifr) < 0 ) {
        fprintf(stderr, "ioctl for device name failed!\n");
        close(TunFD);
        exit(3);
    }

    strcpy(dev, ifr.ifr_name);
    fprintf(stderr, "Tunnel interface: %s\n", dev);
    fprintf(stderr, "Proxy ready for action!\n");

    unsigned char RBuf[CaptureLen]; //Data buffers
    int RBufLen = 0; //Packet length
    
    //Start an infinite loop
    while (1) {
        // Read from tap device file
        // This is data coming from the PI going to the outside
        RBufLen = read(TunFD, RBuf, CaptureLen);
        
        if (RBufLen > 0){
            #ifdef DEBUG_NET
            printf("Read %d bytes from tap\n", RBufLen);
            #endif
            if (RBufLen == 0) {
                fprintf(stderr, "End of file on %s\n", TunTapDev);
                exit(0);
            } else if (RBufLen < 0) {
                fprintf(stderr, "Some error occured while reading from %s: %d\n", TunTapDev, RBufLen);
                exit(4);
            }
            w3150_macraw_write(RBuf, RBufLen);
        }
        
        // Read from the W3150
        if (w3150_macraw_check_recv() == 1){
            w3150_recv_len = w3150_macraw_read(w3150_recv_buf);
            #ifdef DEBUG_NET
            printf("Read %d bytes from W3150\n", w3150_recv_len);
            #endif
            // Write to tun/tap device file
            write(TunFD, w3150_recv_buf, w3150_recv_len);
        }
    }
}
