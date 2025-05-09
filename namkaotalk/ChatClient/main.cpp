#include <format>  // C++20 string formatting
#include <thread>
#include <iostream>
#include "ChatClient.h"

int main() {

	try {
		ChatClient client(L"127.0.0.1", network::PORT);
		client.Connect();

		std::thread recvThread(&ChatClient::RecvLoop, &client);

		std::string message;
		while (true) {
			std::cout << "> ";
			if (!std::getline(std::cin, message) || message == "exit")
				break;

			std::cout << std::format("[me]: {}\n", message);

			client.SendPacket(message);
		}

		recvThread.join();
	}
	catch (const std::exception& e) {
		std::cerr << std::format("[!] Exception: {}\n", e.what());
		return 1;
	}

	return 0;
}