/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-22 18:26:42
 * @LastEditTime: 2019-12-05 23:17:54
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "siec101_2002.h"
#include "terminal_io.h"
/**
 * @description: 发送固定帧
 * @param {bPRM:启动报文位.1下行,0上行,bCode:功能码} 
 * @return: 
 */
void TxdFixFrame(BYTE bPRM, BYTE bCode)
{
	BYTE bChkSum;
	BYTE *pBuf;

	pBuf = m_Txd.buf; //发送缓存区
	m_Txd.len = 0;

	pBuf[m_Txd.len++] = 0x10;//启动字符
	pBuf[m_Txd.len++] = GetCtrlCode(bPRM, bCode);  //获取控制字

	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.wLinkAddr); //链路地址域（子站站址）低位地址
	if (m_pCfg.bLinkAddrLen == 2){
		pBuf[m_Txd.len++] = HIBYTE(m_pCfg.wLinkAddr); //链路地址域（子站站址）高位地址
	}
	//校验和
	bChkSum = ChkSum(pBuf + 1, m_Txd.len-1);
	pBuf[m_Txd.len++] = bChkSum;
	//结束符
	pBuf[m_Txd.len++] = 0x16;
	//发送报文
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 合成可变帧头部
 * @param {type} 
 * @return: 
 */
void TxdVarFrmHead(BYTE bType,BYTE bReason)
{
    BYTE *pBuf;

    pBuf = m_Txd.buf;
    pBuf[0] = pBuf[3] = 0x68;//启动字符
    m_Txd.len = 5;//定位到链路地址

    pBuf[m_Txd.len++] = LOBYTE(m_pCfg.wLinkAddr);
    if(m_pCfg.bLinkAddrLen == 2){
        pBuf[m_Txd.len++] = HIBYTE(m_pCfg.wLinkAddr);
    }

    pBuf[m_Txd.len++] = bType;//类型标识符
    m_Txd.len++;//可变限定词
    pBuf[m_Txd.len++] = bReason;//传输原因
    if(m_pCfg.bCauseTransLen == 2){//传输原因长度
        pBuf[m_Txd.len++] = 0;
    }

    pBuf[m_Txd.len++] = LOBYTE(m_pCfg.wAsduAddr);//公共地址
    if(m_pCfg.bAsduAddrLen == 2 ){
        pBuf[m_Txd.len++] = HIBYTE(m_pCfg.wAsduAddr);
    }
}
/**
 * @description: 合成可变帧尾部
 * @param {type} 
 * @return: 
 */
void TxdVarFrmTail(BYTE bPRM,BYTE bCode,BYTE bNum)
{
    BYTE *pBuf;

    pBuf = m_Txd.buf;
    pBuf[1] = pBuf[2] = m_Txd.len - 4;//报文长度
    pBuf[4] = GetCtrlCode(bPRM,bCode);//控制字

    if(m_pCfg.bLinkAddrLen == 2){//可变结构限定词
        pBuf[8] = bNum;
    }
    else{
        pBuf[7] = bNum;
    }

    pBuf[m_Txd.len++] = ChkSum(pBuf+4,pBuf[1]);
    pBuf[m_Txd.len++] = 0x16; 
}
/**
 * @description:设置控制字 
 * @param {bPRM:启动报文位，bCode:功能码} 
 * @return: 返回控制字
 */
BYTE GetCtrlCode(BYTE bPRM, BYTE bCode)
{

	BYTE bCodeTmp = 0;

	bCodeTmp += bCode;

	if (bPRM){ //启动报文位
		bCodeTmp |= 0x40;
	}
	if(UF_LINK_INIT_OK == TRUE){ //初始化结束后才判断是否有一级用户数据
		if (SearchClass1() == OK){//有一级用户数据
			bCodeTmp |= 0x20;
		}   	 
	} 
	return bCodeTmp;
}
/**
 * @description:固定帧，子站向主站回应 
 * @param {none} 
 * @return:TRUE回答正确
 */
