#include <stdint.h>
#include <w3150.h>
#include <stdio.h>
#include <wiringPiSPI.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>


/* This library is designed for use with the 
 * macraw functionality of the W3150+.  Therefore, many
 * of the functions are set up to work with socket 0.
 */

//#define DEBUG_RECV
//#define DEBUG_TX
//#define DEBUG
//#define DEBUG_TRANSFER

static const int CHANNEL = 1;

int initializeSPI(int rate){
    
    int fd = 0;

    printf("Initializing SPI\n");
    fd = wiringPiSPISetupMode(CHANNEL, rate, 0);

    if (fd == -1){
        printf("Error Setting up SPI");
        exit(0);
    }
    else
        return fd;
}

uint8_t spi_transfer(uint8_t* bytes, int len){

    int result = 0;
    
    #ifdef DEBUG_TRANSFER
    int i = 0;
    #endif

    #ifdef DEBUG_TRANSFER
    for (i = 0; i < len; i++)
        printf("%X ", bytes[i]);
    printf("\n");
    #endif

    result = wiringPiSPIDataRW(CHANNEL, (unsigned char*)bytes, len);
    
    #ifdef DEBUG_TRANSFER
    for (i = 0; i < len; i++)
        printf("%X ", bytes[i]);
    printf("\n");
    printf("+++++++++++++++++++++++++\n");
    #endif

    return bytes[len-1];

}

/* Private register IO functions */
static uint8_t w3150_read_register(uint16_t addr){
	
    uint8_t data = 0;
    int len = 4;
    uint8_t buffer[len];

    buffer[0] = W3150_READ;
    buffer[1] = (uint8_t)(addr >> 8);
    buffer[2] = (uint8_t)(addr & 0xff);
    buffer[3] = 0x0;

    data = spi_transfer(buffer, len);

    return data;
}

static void w3150_write_register(uint16_t addr, uint8_t data) {
    
    int len = 4;
    uint8_t buffer[len];

    buffer[0] = W3150_WRITE;
    buffer[1] = (uint8_t)(addr >> 8);
    buffer[2] = (uint8_t)(addr & 0xff);
    buffer[3] = data;
    
    data = spi_transfer(buffer, len);
    
}

void w3150_set_mac(const uint8_t *mac){

    int i;
    for(i = 0; i < 6; i++)
        w3150_write_register((SHAR+i), mac[i]);

}

void w3150_set_ip(const uint8_t *ip){

    int i;
    for(i = 0; i < 4; i++)
        w3150_write_register((SIPR+i), ip[i]);

}

void w3150_set_subnet(const uint8_t *subnet){

    int i;
    for(i = 0; i < 4; i++)
        w3150_write_register((SUBR+i), subnet[i]);

}

void w3150_set_gateway(const uint8_t *gw){

    int i;
    for(i = 0; i < 4; i++)
        w3150_write_register((GAR+i), gw[i]);

}

void w3150_read_mac(uint8_t *mac){

    int i;
    for (i = 0; i < 6; i++)
        mac[i] = w3150_read_register(SHAR+i);

}

void w3150_read_ip(uint8_t *ip){

    int i;
    for (i = 0; i < 4; i++)
        ip[i] = w3150_read_register(SIPR+i);

}

void w3150_read_subnet(uint8_t *subnet){

    int i;
    for (i = 0; i < 4; i++)
        subnet[i] = w3150_read_register(SUBR+i);

}

void w3150_read_gateway(uint8_t *gw){

    int i;
    for (i = 0; i < 4; i++)
        gw[i] = w3150_read_register(GAR+i);

}

/* Public Read and Write Methods */

void w3150_read(uint16_t addr, uint8_t *buf, uint16_t len){

    int i;
    for (i = 0; i < len; i++)
        buf[i] = w3150_read_register(addr + i);
}


void w3150_write(uint16_t addr, uint8_t *buf, uint16_t len){

    int i;
    for (i = 0; i < len; i++){
        w3150_write_register(addr+i, buf[i]);
        #ifdef DEBUG_TRANSFER
        printf("%X ", buf[i]);
        #endif
    }
    #ifdef DEBUG_TRANSFER
    printf("\n");
    #endif
}

/* General configuration methods */

