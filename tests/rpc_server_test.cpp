
#include "gtest/gtest.h"
#include "rpc_server.h"
#include "tcp/tcp_client.h"
#include "spdlog/spdlog.h"

#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <sstream>

namespace rpc {




class RpcServerTest: public testing::Test {
public:
    static const uint16_t port = 8888;
    RpcServerTest(): rpc_server_(new RpcServer("Rpc Server", port)) {
        rpc_server_->Start();
        rpc_server_->HandleReceiveData([&](const std::string &recv_msg, std::string & reply_msg){
            std::istringstream is(recv_msg);
            std::string method;
            uint32_t term, index;
            std::string content;
            is >> method;
            if (method == "AppendEntries") {
                is >> term >> index >> content;
                std::lock_guard<std::mutex> lock(mu_);
                rpc_server_->Call(method, term, index, content, this);
                reply_msg = "ok";
            } else {
                reply_msg = "ERROR: " + recv_msg+" not found.";
            }
        });

        rpc_server_->Register("AppendEntries", +[](uint32_t term, uint32_t index, const std::string &content, RpcServerTest *node){
            node->terms_.push_back(term);
            node->indexes_.push_back(index);
            node->contents_.push_back(content);
        });
    }
    ~RpcServerTest() {
        if (rpc_server_) {
            rpc_server_->Close();
        }
    }

public:
    std::shared_ptr<RpcServer> rpc_server_;
    std::vector<uint32_t> terms_;
    std::vector<uint32_t> indexes_;
    std::vector<std::string> contents_;
    std::mutex mu_;

};

TEST_F(RpcServerTest, AppendEntriesTest) {

    std::mutex close_mu;
    std::condition_variable close_cv;
    bool to_close = false;
    uint32_t term = 1;
    uint32_t index = 1;
    std::string content = "hello";
    std::thread client([&](){
        std::unique_lock<std::mutex> lock(close_mu);
        tcp::TcpClient client;
        client.ConnectTo("127.0.0.1", RpcServerTest::port);
        std::string recv_msg;
        std::string send_msg = std::string("AppendEntries") + " " + std::to_string(term) + " " + std::to_string(index) + " " + content;
        client.SendMsg(send_msg);
        client.RecvMsg(&recv_msg);
        spdlog::info(recv_msg);
        to_close = true;
        close_cv.notify_one();
    });
    client.join();

    std::unique_lock<std::mutex> lock(close_mu);
    while (!to_close) {
        close_cv.wait(lock);
    }

    ASSERT_EQ(terms_.size(), 1);
    ASSERT_EQ(indexes_.size(), 1);
    ASSERT_EQ(contents_.size(), 1);

    ASSERT_EQ(terms_.back(), term);
    ASSERT_EQ(indexes_.back(), index);
    ASSERT_EQ(contents_.back(), content);

}



}