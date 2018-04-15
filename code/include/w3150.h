#ifndef W3150_H__
#define W3150_H__

#include <stdint.h>

#define W3150_READ 0x0F
#define W3150_WRITE 0xF0

/* Common Registers */
#define MR   		0x0000   // Mode Register
#define GAR  		0x0001   // Gateway Address: 0x0001 to 0x0004
#define SUBR 		0x0005   // Subnet mask Address: 0x0005 to 0x0008
#define SHAR  		0x0009   // Source Hardware Address (MAC): 0x0009 to 0x000E
#define SIPR 		0x000F   // Source IP Address: 0x000F to 0x0012
#define RMSR 		0x001A   // RX Memory Size Register
#define TMSR 		0x001B   // TX Memory Size Registerping
#define IR 	 		0x0015   // Interrupt
#define IMR  		0x0016   // Interrupt Mask
#define RTR0 		0x0017 
#define RTR1 		0x0018
#define RCR  		0x0019
#define PATR0 		0x001C
#define PATR1 		0x001D
#define PPPALGO 	0x001E
#define VERSIONR 	0x001F
#define PTIMER 		0x0028
#define PMAGIC 		0x0029
#define PSTATUS 	0x0035
#define IMR2        0x0036

/* Socket Modes */
#define CLOSE       0x00
#define TCP         0x01
#define UDP         0x02
#define IPRAW       0x03
#define MACRAW      0x04
#define PPPOE       0x05
#define ND          0x20
#define MULTI       0x80

/* Socket Commands */
#define SOCK_OPEN       0x01
#define SOCK_LISTEN     0x02
#define SOCK_CONNECT    0x04
#define SOCK_DISCON     0x08
#define SOCK_CLOSE      0x10
#define SOCK_SEND       0x20
#define SOCK_SEND_MAC   0x21
#define SOCK_SEND_KEEP  0x22
#define SOCK_RECV       0x40

/* Socket Status */
#define STATUS_CLOSED       0x00
#define STATUS_INIT         0x13
#define STATUS_LISTEN       0x14
#define STATUS_SYNSENT      0x15
#define STATUS_SYNRECV      0x16
#define STATUS_ESTABLISHED  0x17
#define STATUS_FIN_WAIT     0x18
#define STATUS_CLOSING      0x1A
#define STATUS_TIME_WAIT    0x1B
#define STATUS_CLOSE_WAIT   0x1C
#define STATUS_LAST_ACK     0x1D
#define STATUS_UDP          0x22
#define STATUS_IPRAW        0x32
#define STATUS_MACRAW       0x42
#define STATUS_PPPOE        0x5F

/* Socket Interrupt */
#define IR_SEND_OK  0x10
#define IR_TIMEOUT  0x08
#define IR_RECV     0x04
#define IR_DISCON   0x02
#define IR_CON      0x01

/* Socket 0 Registers */
#define S0_MR       0x0400  // Mode Register
#define S0_CR       0x0401  // Command Register
#define S0_IR       0x0402  // Interrupt Register
#define S0_SR       0x0403  // Status Register
#define S0_DIPR0    0x040C  // Dest IP 0
#define S0_DIPR1    0x040D  // Dest IP 1
#define S0_DIPR2    0x040E  // Dest IP 2
#define S0_DIPR3    0x040F  // Dest IP 3
#define S0_TX_FSR0  0x0420  // TX Free Size 0
#define S0_TX_FSR1  0x0421  // TX Free size 1
#define S0_TX_RD0   0x0422  // TX Read Pointer 0
#define S0_TX_RD1   0x0423  // TX Read Pointer 1
#define S0_TX_WR0   0x0424  // TX Write Pointer 0
#define S0_TX_WR1   0x0425  // TX Write Pointer 1
#define S0_RX_RSR0  0x0426  // RX Received Size 0
#define S0_RX_RSR1  0x0427  // RX Received Size 1
#define S0_RX_RD0   0x0428  // RX Read Pointer 0
#define S0_RX_RD1   0x0429  // RX Read Pointer 1
#define S0_RX_WR0   0x042A  // RX Write Pointer 0
#define S0_RX_WR1   0x042B  // RX Write Pointer 0

/* MISC... */
#define MACRAW_HEADER_SIZE  0x08

/* Masks and Memory Addressing for MACRAW mode
 * using socket 0.  Using maxed out memory size
 */
#define s0_rx_base  0x6000
#define s0_rx_mask  0x1FFF
#define s0_tx_base  0x4000
#define s0_tx_mask  0x1FFF

/* General configuration methods */
void w3150_init_networking(uint8_t *mac, uint8_t *ip, uint8_t *gw, uint8_t *subnet);
void w3150_ping_block();
void w3150_set_mac(const uint8_t *mac);
void w3150_set_ip(const uint8_t *ip);
void w3150_set_subnet(const uint8_t *subnet);
void w3150_set_gateway(const uint8_t *gw);
void w3150_read_gateway(uint8_t *gw);
void w3150_read_ip(uint8_t *ip);
void w3150_read_mac(uint8_t *mac);
void w3150_read_subnet(uint8_t *subnet);
uint8_t w3150_init_macraw();

/* Read and write methods */
void w3150_read(uint16_t addr, uint8_t *buf, uint16_t len);
void w3150_write(uint16_t addr, uint8_t *buf, uint16_t len);
void w3150_macraw_set_recv();
uint8_t w3150_macraw_check_recv();
uint8_t w3150_macraw_write(uint8_t *tx_buf, uint16_t len);
uint16_t w3150_macraw_read(uint8_t *recv_buf);


#endif 