int FixAck()
{
	BYTE bRxdCode;
	BYTE bTxdCode = 0;

	bRxdCode = UF_RXDCONTROL & BIT_FUNC;
	if (bRxdCode == 4){//no answer
		return OK;
	} 
	switch (bRxdCode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		bTxdCode = 0;
		break;
	case 8:
	case 9:
		bTxdCode = 11;
		break;
	default:
		bTxdCode = 15; //链路未完成
		break;
	}
	TxdFixFrame(0, bTxdCode); //发送固定帧
	return OK;
}

/**
 * @description:子站回应无一级二级数据 
 * @param {none} 
 * @return: 
 */
void Txd_NoData()
{
	BYTE bNum=0;
	BYTE bCode=0x09;

	TxdFixFrame(bNum, bCode);
} //无所请求数据帧

/**
 * @description: 子站回应无一级二级数据
 * @param {none} 
 * @return: 
 */
void Txd_NoData_E5()
{
	m_Txd.len = 0;
	m_Txd.buf[m_Txd.len++] = 0xE5;
	//发送报文
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 寻找一级用户数据
 * @param {type} 
 * @return: 
 */
int SearchClass1()
{
	if(RTU_INIT_FINISH == TRUE){
		return OK;
	}
	if(UF_CALL_ALL_CONFM == TRUE){//总召命令确认标志位
		return OK;
	}
	if(UF_CALL_ALL_YX == TRUE){//总召遥信标志位
		return OK;
	}
	if(UF_CALL_ALL_YC == TRUE){ //总召遥测标志位
		return OK;
	}
	if(UF_CALL_ALL_END == TRUE){//总召命令结束标志位
		return OK;
	}
	if(UF_DELAY_GET == TRUE){//时钟延时获得命令
		return OK;
	}	
	if(UF_YK_SELECT == TRUE){//遥控预置确认
		return OK;
	}
	if(UF_YK_CANCLE == TRUE){//遥控撤销确认
		return OK;
	}
	if(UF_YK_EXCU == TRUE){//遥控执行确认
		return OK;
	}
	if(UF_YK_EXCU_FINISH == TRUE){//遥控执行结束确认
		return OK;
	}
	if(UF_CHANGE_YX == TRUE){//变位遥信(单点soe)
		return OK;
	}
	return ERROR;
}
/**
 * @description:总召命令 激活确认/总召结束
 * @param {} 
 * @return: 
 */
void Txd_CallAllCmd(BYTE bReason)
{
	BYTE bType=0x64;
	BYTE bPRM=0,bCode=0x08,bNum=1;
	BYTE *pBuf;

	pBuf=m_Txd.buf;
	TxdVarFrmHead(bType,bReason);//可变长帧头部

	pBuf[m_Txd.len++] = 0; //信息体地址Lo
	pBuf[m_Txd.len++] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}

	pBuf[m_Txd.len++] = 0x14; //总召唤限定词
	TxdVarFrmTail(bPRM,bCode,bNum); //可变长帧尾部
	TransData(m_Txd.buf, m_Txd.len);//发送报文
}
/**
 * @description:上送站召归一化遥测数据
 * @param {num:遥测个数;bReason:站召唤14/组召唤1d;bType:0x09/0x15,0x09:带品质,0x15不带品质} 
 * @return: 
 */
