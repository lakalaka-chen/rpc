
#include "rpc_server.h"
#include "tcp/tcp_socket.h"

#include <string>
#include "spdlog/spdlog.h"
#include <functional>


namespace rpc {

using tcp::TcpSocket;

RpcServer::RpcServer(const std::string& name, uint16_t port)
    : TcpServer(name, port) {

}

RpcServer::~RpcServer() {
    Close();
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
}

}