#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address)
    , _ip_address(ip_address)
    , _arp_cache{{ip_address.ipv4_numeric(),
                ethernet_address,
                std::numeric_limits<size_t>::max()}}
    , _pending_frames()
{
cerr << "DEBUG: Network interface has Ethernet address "
     << to_string(_ethernet_address)
     << " and IP address " << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
// 1) send_datagram: IPv4 데이터그램 전송
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // IPv4 frame
    EthernetFrame ip_frame;
    ip_frame.header().src  = _ethernet_address;
    ip_frame.header().type = EthernetHeader::TYPE_IPv4;
    ip_frame.payload()     = dgram.serialize();

    uint32_t next_hop_ip = next_hop.ipv4_numeric();

    // 2) find ARP cache
    bool has_mac = false;
    EthernetAddress next_mac;
    for (auto &entry : _arp_cache) {
        if (entry.ip == next_hop_ip && entry.timestamp > _age) {
            has_mac  = true;
            next_mac = entry.mac;
            break;
        }
    }
    if (has_mac) {
        ip_frame.header().dst = next_mac;
        _frames_out.push(ip_frame);
        return;
    }

    // 3) pending_frames에 저장 및 duplicated request 억제
    bool pending_found = false;
    for (auto &pf : _pending_frames) {
        if (pf.ip == next_hop_ip) {
            pending_found = true;
            pf.frames.push(ip_frame);
            if (_age < pf.next_request_deadline)
                return;
            break;
        }
    }
    if (!pending_found) {
        PendingFrames pf;
        pf.ip = next_hop_ip;
        pf.frames.push(ip_frame);
        pf.next_request_deadline = _age + 5000;
        _pending_frames.push_back(move(pf));
    }

    // 4) ARP 요청 생성
    ARPMessage request;
    request.opcode = ARPMessage::OPCODE_REQUEST;
    request.sender_ethernet_address = _ethernet_address;
    request.sender_ip_address = _ip_address.ipv4_numeric();
    request.target_ip_address = next_hop_ip;

    EthernetFrame frame;
    frame.header().dst = ETHERNET_BROADCAST;
    frame.header().src = _ethernet_address;
    frame.header().type = EthernetHeader::TYPE_ARP;
    frame.payload() = request.serialize();
    _frames_out.push(frame);

    // 5) 요청 기한 갱신
    for (auto &pf : _pending_frames) {
        if (pf.ip == next_hop_ip) {
            pf.next_request_deadline = _age + 5000;
            break;
        }
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // 1) 목적지 MAC 검사: 내 MAC 또는 브로드캐스트
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) {
        return {};
    }
    // 2) IPv4 프레임인 경우: 파싱 후 반환
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        if (dgram.parse(frame.payload()) == ParseResult::NoError) {
            return dgram;
        }
        return {};
    }
    // 3) ARP 프레임인 경우
    if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage msg;
        if (msg.parse(frame.payload()) != ParseResult::NoError) {
            return {};
        }

        const uint32_t sender_ip = msg.sender_ip_address;
        const EthernetAddress sender_mac = msg.sender_ethernet_address;

        // 3a) 매핑 갱신(또는 신규 삽입)
        bool found = false;
        for (auto &entry : _arp_cache) {
            if (entry.ip == sender_ip) {
                entry.mac = sender_mac;
                entry.timestamp = _age + 30000;
                found = true;
                break;
            }
        }
        if (!found) {
            _arp_cache.push_back({sender_ip,
                                        sender_mac,
                                        _age + 30000});
        }

        // 3b) 미해결 프레임 전송 및 제거
        for (auto it = _pending_frames.begin();
             it != _pending_frames.end(); ++it) {
            if (it->ip == sender_ip) {
                while (!it->frames.empty()) {
                    auto f = it->frames.front();
                    it->frames.pop();
                    f.header().dst = sender_mac;
                    _frames_out.push(f);
                }
                _pending_frames.erase(it);
                break;
            }
        }

        // 3c) ARP 요청 중 목록에서 제거
        if (msg.opcode == ARPMessage::OPCODE_REQUEST
            && msg.target_ip_address == _ip_address.ipv4_numeric()) {
            ARPMessage reply;
            reply.opcode                  = ARPMessage::OPCODE_REPLY;
            reply.sender_ethernet_address = _ethernet_address;
            reply.sender_ip_address       = _ip_address.ipv4_numeric();
            reply.target_ethernet_address = sender_mac;
            reply.target_ip_address       = sender_ip;

            EthernetFrame fr;
            fr.header().dst  = sender_mac;
            fr.header().src  = _ethernet_address;
            fr.header().type = EthernetHeader::TYPE_ARP;
            fr.payload()     = reply.serialize();
            _frames_out.push(fr);
        }
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // 경과 시간 누적
    _age += ms_since_last_tick;

    // 만료된 ARP 캐시 항목 삭제
    for (auto it = _arp_cache.begin(); it != _arp_cache.end();) {
        if (it->timestamp <= _age) {
            it = _arp_cache.erase(it);
        } else {
            ++it;
        }
    }
}