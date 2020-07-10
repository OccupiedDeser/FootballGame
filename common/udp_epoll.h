/**************************************************************** 
    > Author: Zjh
    > e-Mail: 1379664023@qq.com
    > Date: 2020-07-10 Fri 15:23:38
    > LastEditors: Zjh
    > LastEditTime: 2020-07-10 Fri 15:23:57
 ****************************************************************/

#ifndef _UDP_EPOLL_H
#define _UDP_EPOLL_H

int udp_accept(int fd, struct User* user);
void add_to_sub_reactor(struct User* user);
void del_event(int epollfd, int fd);

#endif
