#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>
<<<<<<< HEAD
#include <optional>
#include <random>
#include <algorithm>
=======
>>>>>>> upstream/lab5-startercode

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
<<<<<<< HEAD

class Timer {
  private:
    size_t _timeout;
    size_t _elapsed = 0;
    bool _running = false;
  
  public:
    Timer(const uint16_t timeout)
      :_timeout(timeout) {}

    //! 매 tick 마다 호출하여 경과 시간 업데이트
    void tick(const size_t time_lapse) {
      if (!_running)
        return;
      _elapsed += time_lapse;
    }

    //! timer가 만료되었는지 판단
    bool is_expired() const {
      return _running && _elapsed >= _timeout;
    }


    //! start timer
    void start() {
      _running = true;
      _elapsed = 0;
    }

    //! stop timer
    void stop() {
      _elapsed = 0;
      _running = false;
    }

    size_t current_timeout() const { return _timeout; }
    bool is_running() const { return _running; }
    void set_timeout(const size_t t) { _timeout = t; }
};

=======
>>>>>>> upstream/lab5-startercode
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

<<<<<<< HEAD
    //! added
    //! window size
    uint16_t _window_size{1};

    //! outstanding statements
    struct OutstandingSegment {
        uint64_t abs_seqno;  
        TCPSegment segment;
    };
    std::deque<OutstandingSegment> _outstanding_segments;

    //! timer
    Timer _timer;

    //! 연속 재전송 횟수
    unsigned int _consecutive_retransmissions{0};

    bool _after_fin{false};
    uint64_t _last_ack{0};

=======
>>>>>>> upstream/lab5-startercode
  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
<<<<<<< HEAD


=======
>>>>>>> upstream/lab5-startercode
