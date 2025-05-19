#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    // Your code here.
    _routing_table.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

static uint32_t make_mask(uint8_t prefix_length) {
    if (prefix_length == 0) {
        return 0x00000000u;
    } else if (prefix_length == 32) {
        return 0xFFFFFFFFu;
    } else {
        return ~((1u << (32 - prefix_length)) - 1);
    }
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // check TTL 
    if (dgram.header().ttl <= 1) {
        return;
    }
    dgram.header().ttl--;

    // find dest. IP
    const uint32_t dst_ip = dgram.header().dst;

    // longest-prefix-match
    int best_len = -1;
    std::tuple<uint32_t, uint8_t, std::optional<Address>, size_t> best_entry;

    for (size_t i = 0; i < _routing_table.size(); i++) {
        uint32_t prefix = std::get<0>(_routing_table[i]);
        uint8_t  plen   = std::get<1>(_routing_table[i]);

        uint32_t mask = make_mask(plen);

        // compare (dst_ip & mask) & prefix
        if ((dst_ip & mask) == prefix) {
            if (static_cast<int>(plen) > best_len) {
                best_len   = plen;
                best_entry = _routing_table[i];
            }
        }
    }

    // drop if there is no matching
    if (best_len < 0) {
        return;
    }

    // bring next hop & interface num
    std::optional<Address> hop_opt = std::get<2>(best_entry);
    size_t if_num = std::get<3>(best_entry);
    Address next_hop = hop_opt.value_or(Address::from_ipv4_numeric(dst_ip));

    // sending
    interface(if_num).send_datagram(dgram, next_hop);
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}