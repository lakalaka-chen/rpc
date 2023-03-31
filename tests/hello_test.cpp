#include "gtest/gtest.h"
#include "tcp/tcp_server.h"


TEST(HELLO_TEST, TEST1) {
    tcp::TcpServer server("simple server", 1236);
    server.Start();
} 