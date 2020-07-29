/**************************************************************** 
    > Author: Zjh
    > e-Mail: 1379664023@qq.com
    > Date: 2020-07-10 Fri 15:23:38
    > LastEditors: Zjh
    > LastEditTime: 2020-07-10 Fri 15:23:57
 ****************************************************************/

#ifndef _WS_EPOLL_H
#define _WS_EPOLL_H

int ws_accept(int fd, struct User* user);
void add_to_sub_reactor(struct User* user);
void del_event(int epollfd, int fd);

#endif
