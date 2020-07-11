
/**************************************************************** 
    > Author: KovZhu
    > e-Mail: kovzhu@qq.com
    > Date: 2020-07-08 Wed 15:14:28
    > LastEditors: KovZhu
    > LastEditTime: 2020-07-11 Sat 13:00:45
 ****************************************************************/

#include "head.h"

int server_port = 0;
char server_ip[20] = { 0 };
int team = -1;
char name[20] = { 0 };
char log_msg[512] = { 0 };
char* conf = "./football.conf";
int sockfd = -1;

void logout(int signum)
{
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void*)&msg, sizeof(msg), 0);
    close(sockfd);
    DBG(L_GREEN"\nBye Bye!"NONE);
    exit(0);
}

void* do_recv(void* arg)
{
    printf("Receiving......\n");
    while (1) {
        struct ChatMsg msg;
        bzero(&msg, sizeof(msg));
        int ret = recv(sockfd, (void*)&msg, sizeof(msg), 0);
        if (ret != sizeof(msg)) {
            continue;
        }

        if (msg.type & CHAT_WALL) {
            printf(BLUE "%s" NONE " : %s\n", msg.name, msg.msg);
        } else if (msg.type & CHAT_MSG) {
            printf(RED "%s" NONE "单独向你发送了一条消息 : %s\n", msg.name, msg.msg);
        } else if (msg.type & CHAT_SYS) {
            printf(YELLOW "Server Info" NONE " : %s\n", msg.msg);
        } else if (msg.type & CHAT_FIN) {
            printf(L_RED "Server Info" NONE "Server Down!\n");
            exit(1);
        }
    }
}

int main(int argc, char** argv)
{
    int opt;
    struct LogRequest request; //HERE
    struct LogResponse response; //HERE
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {
        switch (opt) {
        case 't':
            request.team = atoi(optarg);
            break;
        case 'h':
            strcpy(server_ip, optarg);
            break;
        case 'p':
            server_port = atoi(optarg);
            break;
        case 'm':
            strcpy(request.msg, optarg);
            break;
        case 'n':
            strcpy(request.name, optarg);
            break;
        default:
            fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);
            exit(1);
        }
    }

    if (!server_port)
        server_port = atoi(get_conf_value(conf, "SERVERPORT"));
    if (!request.team)
        request.team = atoi(get_conf_value(conf, "TEAM"));
    if (!strlen(server_ip))
        strcpy(server_ip, get_conf_value(conf, "SERVERIP"));
    if (!strlen(request.name))
        strcpy(request.name, get_conf_value(conf, "NAME"));
    if (!strlen(request.msg))
        strcpy(request.msg, get_conf_value(conf, "LOGMSG"));

    DBG("<" GREEN "Conf Show" NONE "> : server_ip = %s, port = %d, team = %s, name = %s\n%s",
        server_ip, server_port, request.team ? "BLUE" : "RED", request.name, request.msg);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    socklen_t len = sizeof(server);

    if ((sockfd = socket_udp()) < 0) {
        perror("socket_udp()");
        exit(1);
    }

    sendto(sockfd, (void*)&request, sizeof(request), 0, (struct sockaddr*)&server, len); //HERE
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    DBG("<" PINK "Add rfds" NONE "> : set %d in rfds.\n", sockfd);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int ret_val;
    if ((ret_val = select(sockfd + 1, &rfds, NULL, NULL, &tv)) < 0) {
        perror("select");
        exit(1);
    } else if (ret_val == 0) {
        DBG("<" BLUE "Select Return" NONE "> : select failed.\n");
        exit(0);
    } else {
        ret_val = recvfrom(sockfd, (void*)&response, sizeof(response), 0, (struct sockaddr*)&server, &len);
        DBG("<" BLUE "Select Return" NONE "> : select return success %d fds.\n", ret_val);
    }
    printf("ers.msg : %s\n", response.msg);

    if (ret_val != sizeof(response) || response.type == 1) {
        printf("Connection refused.\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr)) < 0) {
        return -1;
    }

    pthread_t recv_t;
    pthread_create(&recv_t, NULL, do_recv, NULL);

    signal(SIGINT, logout);
    struct ChatMsg msg;
    while (1) {
        bzero(&msg, sizeof(msg));
        msg.type = CHAT_WALL;
        strcpy(msg.name, name);
        //printf(RED "Please Input:\n" NONE);
        scanf("%[^\n]s", msg.msg);
        getchar();
        if (strlen(msg.msg)) {
            if (msg.msg[0] == '@') {
                msg.type = CHAT_MSG;
            }
            if (msg.msg[0] == '#') {
                msg.type = CHAT_FUNC;
            }
            send(sockfd, (void*)&msg, sizeof(msg), 0);
        }
    }

    return 0;
}
