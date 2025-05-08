#pragma once
#include <WinSock2.h>
#include <memory>
#include "../ChatLibrary/network.h"

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
	explicit ClientSession(SOCKET socket) noexcept;
	~ClientSession() noexcept;

	void RecvPacket();
	void SendPacket(const char* data, size_t len);
	void Echo(const char* data, size_t len);
	// IOCP 비동기를 걸어준다.
	void OnRecvCompleted(DWORD bytes);
	void OnSendCompleted(DWORD bytes);

	SOCKET GetSocket() const {
		return socket_.get();
	}

private:

	void Close();

	network::ScopedSocket socket_;
	std::unique_ptr<network::IoContext> recvCtx_;
	std::unique_ptr<network::IoContext> sendCtx_;
};