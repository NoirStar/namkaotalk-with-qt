#include "ClientSession.h"
#include <iostream>

ClientSession::ClientSession(SOCKET socket) noexcept
	: socket_(socket)
	, recvOv_(network::IO_TYPE::RECV)
	, sendOv_(network::IO_TYPE::SEND) {

	std::cout << "[+] Client Connected : " << socket_.get() << std::endl;
}

ClientSession::~ClientSession() {

	std::cout << "[-] Client Disconnected : " << socket_.get() << std::endl;
}

void ClientSession::RecvPacket() {
	
	DWORD flags = 0, bytes = 0;

	int ret = WSARecv(
		socket_.get(),
		&recvOv_.wsabuf,
		1,
		&bytes,
		&flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&recvOv_),
		nullptr
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[!] WSARecv failed: " << WSAGetLastError() << std::endl;
		return;
	}
}

void ClientSession::SendPacket(const char* data, size_t len) {

	if (len > sizeof(sendOv_.buffer)) {
		std::cerr << "[!] Data length exceeds buffer size" << std::endl;
		return;
	}

	memcpy(sendOv_.buffer, data, len);
	sendOv_.wsabuf.len = static_cast<ULONG>(len);

	DWORD bytes;
	int ret = WSASend(
		socket_.get(),
		&sendOv_.wsabuf,
		1,
		&bytes,
		0,
		reinterpret_cast<LPWSAOVERLAPPED>(&sendOv_),
		nullptr
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[!] WSASend failed: " << WSAGetLastError() << std::endl;
		return;
	}
}

void ClientSession::OnRecvCompleted(DWORD bytes) {
	if (bytes == 0) {
		std::cerr << "[!] Connection closed by client" << std::endl;
		return;
	}

	std::cout << "[+] Received " << bytes << " bytes from client: "
		<< socket_.get() << std::endl;
	// Process received data
	std::cout << "Data: " << recvOv_.buffer << std::endl;

	RecvPacket();
}

void ClientSession::OnSendCompleted(DWORD bytes) {
	if (bytes == 0) {
		std::cerr << "[!] Connection closed by client" << std::endl;
		return;
	}

	std::cout << "[+] Sent " << bytes << " bytes to client: "
		<< socket_.get() << std::endl;
}