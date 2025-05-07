#include "ChatClient.h"
#include <iostream>
#include <thread>

int main() {
	try {
		ChatClient client(L"127.0.0.1", network::PORT);
		client.Connect();

		std::thread recvThread(&ChatClient::RecvLoop, &client);

		std::string message;
		while (true) {
			std::getline(std::cin, message);
			if (message == "exit") break;

			client.SendPacket(message);
		}

		recvThread.join();

	} catch (const std::exception& e) {
		std::cerr << "[!] Exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}