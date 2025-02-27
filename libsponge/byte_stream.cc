#include "byte_stream.hh"
#include <deque>
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { DUMMY_CODE(capacity); }

size_t ByteStream::write(const string &data) {
    const size_t free_space = min(data.size(), remaining_capacity());
    
    for (size_t = i = 0; i < free_space; i++) {
        buffer_.push_back(data[i]);
    }

    bytes_writen_ += free_space;

    return {};
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    const size_t readable = min(_buffer.size(), len);
    return string(buffer_.begin(), buffer_.begin() + readable);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    const size_t readable = min(_buffer.size(), len);

    for (size_t = i = 0; i < readable; i++) {
        buffer_.pop_front();
    }

    bytes_read_ += readable;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string output = peek_output(len);
    pop_output(len);
    
    return output;
}

void ByteStream::end_input() {}

bool ByteStream::input_ended() const { return {}; }

size_t ByteStream::buffer_size() const { return {}; }

bool ByteStream::buffer_empty() const { return {}; }

bool ByteStream::eof() const { return false; }

size_t ByteStream::bytes_written() const { return {}; }

size_t ByteStream::bytes_read() const { return {}; }

size_t ByteStream::remaining_capacity() const { return {}; }
