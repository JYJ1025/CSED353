Assignment 1 Writeup
=============

My name: Yoonjae Jung

My POVIS ID: jyjllll1025

My student ID (numeric): 20210509

This assignment took me about 10 hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
- output: A ByteStream object to store the reassembled output.
- _capacity: The maximum number of bytes the reassembler can store at any given time.
- _buffer: A std::vector<char> that temporarily holds out-of-order bytes.
- _occupied: A std::vector<bool> to track which positions in _buffer contain valid data.
- _next_index: The global index of the next expected byte to be written into _output.
- _head: The logical starting index in _buffer corresponding to _next_index.
- _buffer_size: The number of stored bytes that are waiting to be written to _output.
- _eof: A flag indicating if the end-of-file (EOF) marker has been received.
- _eof_index: The expected index of the last byte in the stream, used to determine when reassembly is complete.

Implementation Challenges:
- indexing method between bytestream and buffer was unfamiliar so it takes many effort to understand it. 
- Defining the start and end part that buffer stores was challenge for me.

Remaining Bugs:
- Having passed all the tests, there are no bugs I think. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
