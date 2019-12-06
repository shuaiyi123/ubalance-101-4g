/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-22 05:56:16
 * @LastEditTime: 2019-12-05 00:39:24
 * @LastEditors: Please set LastEditors
 */
#ifndef _SIEC101_2002_H
#define _SIEC101_2002_H

#include <string.h>
#include "gtypedef.h"
#include "syslog.h"

#define USER_FLAG_LENTH 25 //用户标志位长度
#define YC_LENTH 16 //遥测长度
#define YX_LENTH 2 //遥信长度
#define MAX_TXDFM_SIZE  256  //发送缓冲区大小
#define MAX_RXDFM_SIZE  256  //接收缓冲区大小
#define MIN_RXDFM_SIZE  5    //最小接收帧长
//帧控制域定义
#define BIT_DIR    0x80    //传输方向位
#define BIT_PRM    0x40    //启动报文位
#define BIT_FCB    0x20    //帧计数位
#define BIT_FCV    0x10    //帧计数有效位
#define BIT_FUNC   0x0F    //功能码所占位
#define BIT_FCBFCV	0x30   //帧计数位和帧计数有效位
//控制域功能码
#define  RESREMLIN 0x00 //复位远方链路
#define  RESRTU    0x01 //复位远动终端
#define  SENDAT    0x03 //传送数据
#define  REQANS    0x08 //召唤要求访问位
#define  REQLINSTA 0x09 //请求远方链路状态
#define  CALFIRDAT 0x0A //召唤一级用户数据
#define  CALSECDAT 0x0B //召唤二级用户数据
//遥控命令格式
#define SELECT_OPEN  0x80 //预置控制分
#define SELECT_CLOSE 0x81 //预置控制合
#define TP_OPEN      0x00 //执行遥控分
#define TP_CLOSE     0x01//执行遥控合

extern BYTE YxData[YX_LENTH];  //遥信数据
extern short YcData[YC_LENTH];  //归一化遥测数据
extern float YcDataFloat[YC_LENTH];//短浮点遥测数据
//用户标志位
extern BYTE userFlag[USER_FLAG_LENTH];
//用户标志位
#define UF_RXDCONTROL		*(userFlag + 0) //保存本次的控制字
#define UF_NEXT_FCBFCV      *(userFlag + 1) //期待下次的FCBFCV位
#define UF_LINK_INIT_OK     *(userFlag + 2) //链路初始化标志位

#define UF_CALL_ALL_CONFM   *(userFlag + 3)  //总召唤命令确认标志
#define UF_CALL_ALL_YX      *(userFlag + 4)  //总召遥信标志 
#define UF_CALL_ALL_YC      *(userFlag + 5)  //总召遥测标志
#define UF_CALL_ALL_END     *(userFlag + 6)  //总召命令结束标志

#define UF_ONE_GROUP_CONFM  *(userFlag + 7)  //确认召唤第1组数据(重要遥信)
#define UF_ONE_GROUP_DATA   *(userFlag + 8)  //上送第1组数据标志
#define UF_ONE_GROUP_END    *(userFlag + 9)  //结束召唤第1组数据(重要遥信)
#define UF_NINE_GROUP_CONFM *(userFlag + 10) //确认召唤第9组数据(重要遥测)
#define UF_NINE_GROUP_DATA  *(userFlag + 11) //上送第9组数据标志
#define UF_NINE_GROUP_END   *(userFlag + 12) //结束召唤第9组数据(重要遥测)

#define UF_DELAY_GET        *(userFlag + 13) //延时获得命令

#define UF_YK_SELECT	    *(userFlag + 14) //遥控预置标志
#define UF_YK_CANCLE	    *(userFlag + 15) //遥控撤销标志
#define UF_YK_EXCU		    *(userFlag + 16) //遥控执行确认标志
#define UF_YK_EXCU_FINISH   *(userFlag + 17) //遥控执行结束标志

#define UF_LINK_TEST        *(userFlag + 18) //链路测试

#define UF_RESET_PROCESS    *(userFlag + 19) //复位进程标志
#define UF_RESET_QUALITY    *(userFlag + 20) //复位进程品质描述词

#define UF_CHANGE_YX        *(userFlag + 21) //变位遥信标志(一级单点SOE)(接外部标志)
#define UF_CHANGE_YX_CLASS2 *(userFlag + 22) //带长时标遥信标志(二级SOE)
#define UF_CHANGE_YC        *(userFlag + 23) //越限遥测标志(接外部标志)
#define RTU_INIT_FINISH     *(userFlag + 24) //设备初始化完成标志位(接外部标志)

#define SET_USER_FLAG(flag)    (flag) = TRUE //将标志位置1
#define RESET_USER_FLAG(flag)  (flag) = FALSE//将标志位置0
//将所有标志位清零
#define RESET_ALL_USER_FLAG(point_start_addr) memset(point_start_addr,0,sizeof(BYTE)*USER_FLAG_LENTH) 

#ifndef PACKED
#define PACKED __attribute__((packed)) //不对齐，结构体的长度，就是各个成员变量长度的和
#endif
//用于遥测短浮点数转换
typedef union union_YC
{
    BYTE  buf[4];
    float dt_float;
}UNION_YC;
extern UNION_YC yc_date;
//用来保存遥控参数
typedef struct YK_PARA
{
    BYTE cmd; //记录遥控命令
    WORD addr; //遥控地址
}PACKED YK_ARGU;
extern YK_ARGU yk_argu;//保存接收的遥控参数
//时钟延时
typedef struct 
{
    WORD msec_t1;
    WORD msec_t2;
}CLOCK_SYN_STRUCT;
extern CLOCK_SYN_STRUCT clock_syn;