void Txd_CallYcData(BYTE num,BYTE bReason,BYTE bType)
{
	BYTE bPRM=0,bCode=0x08,bNum;
	BYTE *pBuf;

	pBuf=m_Txd.buf;
	bNum = 0x80 | num;//顺序信息体,即一个信息体地址带多个遥测数据
	TxdVarFrmHead(bType, bReason); //0x64站召命令，0x14响应总召唤

	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.ycStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.ycStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}
	for(BYTE cnt=0;cnt<num;cnt++){
		pBuf[m_Txd.len++] = LOBYTE(YcData[cnt]);
		pBuf[m_Txd.len++] = HIBYTE(YcData[cnt]);
		if(bType == 0x09){
			pBuf[m_Txd.len++] = 0x00;
		}
	}
	TxdVarFrmTail(bPRM, bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 上送总召唤短浮点遥测
 * @param {num:遥测个数;bReason:响应总召唤0x14,组召唤0x1d} 
 * @return: 
 */
void Txd_callYcFloat(BYTE num,BYTE bReason)
{
	BYTE bType=0x0d;
	BYTE bPRM=0,bCode=0x08,bNum;
	BYTE *pBuf;

	pBuf=m_Txd.buf;
	bNum = 0x80 | num; //SQ=1,顺序传输
	TxdVarFrmHead(bType, bReason); //0x64总召命令，0x14响应站召唤

	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.ycStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.ycStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}
	for(BYTE cnt=0;cnt<num;cnt++){ 
		yc_date.dt_float = YcDataFloat[cnt];
		pBuf[m_Txd.len++] = yc_date.buf[0];
		pBuf[m_Txd.len++] = yc_date.buf[1];
		pBuf[m_Txd.len++] = yc_date.buf[2];
		pBuf[m_Txd.len++] = yc_date.buf[3];
	}
	TxdVarFrmTail(bPRM, bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description:上送单点遥信数据不带品质描述词 ,一个字节表示一个遥信状态
 * @param {num:遥信个数;bReason:响应总召唤0x14,组召唤0x15} 
 * @return: 
 */
void Txd_CallYxData(BYTE num,BYTE bReason)
{
	BYTE bType=0x01;
	BYTE bPRM=0,bCode=0x08,bNum;
	BYTE *pBuf;

	pBuf=m_Txd.buf;
	bNum = num | 0x80;//SQ=1,顺序传输
	TxdVarFrmHead(bType, bReason); //0x01单点遥信,不带品质描述词

	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.yxStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.yxStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}
	for(BYTE cnt=0;cnt<num;cnt++){
		pBuf[m_Txd.len++] = YxData[cnt];
	}
	TxdVarFrmTail(bPRM, bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 组召唤命令 确认/结束
 * @param {bReason:确认0x07,结束0x0a;QOI:第1组0x15,第9组0x1d} 
 * @return: 
 */
void Txd_callGroupCmd(BYTE bReason,BYTE QOI)
{
	BYTE bType=0x64;
	BYTE bPRM=0,bCode=0x08,bNum=1;
	BYTE *pBuf;

	pBuf=m_Txd.buf;
	TxdVarFrmHead(bType, bReason);	 //0x64总召命令，0x0A总召激活结束

	pBuf[m_Txd.len++] = 0; //信息体地址Lo
	pBuf[m_Txd.len++] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}
	pBuf[m_Txd.len++] = QOI; //召唤QOI
	TxdVarFrmTail(bPRM, bCode,bNum); //信息体内容长度为1，信息体地址为0x0000，信息体内容指针
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 延时获得命令,用于校正链路传输延时
 * @param {type} 
 * @return: 
 */
void Txd_delay_get()
{
	BYTE bType=0x6a,bReason=0X07;//0x6a延时获得，0x07确认激活
	BYTE bPRM=0,bCode=0x08,bNum=1;
	BYTE *pBuf;
	WORD msec;
	struct timeval time;
	struct tm *date;

	pBuf=m_Txd.buf;
	TxdVarFrmHead(bType, bReason);	 

	pBuf[m_Txd.len++] = 0; //信息体地址Lo
	pBuf[m_Txd.len++] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}
	gettimeofday(&time,NULL);   //获取系统时间戳
	date = gmtime(&time.tv_sec);//将秒数转化为日期格式
	msec=date->tm_sec*1000 + time.tv_usec/1000;//获取毫秒
	pBuf[m_Txd.len++] = LOBYTE(msec);
	pBuf[m_Txd.len++] = HIBYTE(msec);
	TxdVarFrmTail(bPRM, bCode,bNum); //信息体内容长度为1，信息体地址为0x0000，信息体内容指针
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description:对时确认帧
 * @param {type} 
 * @return: 
 */
void TxdClockSyn()
{
	BYTE btype = 0x67, bReason = 7;
	BYTE bPRM = 0,bCode = 00,bNum = 1;
	BYTE* pBuf;
	
	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);

	pBuf[m_Txd.len++] = 0; //信息体地址Lo
	pBuf[m_Txd.len++] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0;
	}

	TimeScaleCp56(pBuf);//填充7字节时标

	TxdVarFrmTail(bPRM,bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description:遥控选择/撤销/执行报文
 * @param {type} 
 * @return: 
 */
void Txd_YkRespondCmd(BYTE ykCmd,BYTE bReason)
{
	BYTE btype = 0x2d;//bReason = 7,确认激活 bReason = 9,取消激活
	BYTE bPRM = 0,bCode = 0x08,bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);

	pBuf[m_Txd.len++] =LOBYTE(yk_argu.addr); //信息体地址Lo
	pBuf[m_Txd.len++] =HIBYTE(yk_argu.addr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0;
	}

	pBuf[ m_Txd.len++ ] = ykCmd; // 遥控限定词
	TxdVarFrmTail(bPRM,bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 发送子站初始化结束帧,配电终端就地初始化或配电终端远方初始化需要发送此帧
 * @param {type} 
 * @return: 
 */
void TxdRtuInitFinish()
{
	BYTE btype = 0x46,bReason = 4;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;
	
	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype, bReason);
	pBuf[m_Txd.len++] = 0; //信息体地址Lo
	pBuf[m_Txd.len++] = 0; //信息体地址Hi
	if(m_pCfg.bAsduAddrLen == 3){
		pBuf[m_Txd.len++] = 0;
	}
	pBuf[ m_Txd.len++ ] = 0; //COI 0:当地电源合上,1:当地手动复位,2:远方复位
	TxdVarFrmTail(bPRM,bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 发送测试链路帧
 * @param {type} 
 * @return: 
 */
void TxdTestLink()
{
	BYTE btype = 0x68,bReason = 0x07;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);

	pBuf[ m_Txd.len++ ] = 0; //信息体地址Lo
	pBuf[ m_Txd.len++ ] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[ m_Txd.len++ ] = 0;
	}
	
	pBuf[ m_Txd.len++ ] = 0xAA; 
	pBuf[ m_Txd.len++ ] = 0x55; 

	TxdVarFrmTail(bPRM,bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);
}
/**
 * @description: 复位进程,品质描述词:0:不使用,1:总复位,2:复位事件缓冲区滞留的带时标信息.
 * @param {quality:品质描述词} 
 * @return: 
 */
void TxdResetProcess(BYTE quality)
{
	BYTE btype = 0x69;
	BYTE bReason = 0x07;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);

	pBuf[ m_Txd.len++ ] = 0; //信息体地址Lo
	pBuf[ m_Txd.len++ ] = 0; //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[ m_Txd.len++ ] = 0;
	}
	pBuf[m_Txd.len++] = quality;//限定词,进程总复位
	TxdVarFrmTail(bPRM,bCode,bNum);
	TransData(m_Txd.buf, m_Txd.len);//回复成功
	if(quality == 1){//复位
		logMsg(logInfo,"reset device!");
		system("reboot");
	}
	else if(quality == 2){//清除带时标缓冲
		/**********************
		 * 清除带时标信息
		 * *********************/
		logMsg(logInfo,"clear timescale in information.");
	}
	else{
		logMsg(logInfo,"no reset process.");
	}
}
/**
 * @description: 发送不带品质描述词的归一化变化遥测
 * @param {type} 
 * @return: 
 */
void Txd_changeYc15()
{
	BYTE btype = 0x22,bReason = 0x03;//传输原因突发
	BYTE bPRM = 0, bCode = 8, bNum = 1;//SQ=0,单个传输,每个遥测值带有一个信息体地址,一个信息体
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);
	/******************************/
	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.ycStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.ycStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}  
	
	pBuf[m_Txd.len++] = YcData[0]; 
	/*****************************/
	TxdVarFrmTail(bPRM,bCode,bNum);
}
/**
 * @description: 发送带长时标的归一化变化遥测
 * @param {type} 
 * @return: 
 */
