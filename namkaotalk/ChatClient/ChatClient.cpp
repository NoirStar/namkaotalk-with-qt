#include "ChatClient.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

ChatClient::ChatClient(const std::wstring& ip, int port)
	: serverIp_(ip), serverPort_(port)
	, wsaInitializer_() {
}

ChatClient::~ChatClient() {
}

void ChatClient::Connect() {
	socket_ = network::ScopedSocket(socket(AF_INET, SOCK_STREAM, 0));
	if (!socket_.is_valid()) {
		std::cerr << "[!] Socket creation failed: " << WSAGetLastError() << std::endl;
		throw std::runtime_error("Socket creation failed");
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(static_cast<u_short>(serverPort_));
	
	if (InetPton(AF_INET, serverIp_.c_str(), &serverAddr.sin_addr) != 1) {
		std::wcerr << "[!] Invalid address: " << serverIp_ << std::endl;
		throw std::runtime_error("Invalid address");
	}

	if (connect(socket_.get(), reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "[!] Connect failed: " << WSAGetLastError() << std::endl;
		throw std::runtime_error("Connect failed");
	}

	std::wcout << "[+] Connected to server: " << serverIp_ << ":" << serverPort_ << std::endl;
}

void ChatClient::SendPacket(const std::string& msg) {
	if (send(socket_.get(), msg.c_str(), static_cast<int>(msg.size()), 0) == SOCKET_ERROR) {
		std::cerr << "[!] Send failed: " << WSAGetLastError() << std::endl;
	}
}

void ChatClient::RecvLoop() {

	while (true) {

		int bytesReceived = recv(socket_.get(), recvBuffer_.data(), recvBuffer_.size(), 0);
		if (bytesReceived <= 0) {
			std::cerr << "[!] Connection closed or error: " << WSAGetLastError() << std::endl;
			break;
		}

		std::cout << std::string(recvBuffer_.data(), bytesReceived) << std::endl;
	}
}