// //I101S规约的固定帧长结构
// typedef struct
// {
//     BYTE bStart;  //启动字符，一个字符
//     BYTE bCtrl;   //控制域，一个字符
//     WORD wAddr;   //链路地址域（子站地址）
//     BYTE bChkSum; //帧校验和
//     BYTE bStop;   //结束字符
// } PACKED FIXFRM_STRUCT;

// //I101S规约的可变帧长结构
// typedef struct
// {
//     BYTE var_bStart;     //启动字符
//     BYTE var_bLength;    //长度
//     BYTE var_bLength1;   //长度
//     BYTE var_bStart1;    //启动字符
//     BYTE var_bCtrl;      //控制域
//     WORD var_wAddr;      //链路地址域
//     BYTE var_bType;      //类型标识
//     BYTE var_bDefini;    //结构限定字
//     BYTE var_bReason;    //传送原因
//     WORD var_wPulibAddr; //公共地址
//     BYTE var_bData;      //数据开始
// } PACKED  VARFRM_STRUCT;

//接收报文缓存区
typedef struct {
    BYTE *buf;
    WORD len;
}RXDFRAME_TAG;
extern RXDFRAME_TAG m_Rxd;
//发送报文缓存区
typedef struct {
    BYTE *buf;
    WORD len;
}TXDFRAME_TAG;
extern TXDFRAME_TAG m_Txd;
//帧句柄，用于初始化帧结构
typedef struct   
{
   // BYTE bBalance;      //传输模式(非平衡和平衡)
    BYTE bLinkAddrLen;   //链路地址长度
    WORD wLinkAddr;     //链路地址
    BYTE bAsduAddrLen;   //公用地址长度
    WORD wAsduAddr;      //设备地址/公共地址
    BYTE bInfoAddrLen;   //信息体地址长度
    BYTE bCauseTransLen; //传送原因长度
    WORD yxStartAddr; //遥信基地址
    WORD ycStartAddr; //遥测基地址
    WORD ykStartAddr; //遥控基地址
} PACKED TIEC101CFG;
extern TIEC101CFG m_pCfg;

//接收模块函数声明
int RxdMonitor(); //接收报文处理
int SearchFrame();//检索报文
int RxdFixFrame(); //固定帧处理
int RxdVarFrame(); //可变帧处理
BYTE ChkSum(BYTE* p_Addr,WORD chek_len);//计算校验和
void RxdResetLink();//0x00 复位远方链路
void RxdReqLinkStatus();//0x09 请求远方链路状态
int RxdClass1Data();//召唤一级用户数据
int RxdClass2Data();//远方链路状态完好或召唤二级用户数据
void RxdStationCall();//总召唤/召唤某一组数据
int ChkVarControl(BYTE funCode);  //检查链路服务是否完好
int RxdClockSyn();//时钟同步
void RxdTestLink();//链路测试
void ResetProcess();//复位进程
int RxdYkCmd();//遥控处理
int CheckAndRetrans(BYTE bControl);//检查上一帧是否发送成功,失败则重发
void SaveCtrlCode(BYTE funCode);//保存控制字

//发送模块函数声明
int FixAck(); //固定帧应答
void TxdVarFrmHead(BYTE bType,BYTE bReason);//可变帧头部
void TxdVarFrmTail(BYTE bPRM,BYTE bCode,BYTE bNum);//可变帧尾部
void TxdFixFrame(BYTE bPRM, BYTE bCode);//固定帧
BYTE GetCtrlCode(BYTE bPRM, BYTE bCode); //合成发送帧控制域
void Txd_NoData(); //回答没有数据
void Txd_NoData_E5();
void TxdRetry();//重发数据
void TxdRtuInitFinish();//链路初始化结束帧
void Txd_CallAllCmd(BYTE bReason);   //总召命令
void Txd_CallYcData(BYTE num,BYTE bReason,BYTE bType);//上送遥测,归一化
void Txd_callYcFloat(BYTE num,BYTE bReason);//上送短浮点遥测
void Txd_CallYxData(BYTE num,BYTE bReason);//上送遥信数据
void Txd_callGroupCmd(BYTE bReason,BYTE QOI);//组召唤命令 确认/结束
void Txd_delay_get(); //延时获得命令
void UpdateSysTime(); //更新系统时间
void TxdClockSyn();//时钟对时
void RxdDelay();//延时获得
void TimeScaleCp56(BYTE *pBuf);//获取7字节时标
void Txd_changeYc15();//不带时标的越限归一化遥测值
void Txd_changeYc22();//带7字节时标的越限归一化遥测值
void Txd_changeYc24();//带7字节时标的越限短浮点遥测值
void Txd_changeSoeNoTime();//不带时标的单点soe(变位遥信)
void Txd_changeCp56Soe();//带7字节的单点soe(变位遥信)
void Txd_YkRespondCmd(BYTE ykCmd,BYTE bReason);//处理遥控命令
void TxdTestLink();//测试链路
void TxdResetProcess(BYTE quality);//复位进程
int SearchClass1();//寻找1级数据
//规约基本配置模块
void siec101_default_cfg();
#endif
