Assignment 5 Writeup
=============

My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 25 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): I used assn4 best codes

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
I designed the Network Interface class as a bridge that mediates between the Internet layer and the Ethernet link layer of IPv4. The class has its own MAC address and IP address, a queue that stores Ethernet frames to be transmitted, an ARP cache that manages IP→MAC mapping, and a structure that stores waiting frames for each destination where MAC addresses are not yet known. When transmitting IPv4 datagrams, first check the ARP cache for the MAC address of the next hop, and if there is a valid mapping, it is immediately encapsulated into an Ethernet frame and placed in the transmission queue. If there is no mapping or has expired, the datagram is kept in the standby queue, and a broadcast ARP request is generated and sent. The received Ethernet frame is processed only when the destination MAC is the interface's own address or broadcast, and the IPv4 frame is parsed and delivered to the upper layer, and the ARP frame sends the waiting frame after updating the cache and automatically generates and responds to the ARP response if it is the target of the ARP request. In addition, the method tick (ms) accumulates elapsed time in milliseconds, expiring ARP cache items, and manages to send requests back at the appropriate time for standby frames that require retransmission.

Implementation Challenges:
Coordinating the ARP retransmission cycle was the most difficult part of the implementation process. When there was no response, it was necessary to resend at an appropriate time without sending duplicate ARP requests within 5 seconds for the same IP, so it was necessary to carefully write the logic to track the next available request time for each IP.

Remaining Bugs:
- Having passed all the tests, there are no bugs I think. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
