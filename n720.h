/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-13 18:51:43
 * @LastEditTime: 2019-08-20 01:36:46
 * @LastEditors: Please set LastEditors
 */
#ifndef _N720_H_
#define _N720_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>

int check_CGATT_state(int fd, int timeout_sec);
int check_CSQ_state(int fd, int timeout_sec);
int check_CPEG_state(int fd, int timeout_sec);
int check_MYNETACT_state(int fd, int timeout_sec);
int check_socket_state(int fd,int timeout_sec);
int check_connect_state(int fd,int timeout_sec);
int wait_ok_string(int fd, int timeout_sec);
bool n720_check_baudrate(int fd);
bool set_n720_baudrate(int fd,int baudrate);
bool n720_soft_rst(int fd);
bool n720_temp_check(int fd);
int n720_init(int fd,int baudrate,struct termios *opt);
int tcp_client_connect(int *fd,char *addrpwd);
void tcp_client_disconnet(int *fd,int socId);
#endif