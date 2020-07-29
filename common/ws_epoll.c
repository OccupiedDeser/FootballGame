/**************************************************************** 
    > Author: Deser
    > e-Mail: yongk202@163.com
    > Date: 2020-07-28 Tue 21:37:59
    > LastEditors: Deser
    > LastEditTime: 2020-07-29 Wed 07:06:51
 ****************************************************************/

#include "head.h"

extern int port;
extern struct User* rteam;
extern struct User* bteam;
extern int repollfd, bepollfd;
extern pthread_mutex_t bmutex, rmutex;

int check_online(char* name)
{
    //printf("in check_online, check:%s\n", request->name);
    for (int i = 0; i < MAX; i++) {
        //printf("%d:%d %s %d %s\n", i, rteam[i].online, rteam[i].name, bteam[i].online, bteam[i].name);
        if (rteam[i].online && !strcmp(name, rteam[i].name)) {
            return 1;
        }
        if (bteam[i].online && !strcmp(name, bteam[i].name)) {
            return 1;
        }
    }
    return 0;
}

int ws_accept(int fd, struct User* user)
{
    int new_fd, ret, namelen, resplen;
    struct sockaddr_in client;
    char request[MAX_MSG];
    char response[MAX_MSG];
    char name[MAX_MSG];
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    socklen_t client_len = sizeof(client);

    new_fd = accept(fd, (struct sockaddr*)&client, &client_len);
    if (new_fd < 0) {
        perror("accept()");
        exit(1);
    }
    if (!ShakeHand(new_fd)) {
        close(new_fd);
        return -1;
    }

    ret = recv(new_fd, (void*)request, MAX_MSG, 0);
    if (ret <= 0) {
        perror("recv()");
        close(new_fd);
        return -1;
    }

    unsigned char opcode = request[0] & 0xF;
    if (opcode == 0x8) {
        close(new_fd);
        return -1;
    }
    if (opcode != 0x1) { //暂不考虑其他控制帧
    close(new_fd);
        return -1;
    }
    namelen = ParsePack(request, name);
    if (!namelen) {
        printf("文本过长\n");
        close(new_fd);
        return -1;
    }

    if (check_online(name)) {
        resplen = PackData("U have logined in!", strlen("U have logined in!"), response);
        send(new_fd, response, resplen, 0);
        close(new_fd);
        return -1;
    }

    resplen = PackData("Login Success. Enjoy yourself!", strlen("Login Success. Enjoy yourself!"), response);
    send(new_fd, response, resplen, 0);
    
    DBG(GREEN "Info" NONE " : " RED "%s login on %s:%dn", name, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    strcpy(user->name, name);
    user->online = 1;
    user->team = 0;
    user->fd = new_fd;
    return new_fd;
}

void add_event_ptr(int epollfd, int fd, int events, struct User* user)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void*)user;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void del_event(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}
int find_sub(struct User* team)
{
    for (int i = 0; i < MAX; i++) {
        if (!team[i].online)
            return i;
    }
    return -1;
}

void add_to_sub_reactor(struct User* user)
{
    struct User* team = (user->team ? bteam : rteam);
    DBG(YELLOW "Main Thread : " NONE "Add to sub_reactor\n");
    if (user->team) {
        pthread_mutex_lock(&bmutex);
    } else {
        pthread_mutex_lock(&rmutex);
    }

    int sub = find_sub(team);
    if (sub < 0) {
        fprintf(stderr, "Full Team!\n");
        return;
    }
    team[sub] = *user;
    team[sub].online = 1;
    team[sub].flag = 10;
    if (user->team) {
        pthread_mutex_unlock(&bmutex);
    } else {
        pthread_mutex_unlock(&rmutex);
    }
    DBG(L_RED "sub = %d, name = %s\n" NONE, sub, team[sub].name);
    if (user->team) {
        add_event_ptr(bepollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
    } else {
        add_event_ptr(repollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
    }
}