void Txd_changeYc22()
{
	BYTE btype = 0x22,bReason = 0x03;//传输原因突发
	BYTE bPRM = 0, bCode = 8, bNum = 1;//SQ=0,单个传输,每个遥测值带有一个信息体地址,一个信息体
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);
	/******************************/
	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.ycStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.ycStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}  
	
	pBuf[m_Txd.len++] = YcData[0]; 
	/******************************/
	TimeScaleCp56(pBuf);//填充时标
	TxdVarFrmTail(bPRM,bCode,bNum);
}
/**
 * @description: 发送带长时标的短浮点数变化遥测
 * @param {type} 
 * @return: 
 */
void Txd_changeYc24()
{
	BYTE btype = 0x24,bReason = 0x03;//传输原因突发
	BYTE bPRM = 0, bCode = 8, bNum = 1;//SQ=0,单个传输,每个遥测值带有一个信息体地址,一个信息体
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);
	/******************************/
	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.ycStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.ycStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}  
	yc_date.dt_float = YcDataFloat[0];
	pBuf[m_Txd.len++] = yc_date.buf[0]; 
	pBuf[m_Txd.len++] = yc_date.buf[1]; 
	pBuf[m_Txd.len++] = yc_date.buf[2]; 
	pBuf[m_Txd.len++] = yc_date.buf[3]; 
	/******************************/
	TimeScaleCp56(pBuf);//填充时标
	TxdVarFrmTail(bPRM,bCode,bNum);
}
/**
 * @description: 发送不带时标的单点soe
 * @param {type} 
 * @return: 
 */
