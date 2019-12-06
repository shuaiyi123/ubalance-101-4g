#include "terminal_io.h"
#include "n720.h"
#include "sysfs_io.h"

/* GPRS附着 检查*/
int check_CGATT_state(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    int state;
    char ch = '?';
    char last = '?'; 
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for (i = 0; i < 256; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        putchar(ch);
        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            sscanf(buf,"%*[^=]=%d",&state);
           // printf("state=%d\n",state);
            if(state==0){
                ret = 0;
                break;
            }
            else {
                ret=1;
                break;
            }
        }
        last = ch;
        buf[i]=ch;
    }
    return ret;
}

//查询信号强度
int check_CSQ_state(int fd, int timeout_sec)
{
    int i;
    int ret = 0,num=0;
    int state;
    char ch = '?';
    char last = '?'; 
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for (i = 0; i < 256; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        /* display the char */
        putchar(ch);
        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            sscanf(buf,"%*[^:]:%d",&state);
            if(state<12||state==99){
                ret = 0;
                break;
            }
            else {
                ret=1;
                break;
            }
        }
        last   = ch;
        buf[i]=ch;
    }
    return ret;
}

//查询网络是否注册成功
int check_CPEG_state(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for (i = 0; i < 256; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        putchar(ch);
        /* check "ok" */
        if (last == ',' && (ch == '1'||ch=='5')) {
            ret = 1;
            break;
        }
        //putchar(ch);
        last = ch;
    }
    return ret;
}
//激活网络，重复激活返回902错误代码
int check_MYNETACT_state(int fd, int timeout_sec)
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for (i = 0; i < 256; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        /* display the char */
        putchar(ch);
        /* check "ok" */
        if ((last == 'O'||last=='0') && (ch == 'K'||ch=='2')) {
            ret = 1;
            break;
        }
        last = ch;
    }
    return ret;
}
//检查socket号打开状态
int check_socket_state(int fd,int timeout_sec)
{
    int i;
    int ret =0,num=0;
    char ch ='?';
    char buf[256];
    struct timeval timeout;

    timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for(i=0;i<256;i++){
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        putchar(ch);
        buf[i]=ch;
        buf[i+1]='\0';
        if(strstr(buf,"OK") || strstr(buf,"913")){//返回OK或913说明已打开
            ret =1;
            break;
        }      
    }
    return ret;
}
//检查connet连接状态
int check_connect_state(int fd,int timeout_sec)
{
    int i;
    int ret =0,num=0;
    char ch ='?';
    char buf[256];
    struct timeval timeout;

    timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for(i=0;i<255;i++){
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0){
            buf[i+1]='\0';  
            if((strstr(buf,"CONNECT")!=NULL) || (strstr(buf,"912")!=NULL)){//返回OK或912说明已打开
                ret =1;
            } 
        break;
        }
        putchar(ch);
        buf[i]=ch;  
    }
    return ret;
}
//检查是否收到OK字符
int wait_ok_string(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
    for (i = 0; i < 256; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        /* display the char */
        putchar(ch);
        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            ret = 1;
            break;
        }
        last = ch;
    }
    return ret;
}
//查询4g模块波特率
bool n720_check_baudrate(int fd)
{
    int is_ok=0;
    int count;
    struct timeval tv;
    
    tv.tv_sec=1;
    tv.tv_usec=0;
    count=0;
    do {//查询波特率
        count++;
    	 serial_write(fd, "AT+IPR?\r", strlen("AT+IPR?\r"), &tv);
            
        is_ok = wait_ok_string(fd, 1);
    	if (is_ok != 1) {
            usleep(100000);
        }
    } while(is_ok != 1 && count < 3);
   
    if (is_ok != 1) {
        printf("AT+IPR?\r\nerror exit!\n");
        return false;
    }
    return true;
}
/*设置波特率为230400
/const char *cmd1Str[] = {"AT+IPR=230400\r"};*/
//设置N720模块波特率
bool set_n720_baudrate(int fd,int baudrate)
{
    int is_ok=0;
    short count=0;
    struct timeval tv;
    char br[32];
    
    tv.tv_sec=3;
    tv.tv_usec=0;
    sprintf(br,"AT+IPR=%d\r",baudrate);//格式化输出到br_temp中
    count=0;
    do { //设置波特率
        count++;
    	serial_write(fd,br, strlen(br), &tv);
        is_ok = wait_ok_string(fd, 1);
    	if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 5);
    if (is_ok != 1) {
        printf("%s\nerror exit!\n",br);
        return false;
    }
    else printf("\nBaudrate of n720 is modifed 230400\n");
    return true;
}
//4g模块软重启
bool n720_soft_rst(int fd)
{
    int count=0;
    int is_ok=0;
    struct timeval tv;

    tv.tv_sec=3;
    tv.tv_usec=0;
    do{
        count++;
        serial_write(fd,"AT+CFUN=1,1\r",strlen("AT+CFUN=1,1\r"),&tv);
        is_ok=wait_ok_string(fd,1);
        if(is_ok!=1){
            sleep(1);
        }
    }while(is_ok!=1 && count<3);
    if(is_ok!=1){
        printf("AT+CFUN=1,1\nerror exit!\n");
        return false;
    }
    return true;
}
//4g模块查询环境温度
bool n720_temp_check(int fd)
{
    int count=0;
    int is_ok;
    struct timeval tv;

    tv.tv_sec=3;
    tv.tv_usec=0;
    do{
        count++;
        serial_write(fd,"AT$MYADCTEMP=0\r",strlen("AT$MYADCTEMP=0\r"),&tv);
        is_ok=wait_ok_string(fd,1);
        if(is_ok!=1){
            sleep(1);
        }
    }while(is_ok!=1 && count<3);
    if(is_ok!=1){
        printf("AT$MYADCTEMP=0\nerror exit\n");
        return false;
    } 
    return true;
}
//n720初始化
int n720_init(int fd,int baudrate,struct termios *opt)
{
    int flag;
    int state=1;
    int rate,cnt=0;
       //检查4g模块状态
    while(state){
        state=N720_ReadState();
        if(state==1){
            printf("Failure to check n720 module    ");
            sleep(1);
        }
    }
    //取消加热sim卡
    N720_NonheatSimCard();
    //查询4g模块波特率
    loop:flag=n720_check_baudrate(fd);
    if(flag==true){  //4g模块波特率为115200，则修改为baudrate
        set_n720_baudrate(fd,baudrate);
    }
    //串口终端波特率修改为baudrate
    rate=find_baudrate(baudrate);
    flag=set_baudrate(fd,opt,rate);
    if(flag==false){
        printf("set usart baudrate of 230400 faild\n");
        return false;
    }
    //查询波特率是否修改成功
    flag=n720_check_baudrate(fd); //成功返回1
    if(flag != 1){
        if(cnt==3)
            return false;
        cnt++;
        N720_Reset(); //重启后4g模块波特率为9600-115200
        set_baudrate(fd,opt,B115200);
        goto loop;
    }
    return true;
}

