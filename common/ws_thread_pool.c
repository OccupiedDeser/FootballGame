/*************************************************************************
	> File Name: thread_pool.c
	> Author: zhangjiahe
	> Mail: 
	> Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
extern int repollfd, bepollfd;
extern struct User* rteam;
extern struct User* bteam;
extern pthread_mutex_t bmutex, rmutex;

void ws_send_all(char* data, unsigned long long datalen)
{
    char pack[MAX_MSG] = { 0 };
    unsigned long long packlen = 0;
    packlen = PackData(data, datalen, pack);
    if (packlen <= 0) {
    }
    for (int i = 0; i < MAX; i++) {
        if (bteam[i].online) {
            send(bteam[i].fd, (void*)pack, packlen, 0);
        }
        if (rteam[i].online) {
            send(rteam[i].fd, (void*)pack, packlen, 0);
        }
    }
}

int ws_send_to(char* to, struct ChatMsg* msg, int fd)
{
    for (int i = 0; i < MAX; i++) {
        if (rteam[i].online && (!strcmp(to, rteam[i].name))) {
            send(rteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            return 0;
        }
        if (bteam[i].online && (!strcmp(to, bteam[i].name))) {
            send(bteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            return 0;
        }
    }
    return -1;
}

void ws_do_work(struct User* user)
{
    // struct ChatMsg msg, re_msg;
    char buf[MAX_MSG * 2] = { 0 };
    char data[MAX_MSG] = { 0 };
    char pack[MAX_MSG] = { 0 };
    unsigned long long datalen, packlen;
    recv(user->fd, pack, MAX_MSG, 0);
    datalen = ParsePack(pack, data);
    if (datalen <= 0) {
        sprintf(buf, "注意：我们的好朋友 %s 已下线！\n", user->name);
        ws_send_all(buf, strlen(buf));
        if (user->team) {
            pthread_mutex_lock(&bmutex);
        } else {
            pthread_mutex_lock(&rmutex);
        }
        user->online = 0;
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user->fd);
        if (user->team) {
            pthread_mutex_unlock(&bmutex);
        } else {
            pthread_mutex_unlock(&rmutex);
        }
        printf(GREEN "Server Info" NONE " : %s logout!\n", user->name);
        close(user->fd);
    } else {
        sprintf(buf, "%s: %s", user->name, data);
        ws_send_all(buf, strlen(buf));
        printf("<%s> ~ %s\n", user->name, data);
    }

    // if (msg.type & CHAT_WALL) {
    //     msg.type = CHAT_WALL;
    //     strcpy(msg.name, user->name);
    //     printf("<%s> ~ %s\n", user->name, msg.msg);
    //     send_all(&msg);
    // } else if (msg.type & CHAT_MSG) {
    //     char to[20] = { 0 };
    //     int i = 1;
    //     for (; i <= 21; i++) {
    //         if (msg.msg[i] == ' ') {
    //             break;
    //         }
    //     }
    //     if (msg.msg[i] != ' ' || msg.msg[0] != '@') {
    //         memset(&re_msg, 0, sizeof(re_msg));
    //         re_msg.type = CHAT_SYS;
    //         strcpy(re_msg.msg, "私聊格式错误！\n");
    //         send(user->fd, (void*)&re_msg, sizeof(re_msg), 0);
    //     } else {
    //         printf("<%s> $ %s\n", user->name, msg.msg);
    //         msg.type = CHAT_MSG;
    //         strcpy(msg.name, user->name);
    //         strncpy(to, msg.msg + 1, i - 1);
    //         if (send_to(to, &msg, user->fd) < 0) {
    //             msg.type = CHAT_SYS;
    //             bzero(msg.msg, sizeof(msg.msg));
    //             strcpy(msg.msg, "用户不存在或已下线！\n");
    //             send(user->fd, &msg, sizeof(msg), 0);
    //         }
    //     }
    // } else if (msg.type & CHAT_FIN) {
    //     bzero(msg.msg, sizeof(msg.msg));
    //     msg.type = CHAT_SYS;
    //     sprintf(msg.msg, "注意：我们的好朋友 %s 已下线！\n", user->name);
    //     strcpy(msg.name, user->name);
    //     send_all(&msg);
    //     if (user->team) {
    //         pthread_mutex_lock(&bmutex);
    //     } else {
    //         pthread_mutex_lock(&rmutex);
    //     }
    //     user->online = 0;
    //     int epollfd = user->team ? bepollfd : repollfd;
    //     del_event(epollfd, user->fd);
    //     if (user->team) {
    //         pthread_mutex_unlock(&bmutex);
    //     } else {
    //         pthread_mutex_unlock(&rmutex);
    //     }
    //     printf(GREEN "Server Info" NONE " : %s logout!\n", user->name);
    //     close(user->fd);
    // } else if (msg.type & CHAT_FUNC) {
    //     bzero(&msg, sizeof(msg));
    //     msg.type = CHAT_SYS;
    //     strcat(msg.msg, "\n");
    //     for (int i = 0; i < MAX; i++) {
    //         if (rteam[i].online) {
    //             strcat(msg.msg, rteam[i].name);
    //             strcat(msg.msg, "\n");
    //         }
    //         if (bteam[i].online) {
    //             strcat(msg.msg, bteam[i].name);
    //             strcat(msg.msg, "\n");
    //         }
    //     }
    //     send(user->fd, (void*)&msg, sizeof(msg), 0);
    // }
}

void task_queue_init(struct task_queue* taskQueue, int sum, int epollfd)
{
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void*));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue* taskQueue, struct User* user)
{
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN "Thread Pool" NONE " : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN "Thread Pool" NONE " : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}

struct User* task_queue_pop(struct task_queue* taskQueue)
{
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN "Thread Pool" NONE " : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User* user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN "Thread Pool" NONE " : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN "Thread Pool" NONE " : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void* ws_thread_run(void* arg)
{
    pthread_detach(pthread_self());
    struct task_queue* taskQueue = (struct task_queue*)arg;
    while (1) {
        struct User* user = task_queue_pop(taskQueue);
        ws_do_work(user);
    }
}