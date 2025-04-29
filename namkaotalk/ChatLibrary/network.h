#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <utility>
#include <stdexcept>
#include <memory>

namespace network {

class WinSocket {

public:
	explicit WinSocket(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags) {
		socket_ = WSASocket(af, type, protocol, lpProtocolInfo, g, dwFlags);
		if (socket_ == INVALID_SOCKET) {
			throw std::runtime_error("Failed to create socket");
		}
	};
	~WinSocket() noexcept {
		if (socket_ != INVALID_SOCKET)
			closesocket(socket_);
	};

	WinSocket(const WinSocket&) = delete;
	WinSocket& operator=(const WinSocket&) = delete;
	WinSocket(WinSocket&& rhs) noexcept 
		: socket_(std::exchange(rhs.socket_, INVALID_SOCKET)) {}
	WinSocket& operator=(WinSocket&& rhs) noexcept {
		if (this != &rhs) {
			if (socket_ != INVALID_SOCKET)
				closesocket(socket_);
			socket_ = std::exchange(rhs.socket_, INVALID_SOCKET);
		}
		return *this;
	}

	SOCKET get() const noexcept { return socket_; }

private:
	SOCKET socket_ = INVALID_SOCKET;
};

class IocpSocket {
public:
	explicit IocpSocket(WinSocket socket) : socket_(std::move(socket)) {}
	~IocpSocket() noexcept = default;

	IocpSocket(const IocpSocket&) = delete;
	IocpSocket& operator=(const IocpSocket&) = delete;
	IocpSocket(IocpSocket&& rhs) noexcept = default;
	IocpSocket& operator=(IocpSocket&& rhs) noexcept = default;

	SOCKET get() const noexcept { return socket_.get(); }

	void BindToIocp(HANDLE hIocp, ULONG_PTR key);
	bool PostSend(WSABUF* buf, OVERLAPPED* overlapped);
	bool PostRecv(WSABUF* buf, OVERLAPPED* overlapped);

private:
	WinSocket socket_;
};

} // namespace network