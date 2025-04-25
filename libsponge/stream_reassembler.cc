#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by make check_lab1.

// You will need to add private members to the class declaration in stream_reassembler.hh

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _buffer(capacity, 0)
    , _occupied(capacity, false)
    , _next_index(0)
    , _head(0)
    , _buffer_size(0)
    , _eof(false)
    , _eof_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    const size_t available_capacity = _output.remaining_capacity();
    const size_t start = max(index, _next_index);
    const size_t end = min(index + data.size(), _next_index + available_capacity);

    for (size_t i = start; i < end; i++) {
        size_t buffer_idx = (_head + i - _next_index) % _capacity;
        if (_occupied[buffer_idx] == false) {
            _buffer[buffer_idx] = data[i - index];
            _occupied[buffer_idx] = true;
            _buffer_size++;
        }
    }

    if (eof && (index + data.size() == end)) {
        _eof = true;
        _eof_index = index + data.size();
    }

    string contiguous_data;
    while (_buffer_size > 0 && (_occupied[_head] == true)) {
        contiguous_data += _buffer[_head];
        _occupied[_head] = false;
        _head = (_head + 1) % _capacity;
        _next_index++;
        _buffer_size--;
    }

    if (!contiguous_data.empty()) {
        _output.write(contiguous_data);
    }

    if (_eof && _buffer_size == 0 && _next_index == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _buffer_size; }

bool StreamReassembler::empty() const { return _buffer_size == 0; }