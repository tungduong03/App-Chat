#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <map>
using namespace std;

#pragma comment(lib, "ws2_32.lib")
struct info_clients {
    int client;
    char nick_name[128];
    char channel[128];
};
info_clients clients[1000]; // Danh sách các client đang kết nối
int num = 0, number_channel = 0;
struct info_channels {
    char channel[128];
    int soLuong;
};
info_channels channels[50]; // Danh sách các channels đang có
mutex mtx;


//Hàm check nick name đã tồn tại chưa
int checkNickName(char* nick) {
    for (info_clients client : clients) {
        if (!strcmp(client.nick_name, nick)) return 0;
    }
    return 1;
}

//Hàm check channel đã tồn tại chưa
int checkChannel(char* channel) {
    for (info_clients client : clients) {
        if (!strcmp(client.channel, channel)) return 0;
    }
    return 1;
}


// Hàm gửi tin nhắn tới tất cả các client
void broadcastMessage(const char* message, int sender, const char* channel) {
    for (info_clients client : clients) {
        if (strcmp(client.nick_name, clients[sender].nick_name) && !strcmp(client.channel, channel)) { // Không gửi lại tin nhắn cho người gửi
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            strncpy_s(msg, clients[sender].nick_name, strlen(clients[sender].nick_name));
            strcat_s(msg, " : ");
            strcat_s(msg, message);
            send(client.client, msg, strlen(msg), 0);
        }
    }
}

// Hàm gửi tin nhắn tới tất cả các client
void alertMessage(const char* message, int sender, const char* channel) {
    for (info_clients client : clients) {
        if (strcmp(client.nick_name, clients[sender].nick_name) && !strcmp(client.channel, channel)) { // Không gửi lại tin nhắn cho người gửi
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            strcat_s(msg, message);
            send(client.client, msg, strlen(msg), 0);
        }
    }
}

