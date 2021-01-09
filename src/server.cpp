#include "common/ip_address.h"
#include "common/udp_socket.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

struct ServerOptions {
    uint16_t port;
};

ServerOptions ParseCmdline(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error("wrong number of parameters");
    }
    ServerOptions ret;
    std::istringstream ss(argv[1]);
    char tail;
    if (!(ss >> ret.port) || ss >> tail) {
        std::ostringstream err;
        err << "invalid port number: \"" << argv[1] << "\"";
        throw std::runtime_error(err.str());
    }
    return ret;
}

int main(int argc, char** argv) {
    ServerOptions opts;
    try {
        opts = ParseCmdline(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "usage: " << argv[0] << " PORT" << std::endl;
        return 1;
    }

    UdpSocket socket(opts.port);

    while (true) {
        auto [alice_buf, alice_addr] = socket.RecvFrom();
        auto [bob_buf, bob_addr] = socket.RecvFrom();
        std::cout << alice_addr.ToString() << " <=> " << bob_addr.ToString() << std::endl;
        socket.SendTo(alice_addr.ToBytes(), bob_addr);
        socket.SendTo(bob_addr.ToBytes(), alice_addr);
    }
}