void Txd_changeSoeNoTime()
{
	BYTE btype = 0x01,bReason = 0x03;//传输原因突发
	BYTE bPRM = 0, bCode = 8, bNum = 1;//SQ=0,单个传输,每个遥测值带有一个信息体地址,一个信息体
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);
	//不带时标单点soe
	/******************************/ 
	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.yxStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.yxStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}  
	
	pBuf[m_Txd.len++] = YxData[0];  
	/*****************************/
	TxdVarFrmTail(bPRM,bCode,bNum);
}
/**
 * @description: 发送带长时标的单点soe
 * @param {type} 
 * @return: 
 */
void Txd_changeCp56Soe()
{
	BYTE btype = 0x1e,bReason = 0x03;//传输原因突发
	BYTE bPRM = 0, bCode = 8, bNum = 1;//SQ=0,单个传输,每个遥测值带有一个信息体地址,一个信息体
	BYTE* pBuf;

	pBuf = m_Txd.buf;
	TxdVarFrmHead(btype,bReason);
	//带长时标单点soe
	/******************************/
	pBuf[m_Txd.len++] = LOBYTE(m_pCfg.yxStartAddr); //信息体地址Lo
	pBuf[m_Txd.len++] = HIBYTE(m_pCfg.yxStartAddr); //信息体地址Hi
	if(m_pCfg.bInfoAddrLen == 3){
		pBuf[m_Txd.len++] = 0; 
	}  
	
	pBuf[m_Txd.len++] = YxData[0];  
	 /****************************/
	TimeScaleCp56(pBuf);//填充时标
	TxdVarFrmTail(bPRM,bCode,bNum);
}
/**
 * @description: 填充7字节时标
 * @param {type} 
 * @return: 
 */
void TimeScaleCp56(BYTE *pBuf)
{
	WORD msec;
	struct tm *date;
	struct timeval time;

	gettimeofday(&time,NULL);//获取系统时间
	date = gmtime(&time.tv_sec);//将秒数时间戳转化为日期格式时间
	msec = date->tm_sec*1000 + time.tv_usec/1000;//提取毫秒
	pBuf[m_Txd.len++] = LOBYTE(msec);
	pBuf[m_Txd.len++] = HIBYTE(msec);
	pBuf[m_Txd.len++] = date->tm_min;
	pBuf[m_Txd.len++] = date->tm_hour+8;//北京时间 = 世界时 + 8
	pBuf[m_Txd.len++] = date->tm_mday;
	pBuf[m_Txd.len++] = date->tm_mon;
	pBuf[m_Txd.len++] = date->tm_year+1900-2000;
}
/**
 * @description:主站接收数据失败，重新发送 
 * @param {none} 
 * @return: RTUE:
 */
void TxdRetry()
{
	TransData(m_Txd.buf, m_Txd.len);
}
