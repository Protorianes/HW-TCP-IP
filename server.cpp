#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 12345;

int client1_socket = -1;
int client2_socket = -1;
std::mutex mtx;

void handle_client(int client_socket, int other_socket) {
    char buffer[1024];
    while (true) {
        int valread = read(client_socket, buffer, sizeof(buffer));
        if (valread <= 0) {
            std::cout << "Клиент отключился.\n";
            close(client_socket);
            break;
        }
        // Передача сообщения другому клиенту
        std::lock_guard<std::mutex> lock(mtx);
        if (other_socket != -1) {
            send(other_socket, buffer, valread, 0);
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    // Создаем сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed\n";
        return -1;
    }

    // Настраиваем адрес
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Связываем сокет
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return -1;
    }

    // Начинаем слушать
    if (listen(server_fd, 2) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return -1;
    }

    std::cout << "Ожидание двух клиентов...\n";

    // Принимаем первого клиента
    client1_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addr_len);
    if (client1_socket < 0) {
        std::cerr << "Accept failed\n";
        close(server_fd);
        return -1;
    }
    std::cout << "Первый клиент подключен.\n";

    // Принимаем второго клиента
    client2_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addr_len);
    if (client2_socket < 0) {
        std::cerr << "Accept failed\n";
        close(client1_socket);
        close(server_fd);
        return -1;
    }
    std::cout << "Второй клиент подключен.\n";

    // Создаем потоки для обработки каждого клиента
    std::thread t1(handle_client, client1_socket, client2_socket);
    std::thread t2(handle_client, client2_socket, client1_socket);

    t1.join();
    t2.join();

    close(server_fd);
    return 0;
}
