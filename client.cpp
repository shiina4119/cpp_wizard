#include "packet.hpp"
#include <asio.hpp>
#include <fstream>
#include <iostream>
#include <set>

using asio::ip::tcp;

std::vector<u8> read_into_buffer(tcp::socket &socket) {
    std::vector<u8> buffer(17);
    asio::error_code ec;
    size_t bytes_read = asio::read(socket, asio::buffer(buffer), ec);
    if (ec == asio::error::eof) {
        return {};
    } else if (ec) {
        throw asio::system_error(ec);
    }
    buffer.resize(bytes_read);
    return buffer;
}

int main() {
    try {
        asio::io_context ioc;
        tcp::socket socket(ioc);
        tcp::resolver resolver(ioc);

        std::vector<response_packet> response_packets;
        std::set<u32> recv_seq;
        u32 last_seq = 0;

        asio::connect(socket, resolver.resolve("127.0.0.1", "3000"));
        auto stream_all_request_packet = make_request_packet(1, 0);
        asio::write(socket, asio::buffer(stream_all_request_packet));

        try {
            while (true) {
                auto buffer = read_into_buffer(socket);
                if (buffer.empty()) {
                    break;
                }

                response_packet packet(buffer);
                response_packets.push_back(packet);
                recv_seq.insert(packet.get_sequence());
                last_seq = std::max(last_seq, packet.get_sequence());
            }
        } catch (std::exception &e) {
            std::cerr << "Stream ended: " << e.what() << std::endl;
        }

        std::cout << "Streaming data read" << std::endl;

        socket.close();

        for (u32 i = 1; i <= last_seq; ++i) {
            if (!recv_seq.count(i)) {
                try {
                    asio::connect(socket,
                                  resolver.resolve("127.0.0.1", "3000"));
                    auto missing_seq_request_packet = make_request_packet(2, i);
                    asio::write(socket,
                                asio::buffer(missing_seq_request_packet));

                    auto buffer = read_into_buffer(socket);
                    response_packets.push_back(response_packet(buffer));

                    socket.close();
                } catch (const std::exception &e) {
                    std::cerr << "Failed to retrieve packet " << i << ": "
                              << e.what() << std::endl;
                }
            }
        }

        std::cout << "Missing packets retrieved" << std::endl;

        std::sort(response_packets.begin(), response_packets.end(),
                  [](const response_packet &a, const response_packet &b) {
                      return a.get_sequence() < b.get_sequence();
                  });

        json output = json::array();
        for (const auto &packet : response_packets) {
            output.push_back(packet.des_json());
        }

        std::ofstream file("output.json");
        file << output.dump(4);
        file.close();

        std::cout << "Data dumped to output.json" << std::endl;

    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
