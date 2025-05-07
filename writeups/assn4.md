Assignment 4 Writeup
=============

<<<<<<< HEAD
My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 25 hours to do (including the time on studying, designing, and writing the code).
=======
My name: [your name here]

My POVIS ID: [your povis id here]

My student ID (numeric): [your student id here]

This assignment took me about [n] hours to do (including the time on studying, designing, and writing the code).
>>>>>>> upstream/lab5-startercode

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.00, 0.00]

Program Structure and Design of the TCPConnection:
<<<<<<< HEAD
The TCPConnection class is clearly designed to be modular. Within the class, TCPSender and TCP Receiver, which separate the roles of data transmission and reception, were placed as independent components to achieve clear accountability separation. TCPConnection effectively manages these two components to manage the entire life cycle of TCP sessions, such as establishing connections, exchanging data, and disconnecting.
Health management is achieved through variables such as _connection_age and _last_segment_received, which are utilized to control timer management and "linger-after-close" behavior that is maintained for a certain period of time after connection termination. Bulian flags such as _is_reset and _linger_after_streams_finish are also used to indicate error conditions or to control normal shutdown procedures.
Incoming TCP segments are processed by the segment_received() method following a well-defined processing procedure, in which segments are delivered to the receiver, timers are updated, and ACK processing is performed. If necessary, a keep-alive response is generated, and finally retransmissions or new data transfer logic is called. Since the exported segment always passes through the send_segment() method, each segment contains the correct window size and ACK field.
The retransmission control is mainly concentrated in the tick() method. This method advances the connection time, activates the transmitter's retransmission timer, and forces the connection to reset when the maximum number of consecutive retransmissions is exceeded. The auxiliary methods that the caller can check, bytes_in_flight(), or the transmitter's retransmission counter, help determine the current flow-control status.

Implementation Challenges:
Implementing the "linger-after-close" action was particularly tricky. To ensure that exactly ten round-trip time timers started and ended at the right time, _last_segment_received_received was carefully updated and coordinated with the ringer flag.
Handling the initial SYN/ACK handshake also required special exceptions. Until the initial sequence number was sent or the ACK number was determined, the connection had to ignore any other segment other than SYN, allow exactly SYN, and then properly trigger the first response.
The keep-alive logic also increased its complexity. The connection had to detect a segment with length zero at the exact expected sequence number, which required it to respond with an empty ACK proving that the connection was alive.
Error handling and abnormal connection termination required the task of transmitting reset segments, setting status flags, and converting both transmitter and receiver streams to error states.
Finally, in the process of correctly informing the window size of the receiver, it was necessary to clamp it so that it did not exceed the maximum size that can be expressed in 16 bits. This prevented the window field from overflowing.

Remaining Bugs:
Two of Lab4's current tests have failed.
Test 39: t_ack_rst
Test 41: t_ack_rst_win
Both tests deal with situations where a TCP connection has received a RST (Reset) segment. Upon receipt of this segment, the TCP connection must terminate immediately and enter an error state. However, the code currently implemented did not handle it correctly, resulting in the connection not being terminated normally and continuing to be maintained.

=======
[]

Implementation Challenges:
[]

Remaining Bugs:
[]
>>>>>>> upstream/lab5-startercode

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
