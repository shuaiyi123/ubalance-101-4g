/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-14 00:52:19
 * @LastEditTime: 2019-12-05 18:27:45
 * @LastEditors: Please set LastEditors
 */
#ifndef _TERMINAL_IO_H_
#define _TERMINAL_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

#include "gtypedef.h"
#include "syslog.h"

int gfd;//全局串口文件描述符

int  open_serial(char *dev);
void close_serial(int pf);
void termios_setup(int fd, speed_t speed_param);
int set_termios(int fd, struct termios *options, int databits, int stopbits, int parity);
int set_baudrate(int fd, struct termios *opt, int baudrate);
int find_baudrate(int rate);
int serial_read_ch(int fd, char *c, struct timeval *read_tv);
int serial_read(int fd, const void *data, int bufLen,int timeout_sec);
int TransData(BYTE *buff, int bufLen);
int serial_write(int fd, const void *buf, int len, struct timeval *write_tv);
int RecvData(BYTE *buff,int bufLen);
int serial_init(int *fd,int baud,struct termios *opt);
#endif