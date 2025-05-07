#include "byte_stream.hh"
<<<<<<< HEAD
// #include <deque>
=======
>>>>>>> upstream/lab5-startercode

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

<<<<<<< HEAD
ByteStream::ByteStream(const size_t capacity) : buffer_(), capacity_(capacity) {}

size_t ByteStream::write(const string &data) {
    const size_t free_space = min(data.size(), remaining_capacity());

    for (size_t i = 0; i < free_space; i++) {
        buffer_.push_back(data[i]);
    }

    bytes_written_ += free_space;

    return free_space;
=======
ByteStream::ByteStream(const size_t capacity) { DUMMY_CODE(capacity); }

size_t ByteStream::write(const string &data) {
    DUMMY_CODE(data);
    return {};
>>>>>>> upstream/lab5-startercode
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
<<<<<<< HEAD
    const size_t readable = min(buffer_.size(), len);
    return string(buffer_.begin(), buffer_.begin() + readable);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t readable = min(buffer_.size(), len);

    buffer_.erase(buffer_.begin(), buffer_.begin() + readable);

    bytes_read_ += readable;
}
=======
    DUMMY_CODE(len);
    return {};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { DUMMY_CODE(len); }
>>>>>>> upstream/lab5-startercode

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
<<<<<<< HEAD
    string output = peek_output(len);
    pop_output(len);

    return output;
}

void ByteStream::end_input() { end_ = true; }

bool ByteStream::input_ended() const { return end_; }

size_t ByteStream::buffer_size() const { return buffer_.size(); }

bool ByteStream::buffer_empty() const { return buffer_.empty(); }

bool ByteStream::eof() const { return end_ && buffer_.empty(); }

size_t ByteStream::bytes_written() const { return bytes_written_; }

size_t ByteStream::bytes_read() const { return bytes_read_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - buffer_.size(); }
=======
    DUMMY_CODE(len);
    return {};
}

void ByteStream::end_input() {}

bool ByteStream::input_ended() const { return {}; }

size_t ByteStream::buffer_size() const { return {}; }

bool ByteStream::buffer_empty() const { return {}; }

bool ByteStream::eof() const { return false; }

size_t ByteStream::bytes_written() const { return {}; }

size_t ByteStream::bytes_read() const { return {}; }

size_t ByteStream::remaining_capacity() const { return {}; }
>>>>>>> upstream/lab5-startercode
