#include "packet.hpp"

u32 response_packet::parse_big_endian(const std::vector<u8> &data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

response_packet::response_packet(const std::vector<u8> &buffer) {
    auto begin = buffer.begin();
    symbol = std::string(begin, begin + 4);
    trade_type = static_cast<char>(buffer[4]);
    qty = parse_big_endian(std::vector<u8>(begin + 5, begin + 9));
    price = parse_big_endian(std::vector<u8>(begin + 9, begin + 13));
    seq = parse_big_endian(std::vector<u8>(begin + 13, buffer.end()));
}

json response_packet::des_json() const {
    return {{"symbol", symbol},
            {"trade_type", std::string(1, trade_type)},
            {"quantity", qty},
            {"price", price},
            {"sequence", seq}};
}

u32 response_packet::get_sequence() const { return seq; }

std::vector<u8> make_request_packet(u8 call_type, u8 resend_seq = 0) {
    return {call_type, resend_seq};
}
