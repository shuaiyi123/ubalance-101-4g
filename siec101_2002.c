/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-22 20:30:54
 * @LastEditTime: 2019-12-05 05:06:09
 * @LastEditors: Please set LastEditors
 */
#include "siec101_2002.h"

UNION_YC yc_date; //提取遥测短浮点型字节
TXDFRAME_TAG m_Txd; //发送报文缓存区
RXDFRAME_TAG m_Rxd ; //接收报文缓冲区
YK_ARGU yk_argu; ////保存接收的遥控参数
CLOCK_SYN_STRUCT clock_syn;//时钟延时
BYTE YxData[YX_LENTH]={1,0};  //遥信数据
 //归一化遥测数据
short YcData[YC_LENTH]={0,11065,11384,11349,19527,19482,19512,117,147,138,-28738,-28574,-28624,-28576,-31904,-1};
//短浮点数遥测数据
float YcDataFloat[YC_LENTH]={101.22,98.32,21.541,35.253,65.231,87.85,118.87,132.52,32.8};
BYTE userFlag[USER_FLAG_LENTH]={0}; //用户标志位初始化为0
BYTE devFlag[5]={0}; //设备标志为,初始化为0
TIEC101CFG m_pCfg;  //报文配置结构体

/**
 * @description: RTU初始化，主要对RTU的一些基础配置
 * @param {none} 
 * @return:
 */
void siec101_default_cfg()
{
   // m_pCfg.bBalance = 0; //非平衡传输
    m_pCfg.bLinkAddrLen = 1;    //链路地址长度：1/2字节
    m_pCfg.wLinkAddr = 0x0001; //链路地址
    m_pCfg.bAsduAddrLen = 2;   //公共地址长度：1/2字节 
    m_pCfg.wAsduAddr = 0x0001;  //公共地址
    m_pCfg.bCauseTransLen = 1;  //传输原因：1/2字节
    m_pCfg.bInfoAddrLen = 2;     //信息体地址：2/3字节     
    m_pCfg.yxStartAddr = 0x0001; //遥信基地址 1H------4000H
    m_pCfg.ycStartAddr = 0x4001; //遥测基地址 4001H------5000H
    m_pCfg.ykStartAddr = 0x6001; //遥控基地址 6001H------6100H
}
