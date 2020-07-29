/**************************************************************** 
    > Author: Deser
    > e-Mail: yongk202@163.com
    > Date: 2020-07-24 Fri 11:38:31
    > LastEditors: Deser
    > LastEditTime: 2020-07-26 Sun 21:04:22
 ****************************************************************/
#include "WebSocket.h"

void GenResponseHead(char* response, char* key)
{
    int needle;
    memset(response, 0, BUFLENGTH);
    strcat(response, "HTTP/1.1 101 Switching Protocols\r\n");
    strcat(response, "Upgrade: websocket\r\n");
    strcat(response, "Connection: Upgrade\r\n");

    char _str_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char accept[200] = { 0 };
    strcat(accept, key);
    strcat(accept, _str_guid);

    // sha1
    SHA1Context sha;
    uint8_t _sha_accept[SHA1HashSize];
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char*)accept, strlen(accept));
    SHA1Result(&sha, _sha_accept);

    // base64
    char* _base_sha_accept = base64_encode(_sha_accept);

    strcat(response, "Sec-WebSocket-Accept: ");
    strcat(response, _base_sha_accept);
    strcat(response, "\r\n\r\n");
    free(_base_sha_accept);
}

void getKey(char* head, char* key)
{
    head = strstr(head, "Sec-WebSocket-Key: ");
    head += strlen("Sec-WebSocket-Key: ");
    while (1) {
        if (*head == '\r' || *head == '\n') {
            *key = '\0';
            break;
        }
        *key = *head;
        head++;
        key++;
    }
    return;
}

unsigned long long ParsePack(char* Package, char* data)
{
    int fin, maskflag;
    unsigned long long datalen;
    char mask[4] = { 0 };
    char *datahead, *maskhead;

    fin = Package[0] & 0x80;
    //暂时不考虑多帧数据
    if (!fin) {
        return 0;
    }

    maskflag = Package[1] & 0x80;
    //没有掩码暂不分析
    if (!maskflag) {
        return 0;
    }

    datalen = Package[1] & 0x7F;
    maskhead = Package + 2;
    datahead = Package + 6;
    if (datalen == 126) {
        ((char*)&datalen)[1] = Package[2];
        ((char*)&datalen)[0] = Package[3];
        maskhead += 2;
        datahead += 2;
    } else if (datalen == 127) {
        for (int i = 0; i < 8; i++) {
            ((char*)&datalen)[i] = Package[9 - i];
        }
        maskhead += 8;
        datahead += 8;
    }

    //暂时不考虑大文本
    if (datalen > BUFLENGTH) {
        return 0;
    }

    memcpy(mask, maskhead, 4);
    memset(data, 0, BUFLENGTH);
    memcpy(data, datahead, datalen);
    for (int i = 0; i < datalen; i++) {
        data[i] = (data[i] ^ mask[i % 4]);
    }
    data[datalen] = '\0';
    return datalen;
}

unsigned long long PackData(const char* data, unsigned long long datalen, unsigned char* Package)
{
    if (!(data && Package)) {
        return 0;
    }

    Package[0] = 0x81;

    unsigned long long PackLen;

    if (datalen < 126) {
        PackLen = datalen + 2;
        Package[1] = (unsigned char)datalen;
    } else if (datalen <= 0xFFFF) {
        PackLen = datalen + 4;
        Package[1] = 126;
        Package[2] = ((unsigned char*)&datalen)[1];
        Package[3] = ((unsigned char*)&datalen)[0];
    } else {
        PackLen = datalen + 10;
        Package[1] = 127;
        for (int i = 0; i < 8; i++) {
            Package[9 - i] = ((unsigned char*)&datalen)[i];
        }
    }

    //暂时不考虑大文本
    if (PackLen > BUFLENGTH) {
        return 0;
    }

    memcpy(Package + PackLen - datalen, data, datalen);
    return PackLen;
}

bool ws_echo(int fd)
{
    char buf[BUFLENGTH];
    char data[BUFLENGTH];
    unsigned long long datalen, packlen;

    memset(buf, 0, BUFLENGTH);
    int n = recv(fd, buf, BUFLENGTH, 0);
    if (n < 0) {
        perror("recv()");
        return false;
    }
    // for(int i = 0; i < n; i++){
    //     printf("%u ", (unsigned char)buf[i]);
    // }
    // putchar('\n');
    unsigned char opcode = buf[0] & 0xF;
    if (opcode == 0x8) {
        return false;
    }
    if (opcode != 0x1) { //暂不考虑其他控制帧
        return false;
    }
    datalen = ParsePack(buf, data);
    if (!datalen) {
        printf("文本过长\n");
        return false;
    }
    printf("data(%llu个字符): %s\n", datalen, data);
    memset(buf, 0, BUFLENGTH);
    packlen = PackData(data, datalen, buf);
    if (!packlen) {
        printf("文本过长\n");
        return false;
    }
    send(fd, buf, packlen, 0);
    // printf("%s\n", buf);
    return true;
}

bool ShakeHand(int fd)
{
    char buf[BUFLENGTH];
    memset(buf, 0, BUFLENGTH);
    int n = recv(fd, buf, BUFLENGTH, 0);
    if (n < 0) {
        perror("recv()");
        return false;
    }
    // printf("response:\n%s\n", buf);
    char key[GUIDLEN], response[BUFLENGTH];
    memset(key, 0, GUIDLEN);
    memset(response, 0, BUFLENGTH);
    getKey(buf, key);
    GenResponseHead(response, key);
    int ret = send(fd, response, BUFLENGTH, 0);
    // printf("reponse:\n%s\n", response);
    if (ret < 0) {
        perror("send");
        return false;
    }
    
    return true;
}