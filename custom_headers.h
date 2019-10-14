#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<vector>
#include<utility>
#include<unordered_map>
#include <openssl/ssl.h>
// imports
void *setup_server(void *arguments);
void *serveReq(void *arguments);
int get_connection(char *ip,int port);
void send_file(int sockid);
void download_file();
int get_connection(char *ip,int port);
void *serveTrackreq(void *arguments);
void *tracker(void *arguments);
void share_file(int sockid,char *ip);
void share(char *port); 
void get_file(int sockid);
char* filehash(char *name);