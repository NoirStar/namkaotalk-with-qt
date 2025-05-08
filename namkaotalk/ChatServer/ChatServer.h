#pragma once

#include <WinSock2.h>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <shared_mutex>
#include "ClientSession.h"

class ChatServer {
public:
	explicit ChatServer(int port);
	~ChatServer();
	void Run();
	void Broadcast(const char* msg, size_t len, SOCKET senderSocket);

private:
	void AcceptClientLoop();
	void WorkerThreadLoop();

	network::ScopedIocpHandle hIOCP_;
	network::ScopedSocket hServerListenSocket_;
	network::WinSockInitializer wsaInitializer_;

	int port_;
	std::unordered_map<SOCKET, std::shared_ptr<ClientSession>> clients_;
	std::shared_mutex clientMutex_;
	std::vector<std::thread> workerThreads_;
	std::atomic<bool> running_;
};