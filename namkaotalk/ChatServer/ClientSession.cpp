#include "ClientSession.h"
#include <iostream>

ClientSession::ClientSession(SOCKET socket) noexcept
	: socket_(socket)
	, recvCtx_(std::make_unique<network::IoContext>(network::IO_TYPE::RECV))
	, sendCtx_(std::make_unique<network::IoContext>(network::IO_TYPE::SEND)) {

	std::cout << "[+] Client Connected : " << socket_.get() << std::endl;
}

ClientSession::~ClientSession() {

	std::cout << "[-] Client Disconnected : " << socket_.get() << std::endl;
}

void ClientSession::RecvPacket() {
	
	DWORD flags = 0, bytes = 0;

	// �񵿱� ���� �Լ� (Ŭ�󼼼��� ���� Ŭ���̾�Ʈ->���� �ޱ�)
	// �񵿱�� ������ �ɾ��ش�.
	int ret = WSARecv(
		socket_.get(),
		&recvCtx_->wsabuf,
		1,
		&bytes,
		&flags,
		reinterpret_cast<LPWSAOVERLAPPED>(recvCtx_.get()),
		nullptr
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[!] WSARecv failed: " << WSAGetLastError() << std::endl;
		return;
	}
}

void ClientSession::SendPacket(const char* data, size_t len) {

	if (len > sendCtx_->buffer.size()) {
		std::cerr << "[!] Data length exceeds buffer size" << std::endl;
		return;
	}

	sendCtx_->reset();

	std::memcpy(sendCtx_->buffer.data(), data, len);
	sendCtx_->wsabuf.buf = sendCtx_->buffer.data();
	sendCtx_->wsabuf.len = static_cast<ULONG>(len);

	DWORD bytes;

	// �񵿱� �۽� �Լ� (Ŭ�󼼼��� ���� ����->Ŭ���̾�Ʈ)
	// �񵿱�� �۽��� �ɾ��ش�.
	int ret = WSASend(
		socket_.get(),
		&sendCtx_->wsabuf,
		1,
		&bytes,
		0,
		reinterpret_cast<LPWSAOVERLAPPED>(sendCtx_.get()),
		nullptr
	);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[!] WSASend failed: " << WSAGetLastError() << std::endl;
		return;
	}
}

// GetQueuedCompletionStatus ���� ť���� �ϳ��� Context�� �������Եǰ�, �ű⼭ ȣ��Ǵ� �Լ�(�ݹ��Լ�)
void ClientSession::OnRecvCompleted(DWORD bytes) {
	if (bytes == 0) {
		std::cerr << "[!] Connection closed by client" << std::endl;
		Close();
		return;
	}

	std::cout << "[+] Received " << bytes << " bytes from client: "
		<< socket_.get() << std::endl;

	// Process received data
	std::cout << socket_.get() << ": " << std::string(recvCtx_->buffer.data(), bytes) << std::endl;


	// �ٽ� �񵿱� ������ �ɾ��ش�.
	// �� Ŭ���̾�Ʈ Session�� shared_ptr�� ���� �Ҹ���� �ʵ��� ��.
	auto self = shared_from_this();
	self->RecvPacket();
}

void ClientSession::OnSendCompleted(DWORD bytes) {
	if (bytes == 0) {
		std::cerr << "[!] Connection closed by client" << std::endl;
		Close();
		return;
	}

	//std::cout << "[+] Sent " << bytes << " bytes to client: "
	//	<< socket_.get() << std::endl;
}

void ClientSession::Close() {
	// INVALID_SOCKET�� �ٲ��ش�.
	socket_ = network::ScopedSocket();
}