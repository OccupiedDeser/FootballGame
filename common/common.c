/**************************************************************** 
    > Author: KovZhu
    > e-Mail: kovzhu@qq.com
    > Date: 2020-07-08 Wed 15:12:59
    > LastEditors: KovZhu
    > LastEditTime: 2020-07-08 Wed 16:01:26
 ****************************************************************/

#include "common.h"

char *get_conf_value(const char *path, const char *key)
{
    FILE *fp = fopen(path, "r");
    if(fp == NULL)
    {
        printf("Error! File not found!");
    }
    
}

