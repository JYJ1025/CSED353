Assignment 3 Writeup
=============

My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 15 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
- TCP sender maintains internal state such as the next sequence number (_next_seqno), last acknowledged sequence number (_last_ack), window size (_window_size), and a list of outstanding (unacknowledged) segments (_outstanding_segments).
- Timer class help manage time-based retransmissions with functionalities like start, stop, tick, and is_expired.
- fill_window() is the core logic to make segemnts sending data segments, SYN, and FIN when appropriate.It uses deque OutstandingSegment to manage unacked segments.
- ack_received() method manage remove all acked segment and new ack recevied situation and control timer. 
- tick() method is called periodically to simulate time passing, it retransmit unacknowledged segment and control timer. 
- send_empty_segment() for handling edge cases like window updates or FIN-only signaling.

Implementation Challenges:
- Find edge cases for simultaneous syn/fin and consider syn/fin logic was difficult, such as add _after_fin to stop fill_window works after fin.
- Make timer and find start/restart timer logic was challenge

Remaining Bugs:
- Having passed all the tests, there are no bugs I think. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
