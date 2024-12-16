#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

using u32 = uint32_t;
using u8 = uint8_t;

class response_packet {
    std::string symbol;
    char trade_type;
    u32 qty;
    u32 price;
    u32 seq;
    u32 parse_big_endian(const std::vector<u8> &);

public:
    response_packet(const std::vector<u8> &);
    u32 get_sequence() const;
    json des_json() const;
};

std::vector<u8> make_request_packet(u8, u8);
