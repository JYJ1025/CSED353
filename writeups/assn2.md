Assignment 2 Writeup
=============

My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 12 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
- TCPReceiver traslate the wrapped seqno into the correct absolute seqno. 
- I used SYN, FIN related member variables to handle the wrapped seqno.
- For wrap function, I use modulo to traslate seqno and use plus, minus modulo for unrap function.

Implementation Challenges:
- It takes time to understand the relation between TCPrecevier, reassembler, bytestream.
- Finding correct unwrap function was hard.

Remaining Bugs:
- Having passed all the tests, there are no bugs I think. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
