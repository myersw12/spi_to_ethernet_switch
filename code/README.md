# Example Usage

The tx_example and recv_example require no setup.  To see them working, connect the board to another networked device.  For the 
tx_example, you will see an ICMP response packet addressed to broadcast.  

## Tap_example setup
The tap_example requires a small amount of setup:
  * Setup Tap Interface: sudo ip tuntap add mode tap
  * Configure Tap Interface: sudo ifconfig tap0 192.168.10.123
  * Ensure that there are no other interfaces active on the same subnet.
  * Run tap_test: ./tap_example tap0 -tap
 
At this point you can send traffic to other devices on the network using the tap interface.
