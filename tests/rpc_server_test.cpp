
#include "gtest/gtest.h"
#include "rpc_server.h"
#include "spdlog/spdlog.h"

#include <vector>
#include <mutex>
#include <sstream>

namespace rpc {



class RegisterTest: public testing::Test {
public:
    std::shared_ptr<RpcServer> server_;
    RegisterTest(): server_(new RpcServer("RPC SERVER 002", 1111)) {
        spdlog::set_level(spdlog::level::debug);
    }

};


TEST_F(RegisterTest, EchoFunctionTest) {
    server_->Register("Echo", Command<>([](){ std::cout << "Echo Server welcome to you. " << std::endl; }));
    server_->Execute("Echo");
}

TEST_F(RegisterTest, SimpleAddTest) {
    server_->Register("Add", Command<int,int,int>([](int a, int b, int c){
        std::cout << (a+b) << std::endl;
        c = a+b;
    }));
    int a = 2;
    int c = 0;
    server_->Execute("Add", a, 2, c);
}


}