#include "ChatServer.h"
#include <iostream>
#include <WinSock2.h>

ChatServer::ChatServer(int port)
	: port_(port)
	, running_(false)
	, hIOCP_(INVALID_HANDLE_VALUE) {

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		std::cerr << "[!] WSAStartup failed: " << WSAGetLastError() << std::endl;
		throw std::runtime_error("WSAStartup failed");
	}

	hServerListenSocket_ = network::SocketObject(WSASocket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP,
		nullptr,
		0,
		WSA_FLAG_OVERLAPPED
	));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(static_cast<u_short>(port_));

	if (bind(hServerListenSocket_.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "[!] Bind failed: " << WSAGetLastError() << std::endl;
		return;
	}

	if (listen(hServerListenSocket_.get(), SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "[!] Listen failed: " << WSAGetLastError() << std::endl;
		return;
	}

	hIOCP_ = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		nullptr,
		0,
		0
	);

	CreateIoCompletionPort(
		reinterpret_cast<HANDLE>(hServerListenSocket_.get()),
		hIOCP_,
		0,
		0
	);
}

ChatServer::~ChatServer() {

	running_ = false;

	if (hIOCP_) {
		CloseHandle(hIOCP_);
	}

	for (auto& t : workerThreads_) {
		if (t.joinable()) {
			t.join();
		}
	}
	WSACleanup();
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

void ChatServer::AcceptClientLoop() {
	while (running_) {
		SOCKET clientSocket = accept(hServerListenSocket_.get(), nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "[!] Accept failed: " << WSAGetLastError() << std::endl;
			continue;
		}

		auto clientSession = std::make_shared<ClientSession>(clientSocket);
		clients_.push_back(clientSession);
		CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(clientSocket),
			hIOCP_,
			reinterpret_cast<ULONG_PTR>(clientSession.get()),
			0
		);
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
			hIOCP_,
			&bytesTransferred,
			&completionKey,
			&overlapped,
			INFINITE
		);

		auto session = reinterpret_cast<ClientSession*>(completionKey);
		auto context = reinterpret_cast<network::OverlappedEx*>(overlapped);

		if (!session || !context) {
			std::cerr << "[!] Invalid session or context" << std::endl;
			continue;
		}

		switch (context->ioType) {
		case network::IO_TYPE::RECV:
			session->OnRecvCompleted(bytesTransferred);
			break;
		case network::IO_TYPE::SEND:
			session->OnSendCompleted(bytesTransferred);
			break;
		}
	}
}