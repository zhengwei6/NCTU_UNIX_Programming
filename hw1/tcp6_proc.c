#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include "tcp6_proc.h"

net_tcp6 *create_tcp6_table(int *total_len, char *file_path)
{
    FILE *fp;
    net_tcp6 *tcp6_array = NULL;
    char *line = NULL;
    int lines_count = 0;
    size_t len = 0;
    int read;
    char delim[] = " ";

    // count how many lines in file
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("open file error\n");
        return tcp6_array; 
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        lines_count += 1;
    }
    fclose(fp);
    // malloc net_tcp4
    tcp6_array = (net_tcp6 *)malloc(sizeof(net_tcp6) * lines_count);

    // store every line info
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("open file error\n");
        return tcp6_array; 
    }
    int table_index = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        char *ptr = strtok(line, delim);
        int col_index = 0;
        char address[33];
        char port[5];
        while (ptr != NULL) {
            if (col_index == 0) {
                memset(tcp6_array[table_index].sl, '\0', sizeof(tcp6_array[table_index].sl));
                strcpy(tcp6_array[table_index].sl, ptr);
            }
            else if (col_index == 1) {
                memset(address, '\0', 33);
                memset(port, '\0', 5);
                strncpy(address, ptr, 32);
                strncpy(port, ptr + 33, 4);
                struct in6_addr sin6_addr_tmp;
                for (int i = 0 ; i < 16; i++) {
                    sscanf(address+2*i, "%2" SCNx8, &sin6_addr_tmp.s6_addr[15 - i]);
                }
                for (int i = 0 ; i < 4 ; i++) {
                    for (int j = 0;j < 4; j++) {
                        tcp6_array[table_index].local_info.sin6_addr.s6_addr[i*4+j] = sin6_addr_tmp.s6_addr[4*(3 - i) + j];
                    }
                }
                sscanf(port, "%hx", &tcp6_array[table_index].local_info.sin6_port);
            }
            else if (col_index == 2) {
                memset(address, '\0', 33);
                memset(port, '\0', 5);
                strncpy(address, ptr, 32);
                strncpy(port, ptr + 33, 4);
                struct in6_addr sin6_addr_tmp;
                for (int i = 0 ; i < 16; i++) {
                    sscanf(address+2*i, "%2" SCNx8, &sin6_addr_tmp.s6_addr[15 - i]);
                }
                for (int i = 0 ; i < 4 ; i++) {
                    for (int j = 0;j < 4; j++) {
                        tcp6_array[table_index].rem_info.sin6_addr.s6_addr[i*4+j] = sin6_addr_tmp.s6_addr[4*(3 - i) + j];
                    }
                }
                sscanf(port, "%hx", &tcp6_array[table_index].rem_info.sin6_port);
            }
            else if (col_index == 7) {
                memset(tcp6_array[table_index].uid, '\0', sizeof(tcp6_array[table_index].uid));
                strcpy(tcp6_array[table_index].uid, ptr);
            }
            else if (col_index == 9) {
                memset(tcp6_array[table_index].inode, '\0', sizeof(tcp6_array[table_index].inode));
                strcpy(tcp6_array[table_index].inode, ptr);
            }
            col_index++;
            ptr = strtok(NULL, delim);
        }
        table_index++;
    }
    
    *total_len = lines_count;
    return tcp6_array;
}

void tcp6_address_form(net_tcp6 *tcp6_table, int total_len) 
{
    int i;
    char str[100];
    int port;
    //inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
    for (i = 0 ; i < total_len; i++) {
        memset(tcp6_table[i].local_address, '\0', 47);
        memset(tcp6_table[i].remote_address, '\0', 47);
        //printf("%s ",tcp_table[i].sl);
        inet_ntop(AF_INET6, &(tcp6_table[i].local_info.sin6_addr), str, INET6_ADDRSTRLEN);
        str[17] = '\0';
        //printf("%s\n", str);
        sprintf(tcp6_table[i].local_address, "%s:%hu", str, tcp6_table[i].local_info.sin6_port);
        //printf("%hu ", tcp_table[i].local_info.sin_port);
        inet_ntop(AF_INET6, &(tcp6_table[i].rem_info.sin6_addr), str, INET6_ADDRSTRLEN);
        str[17] = '\0';
        sprintf(tcp6_table[i].remote_address, "%s:%hu", str, tcp6_table[i].rem_info.sin6_port);
        //printf("%s ",tcp_table[i].uid);
        //printf("%s\n",tcp_table[i].inode);
    }
}

