add_library (stream_copy STATIC bidirectional_stream_copy.cc)

add_sponge_exec (udp_tcpdump ${LIBPCAP})
add_sponge_exec (tcp_native stream_copy)
add_sponge_exec (tun)
add_sponge_exec (tcp_udp stream_copy)
add_sponge_exec (tcp_ipv4 stream_copy)
add_sponge_exec (tcp_ip_ethernet stream_copy)
add_sponge_exec (webget)
add_sponge_exec (tcp_benchmark)

