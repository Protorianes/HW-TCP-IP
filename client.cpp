#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>

const char* SERVER_IP = "192.168.1.123"; // замените на IP сервера
const int PORT = 12345;

void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        int valread = read(sock, buffer, sizeof(buffer));
        if (valread <= 0) {
            std::cout << "Соединение закрыто.\n";
            break;
        }
        std::cout << "Получено: " << std::string(buffer, valread) << "\n";
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed \n";
        return -1;
    }

    std::cout << "Подключено к серверу!\n";

    // Запускаем поток для получения сообщений
     std::thread receiver(receive_messages, sock);

     // Отправляем сообщения в основном потоке
     while (true) {
         std::string message;
         std::getline(std::cin, message);
         send(sock, message.c_str(), message.size(), 0);
     }

     receiver.join();
     close(sock);
     return 0;
}