//获取模块软件版本，，获取模块厂商信息，，查询PIN状态，，获取SIM卡标识
#define CMD1_COUNT 4
const char *cmd1Str[CMD1_COUNT]={"AT+GMR\r","ATI\r","AT+CPIN?\r","AT$MYCCID\r"};
//查询信号强度，，查询注册网络
#define CMD2_COUNT 2
const char *cmd2Str[CMD2_COUNT]={"AT+CSQ\r","AT+CREG?\r"};
//设置GPRS附着，,，查询当前网络运行制式，,，查询远程通信模块类型，,，设置APN参数
//设置用户名、密码，,，设置PAP认证，,，IP访问控制配置打开内置协议栈主动上报
#define CMD3_COUNT 9
const char *cmd3Str[CMD3_COUNT]={"AT+CGATT?\r","AT$MYSYSINFO=7\r","AT$MYSYSINFO\r","AT$MYTYPE?\r","AT$MYNETCON=0,\"APN\",\"CMNET\"\r",
            "AT$MYNETCON=0,\"USERPWD\",\"\"\r","AT$MYNETCON=0,\"AUTH\",1\r","AT$MYIPFILTER=0,2\r","AT$MYNETURC=1\r"};
//激活/去激活网络连接，，,查询本地IP
#define CMD4_COUNT 2
const char *cmd4Str[CMD4_COUNT]={"AT$MYNETACT=0,1\r","AT$MYNETACT?\r"};
//设置非透明传输服务参数AT$MYNETSRV=<Channel>,<SocketID>,<nettype>,<viewMode>,<ip:port><，，，开启服务AT$MYNETOPEN=<SocketID>    
#define CMD5_COUNT 3
const char *cmd5Str[CMD5_COUNT]={"AT$MYNETCLOSE=1\r","AT$MYNETSRV=0,1,0,0,120.76.196.44:8000\r","AT$MYNETOPEN=1\r"};
//TCP透传客户端模式
int tcp_client_connect(int *fd,char *addrpwd)
{
    short i,j,count;
    int is_ok;
    struct timeval tv;
    char adpw[64]="AT$MYNETCREATE=0,0,1,";
    tv.tv_sec=3;
    tv.tv_usec=0;
    
    strcat(adpw,addrpwd); //将字符串addrpwd追加到adpw后面
    strcat(adpw,"\r");
    //获取模块软件版本，，获取模块厂商信息，
    for (i = 0 ; i < CMD1_COUNT-2; i++) {
        count++;
	    serial_write(*fd, cmd1Str[i], strlen(cmd1Str[i]), &tv);
        is_ok = wait_ok_string(*fd, 1);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd1Str[i]);
        }
    }
    //查询PIN状态，，获取SIM卡标识
    for (i = 2 ; i < CMD1_COUNT; i++) {
        count=0;
        do{  //查询CCID,查询不到重启
            count++;
            serial_write(*fd,cmd1Str[i],strlen(cmd1Str[i]),&tv);
            is_ok = wait_ok_string(*fd, 1);
	        if (is_ok != 1) {
                sleep(1);
            }

        }while(is_ok != 1 && count < 15);
        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd1Str[i]);
            return -2;
        }
    }
    //查询信号强度，，查询注册网络,失败则重启
    for (i = 0 ; i < CMD2_COUNT; i++) {
        count = 0;
        do {   //信号强度过低或注册不上网络，则重启
            count++;
	        serial_write(*fd, cmd2Str[i], strlen(cmd2Str[i]), &tv);
            if(i==0){
                is_ok = check_CSQ_state(*fd, 1);
            } 
            else if(i==1){
                is_ok = check_CPEG_state(*fd, 1);
            }

            if (is_ok != 1) {
                sleep(1);
            }
        } while(is_ok != 1 && count < 25);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd2Str[i]);
            return -2; //重启模块
        }
    }
    //设置GPRS附着，,，查询当前网络运行制式，,，查询远程通信模块类型，,，设置APN参数
    //设置用户名、密码，,，设置PAP认证，,，IP访问控制配置打开内置协议栈主动上报
    for (i = 0 ; i < CMD3_COUNT ; i++) {
        count = 0;
        do {
            count++;
	        serial_write(*fd, cmd3Str[i], strlen(cmd3Str[i]), &tv);
           // printf("AT指令:%s",cmd3Str[i]);
            if(i==0){//GPRS网络附着
                is_ok = check_CGATT_state(*fd, 1);
                if(is_ok!=1){
                    j=0;
                    do{
                        j++;
                        serial_write(*fd, "AT+CGATT=1\r", strlen("AT+CGATT=1\r"), &tv);
                        is_ok = wait_ok_string(*fd, 1);
                        if (is_ok != 1) {
                            sleep(1);
                        } 
                    }while(is_ok != 1 && j < 3);
                }
            } 
            else {
                is_ok = wait_ok_string(*fd, 1);
            } 
            if (is_ok != 1) {
                 sleep(1);
            }
        } while(is_ok != 1 && count < 3);
        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd3Str[i]);
            //return -1;
        }
    }
    //激活/去激活网络连接，，,查询本地IP
    for(i=0;i<CMD4_COUNT;i++){
        count=0;
        do{
            count++;
            serial_write(*fd, cmd4Str[i], strlen(cmd4Str[i]), &tv);
            if(i==0){
                is_ok=check_MYNETACT_state(*fd,1);
            }
            else{
                is_ok = wait_ok_string(*fd, 1);
            }
            if (is_ok != 1) {
                sleep(1);
            }
        }while(is_ok != 1 && count < 3);
        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd4Str[i]);
            return -2; //tcp连接失败重启
        }  
    }
    //关闭待用socket
    count = 0;
    do {  
        count++;
	    serial_write(*fd,cmd5Str[0],strlen(cmd5Str[0]),&tv);
        is_ok = check_socket_state(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 3);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", cmd5Str[0]);
    }  
    //设置透明传输服务参数AT$MYNETSRV=<Channel>,<SocketID>,<nettype>,<viewMode>,<ip:port><，，，开启服务AT$MYNETOPEN=<SocketID>
    count = 0;
    do {  //设置tcp连接模式以及服务器参数
        count++;
	    serial_write(*fd, adpw, strlen(adpw), &tv);
        is_ok = check_connect_state(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 3);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", adpw);
        return -2; //tcp连接失败重启
    } 
    printf("\n"); 
    return 0;
}
//断开tcp连接
void tcp_client_disconnet(int *fd,int socId)
{
    short count;
    int is_ok;
    struct timeval tv;
    char at_tcpClose[32]; //socket号为1

    tv.tv_sec=1;
    tv.tv_usec=0;
    count = 0;
    sprintf(at_tcpClose,"AT$MYNETCLOSE=%d\r",socId);//socket号转化为字符串
    usleep(100000);
    serial_write(*fd,"+++", strlen("+++"), &tv);//将数据模式切换为命令模式
    usleep(100000);
    do {  
        count++;
	    serial_write(*fd,at_tcpClose, strlen(at_tcpClose), &tv);
        is_ok=wait_ok_string(*fd,1);
	    if (is_ok != 1) { //关闭失败
            usleep(100000);
        }
    } while(is_ok != 1 && count < 3);
    if(is_ok!=1){
        printf("\nclosed socket error!\n");
    } 
    else {
        printf("\nclosed socket 1\n");  
    } 
    count = 0;
    do { 
        count++;
	    serial_write(*fd, "AT+CGATT=0\r", strlen("AT+CGATT=0\r"), &tv);
        is_ok = wait_ok_string(*fd, 1);
	    if (is_ok != 1) {
            usleep(1);
        }
    } while(is_ok != 1 && count < 5);
    if (is_ok != 1) {
        printf("AT+CGATT=0\nerror. exit!\n");
    } 
    printf("\n");
}
