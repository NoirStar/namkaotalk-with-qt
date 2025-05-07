#pragma once
#include <string>
#include "../ChatLibrary/network.h"

class ChatClient {
public:
	explicit ChatClient(const std::wstring& ip, int port);
	~ChatClient();
	void Connect();
	void SendPacket(const std::string& msg);
	void RecvLoop();

private:
	network::SocketObject socket_;
	std::wstring serverIp_;
	int serverPort_;
};