void w3150_init_networking(uint8_t *mac, uint8_t *ip, uint8_t *gw, uint8_t *subnet) {

    // Use 8000000 for the baud rate
    // seems to be the highest we can get
    // and still be reliable
    initializeSPI(8000000);

    // Software reset
    w3150_write_register(MR,0x80);

    // Wait for reset
    usleep(5000);

    // Set mac address
    w3150_set_mac(mac);

    // Set source IP address
    w3150_set_ip(ip);

    // Set gateway
    w3150_set_gateway(gw);

    // Set subnet mask
    w3150_set_subnet(subnet);

    // Debug Logging 
    #ifdef DEBUG

    uint8_t r_mac[6];
    uint8_t r_ip[4];
    uint8_t r_subnet[4];
    uint8_t r_gw[4];

    w3150_read_mac(r_mac);
    w3150_read_ip(r_ip);
    w3150_read_subnet(r_subnet);
    w3150_read_gateway(r_gw);

    printf("----------------w3150 Init----------------\n");
    printf("IP Address  : %d.%d.%d.%d\n",r_ip[0], r_ip[1], r_ip[2], r_ip[3]);
    printf("Subnet Mask : %d.%d.%d.%d\n",r_subnet[0], r_subnet[1], r_subnet[2], r_subnet[3]);
    printf("Gateway     : %d.%d.%d.%d\n",r_gw[0], r_gw[1], r_gw[2], r_gw[3]);
    printf("Mac Address : %x:%x:%x:%x:%x:%x\n",r_mac[0], r_mac[1], r_mac[2], r_mac[3], r_mac[4], r_mac[5]);
    printf("------------------------------------------\n");
    #endif

}

void w3150_ping_block(){
    w3150_write_register(MR, 0x10);
}

void w3150_macraw_open_socket(){
    w3150_write_register(S0_CR, SOCK_OPEN);
}

void w3150_macraw_close_socket(){
    w3150_write_register(S0_CR, SOCK_CLOSE);
}

/* Initialize macraw mode on socket 0
 * When using macraw mode, socket 0 is the only
 * socket that can be used.  Therefore the memory
 * assigned to it is maxed out.
 *
 * Return 1 if successful */
uint8_t w3150_init_macraw() {

    // Set RX Memory Size Register
    // Assigning 8KB to Socket 0
    w3150_write_register(RMSR, 0xFF);

    // Set the TX Memory Size Register
    // Assigning 8KB to Socket 0
    w3150_write_register(TMSR, 0xFF);
   
    // Set mode to macraw and open socket
    // Only Socket 0 supports macraw
    w3150_write_register(S0_MR, MACRAW);
    w3150_macraw_open_socket();

    // check if it actually went to macraw mode
    if (w3150_read_register(S0_SR) != STATUS_MACRAW){
        w3150_macraw_close_socket();
        return 0;
    }
    else
        return 1;
}


uint16_t w3150_macraw_get_received_size_register(){

    uint16_t size = 0;

    size = (w3150_read_register(S0_RX_RSR0) << 8) | w3150_read_register(S0_RX_RSR1);

    return size;
}

uint16_t w3150_macraw_get_tx_free_size(){

    uint16_t size = 0;

    size = (w3150_read_register(S0_TX_FSR0) << 8) | w3150_read_register(S0_TX_FSR1);

    return size;
}

uint16_t w3150_macraw_get_tx_write_pointer(){

    uint16_t pointer = 0;

    pointer = (w3150_read_register(S0_TX_WR0) << 8) | w3150_read_register(S0_TX_WR1);

    return pointer;
}

uint16_t w3150_macraw_get_tx_read_pointer(){

    uint16_t pointer = 0;

    pointer = (w3150_read_register(S0_TX_RD0) << 8) | w3150_read_register(S0_TX_RD1);

    return pointer;
}


uint16_t w3150_macraw_get_read_pointer(){
    
    uint16_t pointer = 0x0000;

    pointer = (w3150_read_register(S0_RX_RD0) << 8) | w3150_read_register(S0_RX_RD1);

    return pointer;
}

uint16_t w3150_macraw_get_rx_write_pointer(){

    uint16_t pointer = 0x0000;

    pointer = (w3150_read_register(S0_RX_WR0) << 8) | w3150_read_register(S0_RX_WR1);

    return pointer;
}

void w3150_macraw_set_recv(){
    // Set RECV command
    w3150_write_register(S0_CR, SOCK_RECV);
}

/* check to see if data has sent
 * return 1 if it has */
uint8_t w3150_macraw_check_send(){

    if (w3150_read_register(S0_CR) != 0x00) {
        return 0;
    }
    else
        return 1;
}

/* Check for data in the receive buffer
 * return 1 if there is data */
uint8_t w3150_macraw_check_recv(){
    
    #ifdef DEBUG_RECV_CHECK
    printf("Received Size: %X\n", w3150_macraw_get_received_size_register());
    #endif
    if (w3150_macraw_get_received_size_register() != 0x0000){
        return 1;
    }

    else {
        return 0;
    }
}

void w3150_macraw_write_read_pointer(uint16_t ptr){

    w3150_write_register(S0_RX_RD0, (uint8_t) ((ptr & 0xFF00) >> 8));
    w3150_write_register(S0_RX_RD1, (uint8_t) (ptr & 0x00FF));

}

void w3150_macraw_set_write_pointer(uint16_t ptr){

    w3150_write_register(S0_TX_WR0, (uint8_t) ((ptr & 0xFF00) >> 8));
    w3150_write_register(S0_TX_WR1, (uint8_t) (ptr & 0x00FF));

}

/* Read from the RX buffer
 * returns the number of bytes read */
