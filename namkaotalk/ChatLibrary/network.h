#pragma once

namespace network {

class TcpSocket {
public:
	TcpSocket() = default;
	~TcpSocket() noexcept = default;
	bool connect(const char* host, int port);
	void disconnect();
	bool send(const char* data, size_t size);
	bool receive(char* buffer, size_t size);

};

} // namespace network