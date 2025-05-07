#include <iostream>

#include "ChatServer.h"
#include "../ChatLibrary/network.h"

int main() {

	try {
		ChatServer server(network::PORT);
		server.Run();

		std::cout << "[+] Chat server is running..." << std::endl;
		std::cout << "[+] Press Enter to exit..." << std::endl;
		std::cin.get();
		std::cout << "[+] Shutting down server..." << std::endl;

	} catch (const std::exception& e) {
		std::cerr << "[!] Exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}    