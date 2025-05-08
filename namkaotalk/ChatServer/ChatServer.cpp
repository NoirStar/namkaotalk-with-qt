#include "ChatServer.h"
#include <iostream>
#include <WinSock2.h>

ChatServer::ChatServer(int port)
	: port_(port)
	, running_(false)
	, wsaInitializer_() {

	hServerListenSocket_ = network::ScopedSocket(WSASocket(
		AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED
	));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(static_cast<u_short>(port_));

	if (bind(hServerListenSocket_.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
		throw std::runtime_error("Bind failed");
	}

	if (listen(hServerListenSocket_.get(), SOMAXCONN) == SOCKET_ERROR) {
		throw std::runtime_error("Listen failed");
	}

	// IOCP 포트 생성(Queue)
	hIOCP_ = network::ScopedIocpHandle(
		CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0));

	// IOCP 포트에 소켓을 등록
	CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(hServerListenSocket_.get()), hIOCP_.get(), 0, 0);
}

ChatServer::~ChatServer() {

	running_ = false;

	for (auto& t : workerThreads_) {
		if (t.joinable()) {
			t.join();
		}
	}
}

void ChatServer::Run() {
	running_ = true;

	std::thread(&ChatServer::AcceptClientLoop, this).detach();

	// Start worker threads
	for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
		workerThreads_.emplace_back(&ChatServer::WorkerThreadLoop, this);
	}

	std::cout << "[+] Chat server started on port " << port_ << std::endl;
}

void ChatServer::Broadcast(const char* msg, size_t len, SOCKET senderSocket) {

	for (auto& [sock, session] : clients_) {
		if (sock != senderSocket) {
			session->SendPacket(msg, len);
		}
	}
}

void ChatServer::AcceptClientLoop() {

	while (running_) {
		SOCKET clientSocket = accept(hServerListenSocket_.get(), nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "[!] Accept failed: " << WSAGetLastError() << std::endl;
			continue;
		}

		auto clientSession = std::make_shared<ClientSession>(clientSocket);
		clients_[clientSocket] = clientSession;

		// IOCP 포트에 클라이언트 소켓 등록
		CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(clientSocket),
			hIOCP_.get(),
			reinterpret_cast<ULONG_PTR>(clientSession.get()),
			0
		);

		// 비동기 수신 시작
		clientSession->RecvPacket();
	}
}

void ChatServer::WorkerThreadLoop() {
	while (running_) {
		DWORD bytesTransferred;
		ULONG_PTR completionKey;
		LPOVERLAPPED overlapped;

		// 큐에서 하나씩 꺼내옴, blocking
		BOOL result = GetQueuedCompletionStatus(
			hIOCP_.get(),
			&bytesTransferred,
			&completionKey,
			&overlapped,
			INFINITE
		);

		auto session = reinterpret_cast<ClientSession*>(completionKey);
		auto ctx = reinterpret_cast<network::IoContext*>(overlapped);

		if (!session || !ctx) {
			std::cerr << "[!] Invalid session or context" << std::endl;
			continue;
		}

		switch (ctx->type) {
		case network::IO_TYPE::RECV:
			Broadcast(ctx->buffer.data(), bytesTransferred, session->GetSocket());
			session->OnRecvCompleted(bytesTransferred);
			break;
		case network::IO_TYPE::SEND:
			session->OnSendCompleted(bytesTransferred);
			break;
		}
	}
}