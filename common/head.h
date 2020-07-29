/**************************************************************** 
    > Author: Deser
    > e-Mail: yongk202@163.com
    > Date: 2020-07-26 Sun 21:15:09
    > LastEditors: Deser
    > LastEditTime: 2020-07-29 Wed 07:07:51
 ****************************************************************/

#ifndef _HEAD_H
#define _HEAD_H
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>		
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include "color.h"
#include "store_score.h"
#include "common.h"
#include "datatype.h"
#include "udp_server.h"
#include "udp_client.h"
// #include "thread_pool.h"
// #include "sub_reactor.h"
#include "udp_epoll.h"
#include "ws_epoll.h"
#include "ws_server.h"
#include "WebSocket.h"
#include "ws_sub_reactor.h"
#include "ws_thread_pool.h"

#endif