void read6_comm_file(char *path) 
{
    char *str = NULL;
    int read;
    size_t len = 0;
    FILE *fp  = fopen(path, "r");
    if (fp == NULL) {
        return; 
    }
    read = getline(&str, &len, fp);
    str[strlen(str) - 1] = '\0';
    printf("%s ", str);
    fclose(fp);
    return;
}

void read6_cmdline_file(char *path)
{
    char *str = NULL;
    int read;
    char delim[] = " ";
    size_t len = 0;
    FILE *fp  = fopen(path, "r");
    if (fp == NULL) {
        return;
    }
    read = getline(&str, &len, fp);
    char *ptr = strtok(str, delim);
    if (ptr == NULL) {
        return;
    }
    ptr = strtok(NULL, delim);
    if (ptr == NULL) {
        return;
    }
    while (ptr != NULL) {
        printf("%s ", ptr);
        ptr = strtok(NULL, delim);
    }
    return;

}

void read6_fd(net_tcp6 *net_table, char *dir_path, const int total_len, char *pid)
{
    struct dirent *pDirent;
    DIR *pDir;
    char buffer[300];
    char pathname[300];
    char link_buf[50];
    int i;
    memset(buffer, '\0', 300);
    strcpy(buffer, dir_path);
    strcat(buffer, "/fd/");
    pDir = opendir (buffer);
    if (pDir == NULL) {
        //printf("Can't open sub dir\n");
        return;
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.') {
            continue;
        }
        memset(pathname, '\0', 300);
        memset(link_buf, '\0', 50);
        strcpy(pathname,buffer);
        strcat(pathname, pDirent->d_name);
        readlink(pathname, link_buf, 50);
        if (strncmp(link_buf, "socket:", 6) == 0) {
            char *tmptr = &link_buf[8];
            tmptr[strlen(tmptr) - 1] = '\0';
            //search table
            for (i = 0 ; i < total_len; i++) {
                if (strcmp(tmptr, net_table[i].inode) == 0) {
                    printf("%-6s", "tcp6");
                    printf("%-24s", net_table[i].local_address);
                    printf("%-24s", net_table[i].remote_address);
                    printf("%s/", pid);
                    // /proc/[pid]/comm
                    char comm[100];
                    memset(comm, '\0', 100);
                    strcpy(comm, dir_path);
                    strcat(comm,"/comm");
                    read6_comm_file(comm);
                    // /proc/[pid]/cmdline
                    char cmdline_path[100];
                    memset(cmdline_path, '\0', 100);
                    strcpy(cmdline_path, dir_path);
                    strcat(cmdline_path,"/cmdline");
                    read6_cmdline_file(cmdline_path);
                    printf("\n");
                }
            }
        }
        else if(strncmp(link_buf, "[0000]", 7) == 0) {
            char *tmptr = &link_buf[7];
            tmptr[strlen(tmptr) - 1] = '\0';
            for (i = 0 ; i < total_len ; i++) {
                if (strcmp(tmptr, net_table[i].inode) == 0) {
                    printf("%-6s", "tcp6");
                    printf("%-24s\n", net_table[i].local_address);
                    printf("%-24s\n", net_table[i].remote_address);
                    printf("%s/", pid);
                    // /proc/[pid]/comm
                    char comm[100];
                    memset(comm, '\0', 100);
                    strcpy(comm, dir_path);
                    strcat(comm,"/comm");
                    read6_comm_file(comm);
                    // /proc/[pid]/cmdline
                    char cmdline_path[100];
                    memset(cmdline_path, '\0', 100);
                    strcpy(cmdline_path, dir_path);
                    strcat(cmdline_path,"/cmdline");
                    read6_cmdline_file(cmdline_path);
                    printf("\n");
                }
            }
        } 
    }
    return;
}

int check6_num(const char *name)
{
    return atoi(name);
}

void netstat_tcp6(net_tcp6 *net_table, char *dir_path, const int total_len)
{
    struct dirent *pDirent;
    char **dir_array = NULL;
    int i;
    DIR *pDir;
    pDir = opendir(dir_path);
    if (pDir == NULL) {
        return;
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        char buffer[300];
        if (check6_num(pDirent->d_name) == 0) {
            continue;
        }
        memset(buffer, '\0', 300);
        strcpy(buffer, dir_path);
        strcat(buffer, pDirent->d_name);
        DIR *tmpDir;
        tmpDir = opendir (buffer);
        if (tmpDir == NULL) {
            continue;
        }
        else {
            //read fd
            read6_fd(net_table, buffer, total_len, pDirent->d_name);
        }
        closedir(tmpDir);
    }

}
