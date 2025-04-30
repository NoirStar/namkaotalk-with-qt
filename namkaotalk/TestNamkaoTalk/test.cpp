#include "pch.h"
#include "../ChatLibrary/network.h"
#include "../ChatLibrary/thread.h"
#include <chrono>
#include <iostream>

// Testcase for ChatLibrary
TEST(ChatLibraryTest, IocpSocket_CreateSocket) {

	network::WsaInitializer wsaInitializer;

	try {
		network::IocpSocket iocpSocket{
			network::WinSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 
				nullptr, 0, WSA_FLAG_OVERLAPPED)
		};
		EXPECT_NE(iocpSocket.get(), INVALID_SOCKET);

	} catch (const std::exception& e) {
		FAIL() << "Exception : " << e.what();
	}

}

// Testcase for ThreadPool
TEST(ThreadPoolTest, ThreadPool_ExcutionTest) {
	utils::ThreadPool tp{};

	std::future<int> result = tp.Enqueue([] () -> auto { return 10; });

	EXPECT_EQ(result.get(), 10);
}

TEST(ThreadPoolTest, ThreadPool_ParallelExcutionTest) {
	utils::ThreadPool tp{};

	std::atomic_int counter = 0;
	constexpr int taskCount = 100;

	std::vector<std::future<void>> futures;

	// add tasks and futures
	for (int i = 0; i < taskCount; ++i) {
		futures.emplace_back(
			tp.Enqueue([&counter] {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				++counter;
			})
		);
	}

	// get futures result
	for (auto& f : futures)
		f.get();

	EXPECT_EQ(counter.load(), taskCount);

}