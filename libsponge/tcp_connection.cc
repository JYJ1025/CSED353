#include "tcp_connection.hh"

#include <iostream>
#include <algorithm>
#include <limits>
// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const {return _sender.stream_in().remaining_capacity();}
size_t TCPConnection::bytes_in_flight() const {return _sender.bytes_in_flight();}
size_t TCPConnection::unassembled_bytes() const {return _receiver.unassembled_bytes();}
size_t TCPConnection::time_since_last_segment_received() const {return _connection_age - _last_segment_received;;}

// 다음 송신 후보 세그먼트를 꺼내옴
TCPSegment TCPConnection::dequeue_sender_segment() {
    TCPSegment seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    return seg;
}

// 윈도우 크기 채우기
void TCPConnection::set_window(TCPSegment &seg) {
    size_t recv_win = _receiver.window_size();
    size_t max_uint16 = numeric_limits<uint16_t>::max();
    seg.header().win = static_cast<uint16_t>(min(recv_win, max_uint16));
}

// ACK 필드 채우기 (고정 ACK번호 관리 포함)
void TCPConnection::set_ack(TCPSegment &seg) {
    if (auto cur = _receiver.ackno()) {
        seg.header().ack = true;
        if (!_current_ackno.has_value() || _segments_out.empty()) {
            _current_ackno = *cur;
        }
        seg.header().ackno = _current_ackno.value();
    } else {
        seg.header().ack = false;
    }
}

// send_segment: 한 개만
void TCPConnection::send_segment() {
    if (_sender.segments_out().empty()) return;
    TCPSegment seg = dequeue_sender_segment();
    set_window(seg);
    set_ack(seg);
    _segments_out.push(seg);
}

// 완전 비우고, 빈 세그먼트 만들고, RST 플래그 켜고, 보내기
void TCPConnection::send_rst() {
    _sender.segments_out() = {};
    _sender.send_empty_segment();
    _sender.segments_out().front().header().rst = true;
    send_segment();
}

void TCPConnection::handle_rst() {
    send_rst();
    _is_reset = true;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    return;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    // 1) 연결 시작 전, SYN 없는 첫 패킷은 무시
    if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute()==0 && !seg.header().syn)
        return;
    // 2) RST 수신 시 연결 강제 종료
    if (seg.header().rst) {
        handle_rst()
    }
    // 3) TCPReceiver로 전달
    _receiver.segment_received(seg);
    // 4) update _last_segment_received
    _last_segment_received = _connection_age;
    
    // 5) receiver stream은 처리 완료, sender stream은 처리 완료 X인 경우, linger 중지
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof())
        _linger_after_streams_finish = false;

    // 6) ACK field가 있으면 sender에게 전달
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    // 7) 아직 연결 개시 전 (SYN만 보낸 상태) 처리: 윈도우 채우고 1세그 보냄
    if (_sender.next_seqno_absolute() == 0) {
        _sender.fill_window();  
        send_segment();
        return;
    }

    // 8) window size에 여유가 생겼을 수 있으므로, 재전송/새 데이터 채움
    _sender.fill_window();
    while (!_sender.segments_out().empty())
        send_segment();
}

bool TCPConnection::active() const {
    // 아직 데이터 흐름 중이거나 retransmit 대기 있으면 active
    bool streams_alive = !(_receiver.stream_out().eof()
                          && _sender.stream_in().eof()
                          && _sender.bytes_in_flight() == 0);
    if (_is_reset) {
        return false;
    }
    if (streams_alive) {
        return true;
    }
    // linger 남아 있으면 아직 active
    if (!_linger_after_streams_finish) {
        return false;
    }

    return _connection_age - _last_segment_received < 10 * _cfg.rt_timeout;
}

size_t TCPConnection::write(const std::string &data) {
    size_t n = _sender.stream_in().write(data);
    _sender.fill_window();
    while (!_sender.segments_out().empty())
        send_segment();
    return n;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    // if (_is_reset) 
    //     return;

    _connection_age += ms_since_last_tick;
    _sender.tick(ms_since_last_tick); // update retransmission timer

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        handle_rst();
    }

    while (!_sender.segments_out().empty())
        send_segment();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();      
    while (!_sender.segments_out().empty())
        send_segment();
}

void TCPConnection::connect() {
    _sender.fill_window();
    while (!_sender.segments_out().empty())
        send_segment();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            // Your code here: need to send a RST segment to the peer
            send_rst();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}