#include "network.h"

namespace network {

void IocpSocket::BindToIocp(HANDLE hIocp, ULONG_PTR key) {
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_.get()), hIocp, key, 0) == nullptr) {
		throw std::runtime_error("Failed to bind socket to IOCP");
	}
}

bool IocpSocket::PostSend(WSABUF* buf, OVERLAPPED* overlapped) {
	DWORD bytesSent = 0;
	if (WSASend(socket_.get(), buf, 1, &bytesSent, 0, overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}
	return true;
}

bool IocpSocket::PostRecv(WSABUF* buf, OVERLAPPED* overlapped) {
	DWORD bytesReceived = 0;
	if (WSARecv(socket_.get(), buf, 1, &bytesReceived, 0, overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}
	return true;
}

} // namespace network