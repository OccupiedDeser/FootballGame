/**************************************************************** 
    > Author: Deser
    > e-Mail: yongk202@163.com
    > Date: 2020-07-20 Mon 19:32:42
    > LastEditors: Deser
    > LastEditTime: 2020-07-29 Wed 07:11:16
 ****************************************************************/
#include "head.h"
#define PORT 8888

char* conf = "/home/admini/Desktop/FootballGame/WebServer/footballd.conf";
struct Map court;
int repollfd, bepollfd;
struct User *rteam, *bteam;
int port = PORT;
pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bmutex = PTHREAD_MUTEX_INITIALIZER;

void logout(int signum)
{
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    // send_all(&msg);
    DBG(RED "\nBye Bye!\n");
    for (int i = 0; i < MAX; i++) {
        if (bteam[i].online) {
            close(bteam[i].fd);
        }
        if (rteam[i].online) {
            close(rteam[i].fd);
        }
    }
    exit(0);
}

// int main(int argc, char **argv)
int main()
{
    int opt, listener, epollfd;
    pthread_t red_t, blue_t;
    // while ((opt = getopt(argc, argv, "p:")) != -1)
    // {
    //     switch (opt)
    //     {
    //     case 'p':
    //         port = atoi(optarg);
    //         break;
    //     default:
    //         fprintf(stderr, "Usage : %s -p port\n", argv[0]);
    //         exit(1);
    //     }
    // } //判断一配置文件合法性

    // if (!port)
    //     port = atoi(get_conf_value(conf, "PORT"));
    // court.width = atoi(get_conf_value(conf, "COLS"));
    // court.height = atoi(get_conf_value(conf, "LINES"));

    if ((listener = socket_create_ws(port)) < 0) {
        perror("socket_create_ws()");
        exit(1);
    }

    if (listen(listener, MAX * 2) < 0) {
        perror("listen():");
        exit(1);
    }
    printf("socket_create_ws = %d", listener);

    DBG(GREEN "INFO" NONE " : Server start On port %d.\n", port);

    rteam = (struct User*)calloc(MAX, sizeof(struct User));
    bteam = (struct User*)calloc(MAX, sizeof(struct User));

    epollfd = epoll_create(MAX * 2);
    repollfd = epoll_create(MAX);
    bepollfd = epoll_create(MAX);

    if (epollfd < 0 || repollfd < 0 || bepollfd < 0) {
        perror("epoll_create()");
        exit(1);
    }

    struct task_queue redQueue;
    struct task_queue blueQueue;
    task_queue_init(&redQueue, MAX, repollfd);
    task_queue_init(&blueQueue, MAX, bepollfd);

    pthread_create(&red_t, NULL, ws_sub_reactor, (void*)&redQueue);
    pthread_create(&blue_t, NULL, ws_sub_reactor, (void*)&blueQueue);

    struct epoll_event ev, events[MAX * 2];
    ev.events = EPOLLIN;
    ev.data.fd = listener;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listener, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    socklen_t len = sizeof(client);

    signal(SIGINT, logout);

    while (1) {
        DBG(YELLOW "Main Reactor" NONE " : Waiting for client.\n" NONE);
        int nfds = epoll_wait(epollfd, events, MAX * 2, -1);

        if (nfds < 0) {
            perror("epoll_wait()");
            exit(1);
        }
        for (int i = 0; i < nfds; i++) {
            struct User user;
            char buff[512] = { 0 };
            if (events[i].data.fd == listener) {
                int new_fd = ws_accept(listener, &user);
                struct ChatMsg login_msg;
                bzero(&login_msg, sizeof(login_msg));
                login_msg.type = CHAT_SYS;
                sprintf(login_msg.msg, "你的好友 %s 已上线，快打个招呼吧！\n", user.name);
                ws_send_all(login_msg.msg, strlen(login_msg.msg));
                if (new_fd > 0) {
                    add_to_sub_reactor(&user);
                }
            }
        }
    }

    return 0;
}
