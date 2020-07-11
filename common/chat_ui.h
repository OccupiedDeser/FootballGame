/**************************************************************** 
    > Author: KovZhu
    > e-Mail: kovzhu@qq.com
    > Date: 2020-07-11 Sat 10:19:21
    > LastEditors: KovZhu
    > LastEditTime: 2020-07-11 Sat 10:19:22
 ****************************************************************/
#ifndef _CHAT_UI_H
#define _CHAT_UI_H
#define MSG_WIDTH 100
#define INFO_WIDTH 15
#define MSG_HEIGHT 25
#define INPUT_HEIGHT 5
void destroy_win(WINDOW *win);
void gotoxy(int x, int y);
void gotoxy_putc(int x, int y, int c);
void gotoxy_puts(int x, int y, char* s);
void w_gotoxy_putc(WINDOW *win, int x, int y, int c);
void w_gotoxy_puts(WINDOW *win, int x, int y, char *s);
void init_ui();
void show_message(WINDOW *win, struct ChatMsg *msg, int type);
#endif