#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{static_cast<uint32_t>(std::random_device()())})),
      _initial_retransmission_timeout(retx_timeout),
      _stream(capacity),
      _outstanding_segments(),
      _timer(retx_timeout)
{}

//! 전송되었지만 아직 ack를 받지 못한 byte의 수
uint64_t TCPSender::bytes_in_flight() const {
    size_t bytes = 0;

    for (const auto &seg : _outstanding_segments) {
        const auto &hdr = seg.segment.header();
        
        size_t seg_length = seg.segment.payload().str().size();

        if (hdr.syn || hdr.fin)
            seg_length += 1;

        bytes += seg_length;
    }
    return bytes;
}

//! bytestream에서 가능한 많은 byte를 읽어 TCP segment로 전송
void TCPSender::fill_window() {
    while (true) {
        //! 1. eof or already fin sent
        if (_stream.eof())
            break;

        size_t effective_window = std::max(_window_size, uint16_t{1});

        //! 전송 불가능
        if (bytes_in_flight() >= effective_window)
            break;
        size_t available = effective_window - bytes_in_flight();

        //! 2. make new segment
        TCPSegment segment;
        segment.header().seqno = _isn + _next_seqno;

        //! 2-1. SYN flag
        if (_next_seqno == 0)
            segment.header().syn = true;

        //! 2-2. payload
        size_t read_size = std::min({ _stream.buffer_size(), available - segment.length_in_sequence_space(), TCPConfig::MAX_PAYLOAD_SIZE });
        segment.payload() = _stream.read(read_size);

        //! 2-3. Fin flag (eof or enough space)
        if (_stream.eof() && available > segment.length_in_sequence_space()) {
            segment.header().fin = true;
        }

        if (segment.length_in_sequence_space() == 0)
            break;

        //! 3. push segment
        _segments_out.push(segment);
        _outstanding_segments.push_back({ _next_seqno, segment });
        _next_seqno += segment.length_in_sequence_space();

        //! 4. start timer
        if (!_timer.is_running()) {
            _timer.start();
        }
    }
}


//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno, _isn, _last_ack);

    if (abs_ackno > _next_seqno)
        return;

    //! 1. remove all acked segment
    while (!_outstanding_segments.empty() &&
           (_outstanding_segments.front().abs_seqno +
            _outstanding_segments.front().segment.length_in_sequence_space() <= abs_ackno)) {
        _outstanding_segments.pop_front();
    }

    //! 2. 새로운 ack가 들어온 경우
    if (abs_ackno > _last_ack) {
        _consecutive_retransmissions = 0;
        _last_ack = abs_ackno;
        _timer.set_timeout(_initial_retransmission_timeout);
        //! restart  timer if _outstanding_segments still exsist
        if (!_outstanding_segments.empty()) {
            _timer.stop();
            _timer.start();
        }
    }

    //! 3. stop timer if all _outstanding_segments acked
    if (_outstanding_segments.empty())
        _timer.stop();
    
    //! 4. update _window_size
    _window_size = window_size;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    //! 1. update tick
    _timer.tick(ms_since_last_tick);
    
    if (_timer.is_expired()) {
        //! 2. Stop timer if all _outstanding_segments acked
        if (_outstanding_segments.empty()) {
            _timer.stop();
            return;
        }

        //! 3. Resend oldest unacked segment
        const auto &oldest = _outstanding_segments.front();
        _segments_out.push(oldest.segment);


        if (_window_size > 0) {
            _timer.set_timeout(_timer.current_timeout() * 2);
        }

        _consecutive_retransmissions++;

        //! restart timer
        _timer.stop();
        _timer.start();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _consecutive_retransmissions;
}

void TCPSender::send_empty_segment() {
     TCPSegment seg;
     seg.header().seqno = _isn + _next_seqno;
     _segments_out.push(seg);
}
