/**************************************************************** 
    > Author: Zjh
    > e-Mail: 1379664023@qq.com
    > Date: 2020-07-10 Fri 23:44:56
    > LastEditors: Zjh
    > LastEditTime: 2020-07-10 Fri 23:45:06
 ****************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
struct task_queue {
    int sum;
    int epollfd; //从反应堆
    struct User **team;
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd);
void task_queue_push(struct task_queue *taskQueue, struct User *user);
struct User *task_queue_pop(struct task_queue *taskQueue);
void *thread_run(void *arg);
void send_all(struct ChatMsg* msg);
#endif
