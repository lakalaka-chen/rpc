
#include "gtest/gtest.h"
#include "rpc_server.h"
#include "tcp/tcp_client.h"
#include "spdlog/spdlog.h"

#include <thread>
#include <vector>
#include <future>
#include <mutex>
#include <atomic>
#include <iostream>
#include <sstream>

using namespace rpc;

using RpcServerPtr = std::shared_ptr<RpcServer>;
using ServerPtrFuture = std::future<RpcServerPtr>;


class RpcServerTest: public testing::Test {
public:
    ServerPtrFuture fu_;
    std::vector<uint32_t> terms_;
    std::vector<uint32_t> indexes_;
    std::vector<std::string> contents_;
    std::mutex mu_;
    RpcServerPtr server_ptr_;
    uint16_t port_;

    RpcServerTest() {

        port_ = static_cast<uint16_t>( rand() % (5200 - 1200) ) + 1200;

        spdlog::set_level(spdlog::level::debug);
        spdlog::info("It is initializing. ");

        auto create_server = [this](uint16_t port){
            RpcServerPtr server_ptr = std::make_shared<RpcServer>("Rpc Server", port);
            server_ptr->Start();
            server_ptr->Register("Echo", [](const std::string &recv, std::string &reply){
                reply = recv;
            });
            server_ptr->Register("Store", [&](uint32_t term, uint32_t index, const std::string &content){
                std::lock_guard<std::mutex> lock(mu_);
                terms_.push_back(term);
                indexes_.push_back(index);
                contents_.push_back(content);
            });
            server_ptr->Register("Add", [](int &a, int &b, int &c) {
                c = a+b;
            });
            server_ptr->HandleReceiveData([server_ptr](const std::string &recv, std::string &reply){
                std::istringstream is(recv);
                std::string method;
                is >> method;
                try {
                    if (method == "Echo") {
                        std::string recv_str;
                        is >> recv_str;
                        server_ptr->Call<const std::string&, std::string&>("Echo", std::ref(recv_str), std::ref(reply));
                    } else if (method == "Add") {
                        int a, b, c;
                        is >> a >> b;
                        server_ptr->Call<int&,int&,int&>("Add", a, b, std::ref(c));
                        reply = std::to_string(c);
                    } else if (method == "Store") {
                        uint32_t term, index;
                        std::string content;
                        is >> term >> index >> content;
                        server_ptr->Call<uint32_t, uint32_t, const std::string&>("Store", std::move(term), std::move(index), std::ref(content));
                        reply = "ok";
                    }
                } catch (const std::exception &e) {
                    spdlog::error("{}", e.what());
                }
            });
            return server_ptr;
        };
        fu_ = std::async(create_server, port_);
    }

};


TEST_F(RpcServerTest, TestEcho) {

    tcp::TcpClient client;
    client.ConnectTo("127.0.0.1", port_);
    std::string send_msg = "Echo nihao";
    std::string recv_msg;
    ASSERT_EQ(true, client.SendMsg(send_msg));
    ASSERT_EQ(true, client.RecvMsg(&recv_msg));
    ASSERT_EQ(recv_msg, send_msg.substr(5));
}


TEST_F(RpcServerTest, TestAdd) {

    tcp::TcpClient client;
    client.ConnectTo("127.0.0.1", port_);
    int a = 1;
    int b = 2;

    std::string send_msg = std::string("Add") + " " + std::to_string(a) + " " + std::to_string(b);
    std::string recv_msg;
    ASSERT_EQ(true, client.SendMsg(send_msg));
    ASSERT_EQ(true, client.RecvMsg(&recv_msg));
    ASSERT_EQ(a+b, std::stoi(recv_msg));
}


TEST_F(RpcServerTest, TestStore) {

    tcp::TcpClient client;
    client.ConnectTo("127.0.0.1", port_);
    uint32_t term = 1;
    uint32_t index = 2;
    std::string content = "store_test";

    std::string send_msg = std::string("Store") + " " + std::to_string(term) + " " + std::to_string(index) + " " + content;
    std::string recv_msg;
    ASSERT_EQ(true, client.SendMsg(send_msg));
    ASSERT_EQ(true, client.RecvMsg(&recv_msg));
    ASSERT_EQ("ok", recv_msg);

    std::unique_lock<std::mutex> lock(mu_);
    ASSERT_EQ(1, terms_.size());
    ASSERT_EQ(1, indexes_.size());
    ASSERT_EQ(1, contents_.size());
    ASSERT_EQ(term, terms_.back());
    ASSERT_EQ(index, indexes_.back());
    ASSERT_EQ(content, contents_.back());
}
