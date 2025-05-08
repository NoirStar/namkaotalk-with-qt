#pragma once
#include <string>
#include <array>
#include "../ChatLibrary/network.h"

class ChatClient {
public:
	explicit ChatClient(const std::wstring& ip, int port);
	~ChatClient();
	void Connect();
	void SendPacket(const std::string& msg);
	void RecvLoop();

private:
	network::WinSockInitializer wsaInitializer_;
	network::ScopedSocket socket_;
	std::wstring serverIp_;
	int serverPort_;

	std::array<char, network::BUFFER_SIZE> recvBuffer_{};
};