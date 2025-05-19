Assignment 6 Writeup
=============

My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 5 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): I'm using assn4 best code. 

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
The router holds multiple AsyncNetworkInterface objects, each wrapping a NetworkInterface and providing an asynchronous API via an internal queue _datagrams_out. Incoming Ethernet frames are fed to each interface’s recv_frame, which defers IPv4 datagram delivery by enqueuing them for later routing.

The routing table is a vector of tuples, route_prefix and prefic_length define an IP-prefix to match. next_hop is empty for directly attatched networks. For each datagram, decremental TTL and drop if TTL belows than 1. Iterate through routing table, compute a bitmask and find all entries where (dst_ip & mask) == route_prefix. If next_hop is specified, forward directly to the datagram’s destination address. The public route() method polls each interface’s datagrams_out queue, forwarding every queued datagram in turn. 

Implementation Challenges:
Crafting make_mask correctly for edge cases (0 and 32) and ensuring no off-by-one errors in bit shifts.

Remaining Bugs:
Having passed all the tests, there are no bugs I think. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