// Hàm xử lý mỗi kết nối
void clientHandler(int clientSocket) {
    // Thêm client mới vào danh sách
    mtx.lock();
    int number = num;
    clients[number].client = clientSocket;
    num++;
    mtx.unlock();
    char buf[256];
    memset(buf, 97, sizeof(buf));

    while (1) {
        int flag = 0;
        int ret = recv(clientSocket, buf, sizeof(buf), 0);
        if (ret <= 0) {
            printf("Ket noi %d bi huy\n", clientSocket);
            break;
        }

        if (ret < sizeof(buf))
            buf[ret] = 0;


        char mess[128];
        strncpy_s(mess, buf, strlen(buf));
        // Chuyển đổi mảng ký tự sang chữ hoa
        for (int i = 0; mess[i] != '\0'; ++i) {
            mess[i] = toupper(mess[i]);
        }

        // Tìm kiếm chuỗi keyword trong mess
        char* nick = strstr(mess, "/NICK ");
        if (nick == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;
            if (checkNickName(result)) {
                //chưa tồn tại
                char msg[] = "Tao thanh cong! Ban dang o trong channel All.";
                char msg2[] = "De biet cac lenh thao tac go /help";
                strncpy_s(clients[number].nick_name, result, strlen(result));
                strncpy_s(clients[number].channel, "All", strlen("All")); 
                channels[0].soLuong++;
                send(clientSocket, msg, strlen(msg), 0);
                send(clientSocket, msg2, strlen(msg2), 0);

                char msg1[1024] = "";
                strcat_s(msg1, clients[number].nick_name); 
                strcat_s(msg1, " dang o channel All");
                alertMessage(msg1, number, clients[number].channel);
            }
            else {
                char msg[] = "username da ton tai, tao khong thanh cong!";
                send(clientSocket, msg, strlen(msg), 0);
            }
            flag = 1;
        }

        // Tìm kiếm chuỗi "/channel" trong mess
        char* channel = strstr(mess, "/CHANNEL ");
        if (channel == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;
            if (checkChannel(result)) {
                //chưa tồn tại
                char msg[1024] = "tao thanh cong, ban dang o trong channel ";

                mtx.lock();
                number_channel++;
                mtx.unlock();

                //tạo thành công nên chưa có trong list, thêm mới channel
                for (int p = 0; p <= number_channel; p++) {
                    if (!strcmp(channels[p].channel, clients[number].channel)) {
                        channels[p].soLuong--;
                    }
                }

                strncpy_s(channels[number_channel].channel, result, strlen(result));
                channels[number_channel].soLuong = 1;
                strcat_s(msg, result);
                strncpy_s(clients[number].channel, result, strlen(result));
                send(clientSocket, msg, strlen(msg), 0);
            }
            else {
                char msg[] = "channel da ton tai, tao khong thanh cong!";
                send(clientSocket, msg, strlen(msg), 0);
            }
            flag = 1;
        }

        // Tìm kiếm chuỗi "/join" trong mess
        char* join = strstr(mess, "/JOIN ");
        if (join == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            if (!strcmp(clients[number].channel, "")) {
                char* result = strstr(buf, " ");
                result++;
                if (checkChannel(result)) {
                    //chưa tồn tại
                    char msg[1024] = "khong co channel ban can join!";
                    send(clientSocket, msg, strlen(msg), 0);
                }
                else {
                    char msg[1024] = "";
                    strcat_s(msg, clients[number].nick_name);
                    strcat_s(msg, " dang o channel ");
                    for (int p = 0; p <= number_channel; p++) {
                        if (!strcmp(channels[p].channel, result)) {
                            channels[p].soLuong++;
                        }
                    }
                    strncpy_s(clients[number].channel, result, strlen(result));
                    strcat_s(msg, result);
                    send(clientSocket, msg, strlen(msg), 0);
                    alertMessage(msg, number, clients[number].channel); 
                }
            }
            else {
                char msg[1024] = "ban chua thoat khoi channel hien tai!";
                send(clientSocket, msg, strlen(msg), 0);
            }

            flag = 1;
        }

        // Tìm kiếm chuỗi "/quit" trong mess
        char* quit = strstr(mess, "/QUIT");
        if (quit == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            closesocket(clientSocket);
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            strncpy_s(msg, clients[number].nick_name, strlen(clients[number].nick_name));
            strcat_s(msg, " da thoat khoi phien dang nhap!");
            broadcastMessage(msg, number, clients[number].channel);
            printf("%s\n", msg);
            flag = 1;
            break;
        }

        // Tìm kiếm chuỗi "/part" trong mess
        char* part = strstr(mess, "/PART ");
        if (part == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;
            if (strcmp(clients[number].channel, result)) {
                //không ở trong channel
                char msg[1024] = "nguoi dung khong o trong channel!";
                send(clientSocket, msg, strlen(msg), 0);
            }
            else {
                char msg[1024] = "";
                strcat_s(msg, clients[number].nick_name);
                strcat_s(msg, " da thoat channel ");
                for (int p = 0; p <= number_channel; p++) {
                    if (!strcmp(channels[p].channel, result)) {
                        channels[p].soLuong--;
                    }
                }
                strcat_s(msg, result);
                send(clientSocket, msg, strlen(msg), 0);
                alertMessage(msg, number, clients[number].channel); 
                strncpy_s(clients[number].channel, "", strlen(""));
            }
            flag = 1;
        }

        // Tìm kiếm chuỗi "/list" trong mess
        char* list = strstr(mess, "/LIST");
        if (list == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            flag = 1;
            char msg[1024] = "Cac channel hien tai: ";
            send(clientSocket, msg, strlen(msg), 0);
            char chan[256];
            memset(chan, '\0', sizeof(chan));
            for (int i = 0; i <= number_channel; i++) {
                if (channels[i].soLuong > 0) {
                    strcat_s(chan, ", ");
                    strncat_s(chan, channels[i].channel, strlen(channels[i].channel));
                    strcat_s(chan, " : ");
                    string soLuongStr = to_string(channels[i].soLuong);
                    strcat_s(chan, soLuongStr.c_str());
                }
            }
            char* p = strstr(chan, ",");
            p += 2;
            send(clientSocket, p, strlen(p), 0);
        }


        // Tìm kiếm chuỗi "/PRIVMSG" trong mess
        char* who = strstr(mess, "/WHO ");
        if (who == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;

            char msg[1024] = "";
            int success = 0;
            for (int i = 0; i <= num; i++) {
                if (!strcmp(clients[i].nick_name, result)) { 
                    strcat_s(msg, clients[i].nick_name);
                    strcat_s(msg, ", channel: ");
                    strcat_s(msg, clients[i].channel);
                    send(clientSocket, msg, strlen(msg), 0); 
                    success = 1;
                }
            }
            if (success == 0) {
                char m[1024] = "Khong ton tai nguoi dung!";
                send(clientSocket, m, strlen(m), 0);
            }
            
            flag = 1;
        }

        // Tìm kiếm chuỗi "/PRIVMSG" trong mess
        char* privmsg = strstr(mess, "/PRIVMSG ");
        if (privmsg == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;

            char* nick;
            char* context = NULL;
            nick = strtok_s(result, " ", &context);

            int success = 0;
            for (int i = 0; i <= num; i++) {
                if (!strcmp(clients[i].nick_name, nick)) {
                    send(clients[i].client, context, strlen(context), 0);
                    success = 1;
                }
            }
            if (success == 0) {
                char m[1024] = "Gui khong thanh cong!";
                send(clientSocket, m, strlen(m), 0);
            }
            strcat_s(buf, " ");
            strcat_s(buf, context);
            flag = 1;
        }

        char* check = strstr(mess, "/MES");
        if (check == mess && flag == 0) {
            // Chuỗi keyword được tìm thấy trong buf
            char* result = strstr(buf, " ");
            result++;

            broadcastMessage(result, number, clients[number].channel);
            flag = 1;
        }

        //nếu ko phải lệnh thì gửi tin nhắn đến tất cả các client cùng channel
        if (flag == 0) {
            // Gửi tin nhắn nhận được từ client tới tất cả các client khác cùng channel
            broadcastMessage(buf, number, clients[number].channel);
        }

        //lưu lại log ở server
        printf("%s >> %s: %s\n", clients[number].nick_name, clients[number].channel, buf);
    }

    // Đóng kết nối với client khi kết thúc
    closesocket(clientSocket);
}

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
    strncpy_s(channels[0].channel, "All", strlen("All"));
    channels[0].soLuong = 0;
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }


    while (1) {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        // Chấp nhận kết nối từ client mới
        int client = accept(listener, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (client == -1) {
            perror("accept() failed");
            continue;
        }

        printf("Client moi ket noi: %d\n", client);

        // Tạo một luồng mới để xử lý kết nối của client
        thread clientThread(clientHandler, client);
        clientThread.detach(); // Tách luồng để không cần join

    }

    // Đóng socket lắng nghe
    closesocket(listener);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
