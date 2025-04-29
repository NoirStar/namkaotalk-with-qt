#include "pch.h"
#include "../ChatLibrary/network.h"

// Testcase for ChatLibrary
TEST(ChatLibraryTest, Network_Connect) {
  network::TcpSocket socket;
  
  EXPECT_TRUE(socket.connect("localhost", 8080));
}

TEST(ChatLibraryTest, Network_Disconnect) {
  network::TcpSocket socket;

  socket.connect("localhost", 8080);
  socket.disconnect();

  // Assuming disconnect doesn't return a value
  EXPECT_NO_THROW(socket.disconnect());
}

TEST(ChatLibraryTest, Network_Send) {
  network::TcpSocket socket;
  socket.connect("localhost", 8080);
  const char* message = "Hello, World!";

  EXPECT_TRUE(socket.send(message, strlen(message)));
}

TEST(ChatLibraryTest, Network_Receive) {
  network::TcpSocket socket;
  socket.connect("localhost", 8080);
  char buffer[1024];
  EXPECT_TRUE(socket.receive(buffer, sizeof(buffer)));
}

TEST(ChatLibraryTest, Network_Send_Receive) {
  network::TcpSocket socket;
  socket.connect("localhost", 8080);
  const char* message = "Hello, World!";
  char buffer[1024];
  EXPECT_TRUE(socket.send(message, strlen(message)));
  EXPECT_TRUE(socket.receive(buffer, sizeof(buffer)));
}
