#include "pch.h"
#include "../ChatLibrary/network.h"

// Testcase for ChatLibrary
TEST(ChatLibraryTest, IocpSocket_CreateSocket) {
	network::IocpSocket iocpSocket{
		network::WinSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED)
	};

	EXPECT_NE(iocpSocket.get(), INVALID_SOCKET);
}