uint16_t w3150_macraw_read(uint8_t *recv_buf) {


    // get the receive size
    uint16_t size = w3150_macraw_get_received_size_register();

    uint16_t read_pointer = w3150_macraw_get_read_pointer();

    #ifdef DEBUG_RECV
    printf("received size register: %X\n", size);
    printf("read pointer: %X\n", read_pointer);
    #endif
   
    // calculate offset address
    uint16_t offset = read_pointer & s0_rx_mask;

    #ifdef DEBUG_RECV
    printf("offset: %X\n", offset);
    #endif

    // calculate start address
    uint16_t start_addr = s0_rx_base + offset;

    #ifdef DEBUG_RECV
    printf("start addr (header): %X\n", start_addr);
    #endif

    uint16_t upper_size = 0;
    uint16_t left_size = 0;

    uint16_t new_read_pointer;

    // length of the recieved packet + 2 byte header
    uint16_t macraw_header;

    // check for rx memory overflow while reading 2 byte header
    if ((offset + 2) > (s0_rx_mask + 1)){
        #ifdef DEBUG_RECV
        printf("RX MEMORY OVERFLOW: Header\n");
        #endif

        macraw_header = w3150_read_register(start_addr) << 8;
        macraw_header = macraw_header | w3150_read_register(s0_rx_base);
        offset = 1;
    }
    else {
        macraw_header = w3150_read_register(start_addr) << 8;
        macraw_header = macraw_header | w3150_read_register(start_addr + 1);
        offset += 2;
    }

    #ifdef DEBUG_RECV
    printf("macraw_header: %X\n", macraw_header);
    #endif

    start_addr = s0_rx_base + offset;

    #ifdef DEBUG_RECV
    printf("start addr (body): %X\n", start_addr);
    #endif

    // Check for RX memory overflow
    if (macraw_header + offset > s0_rx_mask + 1){
        #ifdef DEBUG_RECV
        printf("RX MEMORY OVERFLOW: Data\n");
        #endif 

        upper_size = s0_rx_mask + 1 - offset;

        w3150_read(start_addr, recv_buf, upper_size);

        left_size = macraw_header - upper_size;

        #ifdef DEBUG_RECV
        printf("Upper Size: %X\nLeft Size: %X\n", upper_size, left_size);
        #endif 

        w3150_read(s0_rx_base, recv_buf + upper_size, left_size);

    }
    else {
        w3150_read(start_addr, recv_buf, macraw_header);
    }
 
    // turns out this is really important.  If this is not done correctly
    // all sorts of bad things happen.  The read pointer value is used with
    // some internal calculations.
    new_read_pointer = size + read_pointer;

    #ifdef DEBUG_RECV
    printf("new read pointer: %X\n", new_read_pointer);
    printf("new write pointer: %X\n", w3150_macraw_get_rx_write_pointer()); 
    #endif

    // Increase S0_RX_RD by size of packet, don't mess this up.
    w3150_macraw_write_read_pointer(new_read_pointer);

    // Set RECV command
    w3150_macraw_set_recv();
    
    return size;
}

/* Write raw data
 * return 1 if successful */
uint8_t w3150_macraw_write(uint8_t *tx_buf, uint16_t len) {

    uint16_t offset;
    uint16_t start_address;
    uint16_t upper_size;
    uint16_t left_size;
    uint16_t tx_write_pointer;
    uint16_t new_tx_pointer;

    #ifdef DEBUG_TX
    printf("S0_SR: %X\n", w3150_read_register(S0_SR));
    printf("tx free size: %X\n" , w3150_macraw_get_tx_free_size());
    printf("tx read pointer: %x\n", w3150_macraw_get_tx_read_pointer());
    #endif
    while (w3150_macraw_get_tx_free_size() < len)
        usleep(1);

    tx_write_pointer = w3150_macraw_get_tx_write_pointer();
    #ifdef DEBUG_TX
    printf("tx write pointer: %X\n", tx_write_pointer);
    #endif
    offset = tx_write_pointer & s0_tx_mask;

    start_address = s0_tx_base + offset;

    #ifdef DEBUG_TX
    printf("Start Address: %X\n", start_address);
    #endif

    if ((offset + len) > (s0_rx_mask + 1)){
        
        upper_size = s0_tx_mask + 1 - offset;

        w3150_write(start_address, tx_buf, upper_size);

        left_size = len - upper_size;

        w3150_write(s0_tx_base, tx_buf + upper_size, left_size);
    }

    else
        w3150_write(start_address, tx_buf, len);

    // this is really important for all of the same reasons as the
    // read pointer
    new_tx_pointer = tx_write_pointer + len;

    w3150_macraw_set_write_pointer(new_tx_pointer);

    w3150_write_register(S0_CR, SOCK_SEND);

    #ifdef DEBUG_TX
    printf("tx read pointer: %X\n", w3150_macraw_get_tx_read_pointer());
    printf("tx write pointer: %X\n", w3150_macraw_get_tx_write_pointer());
    printf("S0_SR: %X\n", w3150_read_register(S0_SR));
    #endif

    // check to make sure data is finished sending
    while(!w3150_macraw_check_send())
        usleep(1);

    return 1;
}





