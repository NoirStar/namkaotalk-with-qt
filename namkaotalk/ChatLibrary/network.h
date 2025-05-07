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
	COUNT
};

struct OverlappedEx {
	WSAOVERLAPPED overlapped{};
	IO_TYPE ioType;
	WSABUF wsabuf;
	char buffer[BUFFER_SIZE]{};

	explicit OverlappedEx(IO_TYPE t) : ioType(t) {
		wsabuf.buf = buffer;
		wsabuf.len = sizeof(buffer);
	}
};

class SocketObject {
public:
	SocketObject() : socket_(INVALID_SOCKET) {}
	SocketObject(SOCKET socket) : socket_(socket) {
		if (socket_ == INVALID_SOCKET) {
			throw std::runtime_error("Invalid socket");
		}
	}

	~SocketObject() {
		if (socket_ != INVALID_SOCKET) {
			closesocket(socket_);
		}
	}

	SocketObject(SocketObject&& other) noexcept
		: socket_(std::exchange(other.socket_, INVALID_SOCKET)) {}
	SocketObject& operator=(SocketObject&& other) noexcept {
		if (this != &other) {
			if (socket_ != INVALID_SOCKET) {
				closesocket(socket_);
			}
			socket_ = std::exchange(other.socket_, INVALID_SOCKET);
		}
		return *this;
	}

	SocketObject(const SocketObject&) = delete;
	SocketObject& operator=(const SocketObject&) = delete;

	SOCKET get() const {
		return socket_;
	}

	bool is_valid() const {
		return socket_ != INVALID_SOCKET;
	}

private:
	SOCKET socket_;
};

} // namespace network