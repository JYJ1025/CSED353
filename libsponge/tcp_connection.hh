#ifndef SPONGE_LIBSPONGE_TCP_FACTORED_HH
#define SPONGE_LIBSPONGE_TCP_FACTORED_HH

#include "tcp_config.hh"
#include "tcp_receiver.hh"
#include "tcp_sender.hh"
#include "tcp_state.hh"
#include <optional>
#include "wrapping_integers.hh"

//! \brief A complete endpoint of a TCP connection
class TCPConnection {
  private:
    TCPConfig       _cfg;
    TCPReceiver     _receiver{_cfg.recv_capacity};
    TCPSender       _sender{_cfg.send_capacity, _cfg.rt_timeout, _cfg.fixed_isn};

    //! outbound queue of segments that the TCPConnection wants sent
    std::queue<TCPSegment> _segments_out{};

    //! linger timeout handling
    bool _linger_after_streams_finish{true};
    bool   _is_reset{false};
    size_t _connection_age{0};           // 연결 시작 이후 누적 ms
    size_t _last_segment_received{0};    // 마지막 segment 수신 시점 (connection_age 기준)

    std::optional<WrappingInt32> _current_ackno{std::nullopt};

  public:
    //! \name "Input" interface for the writer
    void connect();
    size_t write(const std::string &data);
    size_t remaining_outbound_capacity() const;
    void end_input_stream();
    //!@}

    //! \name "Output" interface for the reader
    ByteStream &inbound_stream() { return _receiver.stream_out(); }
    //!@}

    //! \name Accessors used for testing
    size_t bytes_in_flight() const;
    size_t unassembled_bytes() const;
    size_t time_since_last_segment_received() const;
    TCPState state() const { return {_sender, _receiver, active(), _linger_after_streams_finish}; };
    //!@}

    //! \name Methods for the owner or OS to call
    void segment_received(const TCPSegment &seg);
    void tick(const size_t ms_since_last_tick);
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    bool active() const;
    //!@}

    //! helper: pull all segs from sender, fill ACK+WINDOW, push to _segments_out
    TCPSegment dequeue_sender_segment();
    void set_window(TCPSegment &seg);
    void set_ack(TCPSegment &seg);
    void send_segment();
    void handle_rst();
    ////////////////////////////////////////////////////////
    //! Construct a new connection from a configuration
    explicit TCPConnection(const TCPConfig &cfg) : _cfg{cfg} {}

    //! moving is allowed; copying is disallowed; default construction not possible
    ~TCPConnection();  //!< destructor sends a RST if the connection is still open
    TCPConnection() = delete;
    TCPConnection(TCPConnection &&other) = default;
    TCPConnection &operator=(TCPConnection &&other) = default;
    TCPConnection(const TCPConnection &other) = delete;
    TCPConnection &operator=(const TCPConnection &other) = delete;
    //!@}
    ////////////////////////////////////////////////////////
};

#endif  // SPONGE_LIBSPONGE_TCP_FACTORED_HH