#ifndef TCP_PROC_H
#define TCP_PROC_H
#include <stdio.h>
#include <dirent.h>

typedef struct NET_TCP_4 {
    char sl[4];
    char local_address[15];
    char rem_address[15];
    char uid[10];
    char inode[10];
}net_tcp4;

net_tcp4 *create_tcp4_table(int *total_len, char *file_path);

void print_tcp4_table(const net_tcp4 *tcp_table, const int total_len);

void netstat_tcp4(net_tcp4 *net_table, char *dir_path, const int total_len);
#endif