#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/select.h>
#include <signal.h>

#include "sysfs_io.h"
#include "n720.h"
#include "terminal_io.h"
#include "gtypedef.h"
#include "siec101_2002.h"

#ifdef LOG
    FILE *log_fd;
#endif
// /* Short option names */
// static const char g_shortopts [] = "b:i:vh";

// /* Option names */
// static const struct option g_longopts [] = {
//     { "baudrate",      required_argument,      NULL,        'b' },
//     { "ip:port",       required_argument,      NULL,        'i' },
//     { "version",       no_argument,            NULL,        'v' },
//     { "help",          no_argument,            NULL,        'h' },
//     { 0, 0, 0, 0 }
// };

// static void usage(FILE *fp, int argc, char **argv) {
//     fprintf(fp,
//             "Usage: %s [options]\n\n"
//             "Options:\n"
//             " -b | --baudrate        baudrate range form 0~921600bit/s\n"
//             " -i | --ipPort          ip:port such as 192.68.137.5:8000\n"
//             " -v | --version         Display version information\n"
//             " -h | --help           Show help content\n"
//             " eg ./main  -b 230400 -i 192.168.137:8000\n"
//             "    ./main -b 921600 120.137.50.2:8000\n\n"
//             , basename(argv[0]));
// }
// //输入的错误参数处理
// static void opt_parsing_err_handle(int argc, char **argv, int flag) {
//     /* Exit if no input parameters are entered  */
//     int state = 0;
//     if (argc < 2) {
//         printf("No input parameters are entered, please check the input.\n");
//         state = -1;
//     } else {
//         /* Feedback Error parameter information then exit */
//         if (optind < argc || flag) {
//             printf("Error:  Parameter parsing failed\n");
//             if (flag)
//                 printf("\tunrecognized option '%s'\n", argv[optind-1]);

//             while (optind < argc) {
//                 printf("\tunrecognized option '%s'\n", argv[optind++]);
//             }

//             state = -1;
//         }
//     }

//     if (state == -1) {
//         printf("Tips: '-h' or '--help' to get help\n\n");
//         exit(2);
//     }
// }
//信号处理函数
void sigHan(int sig)
{
    printf("pressed down ctrl+c ");
    //tcp_client_disconnet(&gfd,0);
    #ifdef LOG 
        if(log_fd != NULL){
            fclose(log_fd);//记录报文文件描述词
        }
    #endif
    exit(0);
}
int main(int argc, char **argv) {
    //char ip_port[32];
    //int c=0;
    int baudrate=1200;
	struct termios opt;
    int flag=0;

    // /* Parsing input parameters */
    // while ((c = getopt_long(argc, argv, g_shortopts, g_longopts, NULL)) != -1) {
    //     switch (c) {
    //     case 'b':
    //         baudrate = atoi(optarg);
    //         break;
    //     case 'i':
    //         strncpy(ip_port,optarg,strlen(optarg)); //将ip地址和端口号复制到ip_port
    //         ip_port[strlen(optarg)]='\r';//添加AT指令结束符
    //         ip_port[strlen(optarg)+1]='\0';//添加字符串结束符
    //         break;
    //     case 'v':
    //         /* Display the version */
    //         printf("version : 1.0\n");
    //         exit(0);
    //     case 'h':
    //         usage(stdout, argc, argv);
    //         exit(0);   
    //     default :
    //         flag = 1;
    //         break;
    //     }
    // }
    // opt_parsing_err_handle(argc, argv, flag);
    // //信号处理
     (void)signal(SIGINT,sigHan);
     //串口初始化
    flag=serial_init(&gfd,baudrate,&opt);
    if(flag==false){
         printf("serial inital failed.\n");
         return -1;
     }
    // //4g模块初始化
    // loop:flag=n720_init(gfd,baudrate,&opt);
    // if(flag==false){
    //     printf("n720 initial failed.\n");
    //     return -1;
    // }
    // //TCP连接
    // flag=tcp_client_connect(&gfd,ip_port);
    // if(flag == -2){ //连接失败,重连
    //     goto loop;
    // }
    #ifdef LOG
        time_t now;
        struct tm timenow;
        char date[32];
        time(&now);                     //得到时间秒数
        now = now + 8*3600;
        localtime_r(&now, &timenow);    //线程安全,将秒数转化为日历，并存储在timenow结构体
        strftime(date,32,"%Y-%m-%d,%H:%M:%S.txt",&timenow);
        log_fd=fopen(date,"w");
    #endif
     //初始化101基本配置
    siec101_default_cfg();
    //分配内存
    m_Rxd.buf = (BYTE *)malloc(MAX_RXDFM_SIZE);
    m_Txd.buf = (BYTE *)malloc(MAX_TXDFM_SIZE);
    //内存初始化
    memset(m_Rxd.buf, 0, sizeof(BYTE) * MAX_RXDFM_SIZE);
    memset(m_Txd.buf, 0, sizeof(BYTE) * MAX_TXDFM_SIZE);
    while(1){
        if(RecvData(m_Rxd.buf, MAX_RXDFM_SIZE)>0){
            RxdMonitor();  
        }
    }
    close_serial(gfd);
    return 0;
}
