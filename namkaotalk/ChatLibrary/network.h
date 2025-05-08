#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <utility>
#include <stdexcept>
#include <memory>
#include <array>

#pragma comment (lib, "ws2_32.lib")

namespace network {

constexpr int PORT = 8888;
constexpr int BUFFER_SIZE = 1024;

enum class IO_TYPE : int8_t {
	RECV,
	SEND,
	ACCEPT,
	COUNT,
};

struct IoContext {
	WSAOVERLAPPED overlapped{};
	IO_TYPE type = IO_TYPE::RECV;
	WSABUF wsabuf{};
	std::array<char, BUFFER_SIZE> buffer{};

	explicit IoContext(IO_TYPE t)
		: type(t) {
		reset();
		wsabuf.buf = reinterpret_cast<char*>(buffer.data());
		wsabuf.len = static_cast<ULONG>(buffer.size());
	}

	void reset() {
		std::memset(&overlapped, 0, sizeof(overlapped));
	}
};

class WinSockInitializer {
public:
	WinSockInitializer() {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			throw std::runtime_error("WSAStartup failed");
		}
	}

	~WinSockInitializer() noexcept {
		WSACleanup();
	}
};

class ScopedSocket {
public:
	ScopedSocket() : socket_(INVALID_SOCKET) {}
	ScopedSocket(SOCKET socket) : socket_(socket) {
		if (socket_ == INVALID_SOCKET) {
			throw std::runtime_error("Invalid socket");
		}
	}

	~ScopedSocket() {
		if (socket_ != INVALID_SOCKET) {
			closesocket(socket_);
		}
	}

	ScopedSocket(ScopedSocket&& other) noexcept
		: socket_(std::exchange(other.socket_, INVALID_SOCKET)) {}
	ScopedSocket& operator=(ScopedSocket&& other) noexcept {
		if (this != &other) {
			if (socket_ != INVALID_SOCKET) {
				closesocket(socket_);
			}
			socket_ = std::exchange(other.socket_, INVALID_SOCKET);
		}
		return *this;
	}

	ScopedSocket(const ScopedSocket&) = delete;
	ScopedSocket& operator=(const ScopedSocket&) = delete;

	SOCKET get() const {
		return socket_;
	}

	bool is_valid() const {
		return socket_ != INVALID_SOCKET;
	}

private:
	SOCKET socket_;
};

class ScopedIocpHandle {
public:
	ScopedIocpHandle() : handle_(INVALID_HANDLE_VALUE) {}
	explicit ScopedIocpHandle(HANDLE handle) : handle_(handle) {
		if (handle_ == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Invalid IOCP handle");
		}
	}
	~ScopedIocpHandle() {
		if (handle_) {
			CloseHandle(handle_);
		}
	}

	ScopedIocpHandle(ScopedIocpHandle&& other) noexcept
		: handle_(std::exchange(other.handle_, INVALID_HANDLE_VALUE)) {}
	ScopedIocpHandle& operator=(ScopedIocpHandle&& other) noexcept {
		if (this != &other) {
			if (handle_) {
				CloseHandle(handle_);
			}
			handle_ = std::exchange(other.handle_, INVALID_HANDLE_VALUE);
		}
		return *this;
	}

	ScopedIocpHandle(const ScopedIocpHandle&) = delete;
	ScopedIocpHandle& operator=(const ScopedIocpHandle&) = delete;

	HANDLE get() const {
		return handle_;
	}
	
private:
	HANDLE handle_;
};


} // namespace network