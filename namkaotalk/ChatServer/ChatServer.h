#pragma once

#include <WinSock2.h>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include "ClientSession.h"

class ChatServer {
public:
	explicit ChatServer(int port);
	~ChatServer();
	void Run();

private:
	void AcceptClientLoop();
	void WorkerThreadLoop();

	HANDLE hIOCP_;
	network::SocketObject hServerListenSocket_;
	int port_;
	std::vector<std::shared_ptr<ClientSession>> clients_;
	std::vector<std::thread> workerThreads_;
	std::atomic<bool> running_;
};