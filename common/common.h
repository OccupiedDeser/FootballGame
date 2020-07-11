/**************************************************************** 
    > Author: KovZhu
    > e-Mail: kovzhu@qq.com
    > Date: 2020-07-08 Wed 15:57:15
    > LastEditors: KovZhu
    > LastEditTime: 2020-07-11 Sat 10:11:51
 ****************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

char *get_conf_value(const char *path, const char *key);
int socket_create(int port);
void make_block(int fd);
void make_non_block(int fd);
#ifdef _D
#define DBG(fmt, args...) printf(fmt, ##args)
#else 
#define DBG(fmt, args...) 
#endif
#endif