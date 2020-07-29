/**************************************************************** 
    > Author: Deser
    > e-Mail: yongk202@163.com
    > Date: 2020-07-24 Fri 12:16:25
    > LastEditors: Deser
    > LastEditTime: 2020-07-25 Sat 18:06:25
 ****************************************************************/
#ifndef _WEB_SOCKET_H
#define _WEB_SOCKET_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "base64.h"
#include "sha1.h"

#define BUFLENGTH 1025
#define GUIDLEN 50

bool ws_echo(int fd);
bool ShakeHand(int fd);
unsigned long long PackData(const char* data, unsigned long long datalen, unsigned char* Package);
unsigned long long ParsePack(char* Package, char* data);

#endif