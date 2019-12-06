/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-22 05:56:33
 * @LastEditTime: 2019-12-05 22:55:41
 * @LastEditors: Please set LastEditors
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "siec101_2002.h"
/**
 * @description: 接收处理
 * @param {none} 
 * @return:
 */
int RxdMonitor()
{
    BYTE *pBuf;
    if (SearchFrame()!=OK){ //检索报文是否有效
		logMsg(logErr,"Searching frame error!");
        return ERROR;
	}
    pBuf= m_Rxd.buf;
    
	if(pBuf[0] == 0x10)
		return RxdFixFrame();  //固定帧结构处理 
	
	if(pBuf[0] == 0x68)
		return RxdVarFrame(); //可变帧长结构处理   

	return ERROR;
}

/**
 * @description:判断是否有效报文 
 * @param {none} 
 * @return:函数状态，返回TRUE表示报文有效，返回FALSE表示报文错误
 */
int SearchFrame()  
{
	BYTE *pBuf=m_Rxd.buf;   //获取报文
    WORD wFrameLen;
	BYTE bChkSum;
	BYTE bChkLen;
	WORD wAddr;
	
	switch(pBuf[0])  //报文结构选择 
	{
	case 0x10:  //固定帧结构 
		if(m_pCfg.bLinkAddrLen == 2)//链路层地址长度2
		{
			bChkLen = 3;//校验长度3
			wFrameLen = 6; //固定帧长度为6，1个字节启动字符，1个字节链路控制，2个字节链路地址，1个字节校验和，1个字节结束字符 
	
			if(pBuf[5] != 0x16){//判断结束符是否正确 
				logMsg(logErr,"End charater error !");
				return ERROR;	 
			}

			wAddr = MAKEWORD(pBuf[2], pBuf[3]); //获取两个字节的链路地址 
			if(wAddr != m_pCfg.wLinkAddr){//无效地址 
				logMsg(logErr,"Invalid linkAddress !");
				return ERROR;
			}

			bChkSum = ChkSum(pBuf+1, bChkLen); //求检验和 
			if(pBuf[4] != bChkSum){  //判断检验和是否正确
				logMsg(logErr,"check sum error !");
				return ERROR;	 
			}
		}
		else//链路层地址长度1
		{
			bChkLen = 2;  //校验长度2
			wFrameLen = 5; //帧长度5
			if(pBuf[4] != 0x16){
				logMsg(logErr,"End charater error!");
				return ERROR;	 
			}

			wAddr = (WORD)pBuf[2];
			if(wAddr != m_pCfg.wLinkAddr){  //无效地址
				logMsg(logErr,"invalid linkAddress");
				return ERROR;
			}

			bChkSum = ChkSum(pBuf+1, bChkLen); 
			if(pBuf[3] != bChkSum){ //校验和错误
				logMsg(logErr,"check sum error");
				return ERROR;	 
			} 
		}
		break;
	case 0x68:  //可变帧长帧结构 ，固定4字节报文头 
		if(pBuf[1] != pBuf[2]){ //两次发送数据帧长度字节不相等，则返回错误 
			logMsg(logErr,"frame lenth error!error code=%d",pBuf[1]);
			return ERROR;
		 }
		 if(pBuf[3] != 0x68){  //再次判断启动字符 
		 	logMsg(logErr,"The second start charater error!error code=%d",pBuf[3]);
			return ERROR;
		 }

		 wFrameLen = pBuf[1] + 6; //可变帧的总长度=长度域+6个固定报文头报文尾 
		 if(pBuf[wFrameLen-1] != 0x16){ //判断帧结束字符 
		 	logMsg(logErr,"End charater error!error code=%d",pBuf[wFrameLen-1]);
			return ERROR;
		 }

		 bChkSum = ChkSum(pBuf+4, pBuf[1]); //获取校验和 
		 if(pBuf[wFrameLen-2] != bChkSum){ //判断校验是否正确; 
		 	logMsg(logErr,"check sum error!error sum=%d",pBuf[wFrameLen-2]);
			return ERROR;
		 }
		 if(m_pCfg.bLinkAddrLen == 2)//链路层地址长度为2
			 wAddr = MAKEWORD(pBuf[5], pBuf[6]); //获取链路地址 
		 else{
			 wAddr = (WORD)pBuf[5];
		 } 
		 if(wAddr != m_pCfg.wLinkAddr){//无效地址
		 	logMsg(logErr,"invalid linkAddress = %d",wAddr);
			 return ERROR;
		 }
		break;
	default:
		logMsg(logErr,"not fixframe or varframe");
		return ERROR;
	}
	return OK;	
}
/**
 * @description:计算帧的校验和值
 * @param {校验和变量的起始地址，检验和变量的长度} 
 * @return: 校验和值
 */
BYTE ChkSum(BYTE* p_Addr,WORD chek_len)
{
	BYTE checkSum = 0;  
	BYTE *headAddr;
	headAddr=p_Addr;
	for(WORD i=0;i<chek_len;i++){
		checkSum += *headAddr;     //计算校验和，不考虑溢出
		headAddr++;
	}
	return checkSum;
}
/**
 * @description:固定帧结构处理 
 * @param {none} 
 * @return: 
 */
int RxdFixFrame()
{
	BYTE* pBuf;
	BYTE  bControl;

	pBuf = m_Rxd.buf;
	bControl = pBuf[1];

	SaveCtrlCode(bControl);
	
	if(CheckAndRetrans(bControl) != ERROR){//判断上一帧是否被对方正确接收
		return ERROR;
	}  
	switch(bControl & BIT_FUNC)
	{
		case 0x00: //0x00 复位远方链路
			RxdResetLink();  
			break; 
		case 0x08://相应,链路状态
		case 0x09: //0x09 请求远方链路状态
			RxdReqLinkStatus(); 
			break; 
		case 0x0A: //召唤一级用户数据
			RxdClass1Data(); 
			break; 
		case 0x0B: //远方链路状态完好或召唤二级用户数据
			RxdClass2Data();
			break; 
		default:
			FixAck();
		}
		return OK;
}
/**
 * @description:可变帧结构处理 
 * @param {none} 
 * @return:
 */
int RxdVarFrame() 
{
	BYTE* pBuf;
	BYTE bControl;
	BYTE bType;  //类型标识符
	WORD wAddress;
	BYTE* pData;

	pBuf = m_Rxd.buf;
	bControl = pBuf[4]; //链路控制域 
	SaveCtrlCode(bControl);//保存控制字
	if(UF_LINK_INIT_OK == FALSE){//没有初始化链路
		logMsg(logErr,"no initial link.");
		return ERROR;
	} 
	if(CheckAndRetrans(bControl) != ERROR){//重发数据
		return ERROR;
	}

	pData = pBuf+9;  //应用服务单元公共地址，默认前面属性1字节
	if(m_pCfg.bLinkAddrLen == 2)  //链路地址2字节
	{
		bType = pBuf[7];  
		pData++;
	}
	else{
		bType = pBuf[6];
	}
	if(m_pCfg.bCauseTransLen == 2){//传输原因2字节
		pData ++;
	} 	
	if(m_pCfg.bAsduAddrLen == 2){//公共地址2字节
		wAddress = MAKEWORD(pData[0],pData[1]);
	}  
	else{
		wAddress = pData[0];
	}
	if(wAddress != m_pCfg.wAsduAddr){//无效公共地址
		return ERROR;
	}
	switch(bType)
	{
	case 45://单点遥控0x2d
		RxdYkCmd();
		break;
	case 0x64:  //总召唤
		RxdStationCall(); 
		break;
	case 0x67:  //时钟同步
		RxdClockSyn(); 
		break;
	case 0x68:  //测试链路
		RxdTestLink();
		break;
	case 0x69://复位RTU
		ResetProcess();
		break;
	case 0x6a://延时命令
		RxdDelay();
		break;
	default:
		logMsg(logErr,"frame type error!");	
	}
	return OK;
}
/**
 * @description:请求远方链路状态 
 * @param {none} 
 * @return:
 */
void RxdReqLinkStatus()
{
	RESET_ALL_USER_FLAG(userFlag);//将所有标志位清零
	TxdFixFrame(0, 0x0B);//回复链路完好
	SET_USER_FLAG(RTU_INIT_FINISH);
}
/**
 * @description: 0x00 复位远方链路
 * @param {none} 
 * @return: 
 */
void RxdResetLink()
{
	BYTE *pBuf;
	BYTE bControl;

	pBuf = m_Rxd.buf;
	bControl = pBuf[1];
	
	if(bControl & BIT_PRM){
		SET_USER_FLAG(UF_LINK_INIT_OK); //生成初始化结束事件
	}
	FixAck();//回复链路复位确定
}
/**
 * @description:总召唤,召唤限定词(QOI),20:总召唤
 * @param {none} 
 * @return: TRUE
 */
void RxdStationCall()
{
	BYTE *pBuf;
	BYTE frmLen;
	BYTE QOI;
	pBuf = m_Rxd.buf;
	 
	frmLen = pBuf[1]+4;
	QOI = pBuf[frmLen-1];
	switch (QOI)
	{
	case 0x14://总召唤
		SET_USER_FLAG(UF_CALL_ALL_CONFM);//总召命令确认标志位
		SET_USER_FLAG(UF_CALL_ALL_YX);//总召遥信标志
		SET_USER_FLAG(UF_CALL_ALL_YC );//总召遥测标志
		SET_USER_FLAG(UF_CALL_ALL_END); //总召命令结束标志位
		break;
	case 0x15://第1组召唤(重要遥信)
		SET_USER_FLAG(UF_ONE_GROUP_CONFM);
		SET_USER_FLAG(UF_ONE_GROUP_DATA);
		SET_USER_FLAG(UF_ONE_GROUP_END);
		break;
	case 0x1d://第9组召唤(重要遥测)
		SET_USER_FLAG(UF_NINE_GROUP_CONFM);
		SET_USER_FLAG(UF_NINE_GROUP_DATA);
		SET_USER_FLAG(UF_NINE_GROUP_END);
		break;
	default:
		logMsg(logErr,"Station call QOI invalid!");
		break;
	}
	FixAck(); //链路确认
}
/**
 * @description: 测试链路
 * @param {type} 
 * @return: 
 */
void RxdTestLink()
{
	SET_USER_FLAG(UF_LINK_TEST);
	FixAck(); 
}
/**
 * @description: 复位进程,品质描述词:0:不使用,1:总复位,2:复位事件缓冲区滞留的带时标信息.
 * @param {reset_QPR:复位进程的品质描述词} 
 * @return: 
 */
void ResetProcess()
{
	BYTE * pBuf;
	BYTE len;

	pBuf = m_Rxd.buf;
	len = pBuf[1];//帧长度
	UF_RESET_QUALITY = pBuf[len+3];//获取复位进程品质描述词
	SET_USER_FLAG(UF_RESET_PROCESS);
	FixAck();
}
/**
 * @description:召唤一级用户数据 
 * @param {none} 
 * @return: 
 */
int RxdClass1Data()
{
	if(RTU_INIT_FINISH == TRUE){ //终端就地初始化或主站远方初始化时上送RTU初始化完成帧
		RESET_USER_FLAG(RTU_INIT_FINISH);
		TxdRtuInitFinish();
		return  OK;
	}
	if(UF_CALL_ALL_CONFM == TRUE){
		RESET_USER_FLAG(UF_CALL_ALL_CONFM);
		Txd_CallAllCmd(0x07);  //总召确认
		return OK;
	}
	if(UF_CALL_ALL_YX == TRUE){  //总召遥信
		RESET_USER_FLAG(UF_CALL_ALL_YX);
		Txd_CallYxData(YX_LENTH,0x14);  //总召YX上送,总召唤,不带品质描述词的单点遥信
		return OK;
	}
	if(UF_CALL_ALL_YC == TRUE){  //总召遥测
		RESET_USER_FLAG(UF_CALL_ALL_YC);
		Txd_CallYcData(YC_LENTH,0x14,0x15);  //总召YC上送,归一化值,不带品质描述
		//Txd_callYcFloat(YC_LENTH,0x14);  //短浮点遥测
		return OK;
	}
	if(UF_CALL_ALL_END == 1){
		RESET_USER_FLAG(UF_CALL_ALL_END);
		Txd_CallAllCmd(0x0a); //总召结束
		return OK;
	}
	if(UF_DELAY_GET == TRUE){//延时获得确认
		RESET_USER_FLAG(UF_DELAY_GET);
		Txd_delay_get();
		return OK;
	}
	if(UF_YK_SELECT == TRUE){//遥控选择确认
		RESET_USER_FLAG(UF_YK_SELECT);
		Txd_YkRespondCmd(yk_argu.cmd,7);
		return OK;
		
	}	
	if(UF_YK_CANCLE == TRUE){//遥控撤销
		RESET_USER_FLAG(UF_YK_CANCLE);
		Txd_YkRespondCmd(yk_argu.cmd,9);//bseason=9,停止激活确认
		return OK;

	}
	if(UF_YK_EXCU == TRUE){//遥控执行确认
		RESET_USER_FLAG(UF_YK_EXCU);
		Txd_YkRespondCmd(yk_argu.cmd,7);
		if(yk_argu.cmd == 0x00){
			/************************
			 * 遥控执行控分
			 * **********************/
		}
		else if(yk_argu.cmd == 0x01){
			/************************
			 * 遥控执行控合
		 	* **********************/
		}
		return OK;
	}
	if(UF_YK_EXCU_FINISH == TRUE){//遥控执行结束
		RESET_USER_FLAG(UF_YK_EXCU_FINISH);
		Txd_YkRespondCmd(yk_argu.cmd,0x0a);
		return OK;
	}
	if(UF_CHANGE_YX==TRUE){//上送不带时标变位遥信,单点soe,接着主站召唤二级用户数据,上送带长时标单点soe
		SET_USER_FLAG(UF_CHANGE_YX_CLASS2); 
		RESET_USER_FLAG(UF_CHANGE_YX);
		Txd_changeSoeNoTime();
		return OK;
	}
	Txd_NoData();
	return OK;
}
/**
 * @description:召唤二级用户数据 
 * @param {none} 
 * @return: 状态量
 */
int RxdClass2Data()//远方链路状态完好或召唤二级用户数据
{
	if(UF_CHANGE_YX_CLASS2 == TRUE){//上送二级soe(带长时标)
		RESET_USER_FLAG(UF_CHANGE_YX_CLASS2);
		Txd_changeCp56Soe();
		return OK;
	}
	if(UF_LINK_TEST == TRUE){ //测试链路
		RESET_USER_FLAG(UF_LINK_TEST);
		TxdTestLink();
		return OK;
	}
	if(UF_RESET_PROCESS == TRUE){//复位进程
		RESET_USER_FLAG(UF_RESET_PROCESS);
		TxdResetProcess(UF_RESET_QUALITY);
		return OK;
	}
	if(UF_CHANGE_YX == TRUE){ //有一级用户数据时,先上送一级用户数据
		FixAck();
		return OK;
	}
	//分组召唤可被一级用户数据打断(变位遥信)
	if(UF_ONE_GROUP_CONFM == TRUE){ //确认第1组召唤
		RESET_USER_FLAG(UF_ONE_GROUP_CONFM);
	    Txd_callGroupCmd(0x07,0x15);
		return OK;
	}
	if(UF_ONE_GROUP_DATA == TRUE){//上送第1组数据
		RESET_USER_FLAG(UF_ONE_GROUP_DATA);
		Txd_CallYxData(YX_LENTH,0x15);//召唤第1组
		return OK;
	}
	if(UF_ONE_GROUP_END == TRUE){//结束第1组召唤
		RESET_USER_FLAG(UF_ONE_GROUP_END);
		Txd_callGroupCmd(0x0a,0x15);
		return OK;
	}
		if(UF_NINE_GROUP_CONFM == TRUE){ //确认第9组召唤
		RESET_USER_FLAG(UF_NINE_GROUP_CONFM);
	    Txd_callGroupCmd(0x07,0x1d);
		return OK;
	}
	if(UF_NINE_GROUP_DATA == TRUE){//上送第9组数据
		RESET_USER_FLAG(UF_NINE_GROUP_DATA);
		Txd_CallYcData(YC_LENTH,0x1d,0x15);//召唤第9组
		return OK;
	}
	if(UF_NINE_GROUP_END == TRUE){//结束第9组召唤
		RESET_USER_FLAG(UF_NINE_GROUP_END);
		Txd_callGroupCmd(0x0a,0x1d);
		return OK;
	}
	if(UF_CHANGE_YC == TRUE){//上送二级用户数据,越限遥测
		RESET_USER_FLAG(UF_CHANGE_YC);
		Txd_changeYc22();//带长时标归一化遥测
		return OK;
	}
	Txd_NoData();//无用户数据
	return OK;
}
/**
 * @description: 延时获得
 * @param {type} 
 * @return: 
 */
void RxdDelay()
{
	BYTE *pBuf;
	BYTE bCause;

	pBuf =m_Rxd.buf+8;//定位到传输原因
	if(m_pCfg.bLinkAddrLen == 2){
		pBuf += 1;
	}
	bCause = pBuf[0];
	if(bCause == 0x06){//延时获得命令激活
		if(m_pCfg.bCauseTransLen == 2){//传输原因长度
			pBuf += 1;
		}
		pBuf += 1;
		if(m_pCfg.bAsduAddrLen == 2){//公共地址长度
			pBuf += 1;
		}
		pBuf += 2;
		if(m_pCfg.bInfoAddrLen == 3){//信息体地址长度
			pBuf += 1;
		}	
		pBuf += 1;
		clock_syn.msec_t1 = MAKEWORD(pBuf[0],pBuf[1]);//获取t1
		UpdateSysTime();//更新系统时间
		SET_USER_FLAG(UF_DELAY_GET);//延时获得
		FixAck(); //回复报文
	}
	else if(bCause == 0x03){//延时发送命令
		FixAck(); //回复报文
	}
	else{//传输原因错误
		logMsg(logErr,"cause of transmission error!");
	}
	
}
/**
 * @description:收到t1时刻时更新系统时间 
 * @param {type} 
 * @return: 
 */
void UpdateSysTime()
{
	struct timeval time;
	struct tm *date;

    //获取系统时间戳
    gettimeofday(&time,NULL);
	date = gmtime(&time.tv_sec);
	date->tm_sec = clock_syn.msec_t1/1000;//替换秒
	time.tv_sec = mktime(date);//换算为秒数
	//微妙
    time.tv_usec = (__suseconds_t)(clock_syn.msec_t1%1000*1000);
    //更新系统时间
    settimeofday(&time,NULL);
    //将系统时间同步到硬件时钟
    //system("hwclock -w");
}
/**
 * @description:时钟同步 
 * @param {type} 
 * @return: 
 */
int RxdClockSyn()//时钟同步
{
	BYTE* pBuf;
	BYTE* pData;
	WORD wMSecond;
	BYTE bReason;
	struct timeval time;
	struct tm tv;

	pBuf = m_Rxd.buf;
	pData = pBuf + 10; 

	if(m_pCfg.bLinkAddrLen == 2)
		bReason = pBuf[9];
	else
		bReason = pBuf[8];
	
	if(bReason != 6){
		logMsg(logErr,"causes of transmission error,error reason=%d",bReason);
		return ERROR;
	}

	if(m_pCfg.bLinkAddrLen == 2){
		pData += 1;
	}	
	if(m_pCfg.bAsduAddrLen== 2){
		pData += 1;
	}	
	if(m_pCfg.bCauseTransLen == 2){//传输原因长度
		pData += 1;
	}
	
	if(m_pCfg.bInfoAddrLen == 2){//信息体地址长度
		pData += 2;
	}	
	else if(m_pCfg.bInfoAddrLen == 3){
		pData += 3;
	}	
	else{
		pData += 1;
	}
	wMSecond = MAKEWORD(pData[0], pData[1]);
	time.tv_usec=(wMSecond%1000)*1000; //转换为微妙
	tv.tm_sec = (wMSecond/1000); //提取秒
	tv.tm_min = pData[2]&0x3F; //分占低6位
	tv.tm_hour = pData[3]&0x1F; //时占低5位
	tv.tm_mday = (pData[4]&0x1F);//日占低5位
	tv.tm_mon = (pData[5]&0x0F)-1; //月占低4位
	tv.tm_year = (pData[6]&0x7F)+2000-1900;//年占低7位//-1900
	time.tv_sec = mktime(&tv)-8*3600;//将日历时钟转换为秒计数,UTC=北京时间-8
	//设置时间
	settimeofday(&time,NULL);
	//system("hwclock -w");
	TxdClockSyn();//对时命令
	return OK;
}
/**
 * @description: 遥控处理
 * @param {type} 
 * @return: 
 */
int RxdYkCmd()//遥控处理
{
	BYTE bReason;
	BYTE* pData;
	BYTE* pBuf;

	pBuf = m_Rxd.buf;
	
	if(m_pCfg.bLinkAddrLen == 2){//两个字节的链路地址
		bReason = pBuf[9];//获取传输原因
	}
	else{
		bReason = pBuf[8];
	}
	//未知的传送原因 
	if(bReason != 6 && bReason != 8){
		FixAck();
		logMsg(logErr,"Yk unknown causes of transmission");
		return FALSE;
	}

	pData = pBuf + 10;//获取用户信息
	
	if(m_pCfg.bLinkAddrLen == 2){//链路地址长度
		pData += 1;
	}
	if(m_pCfg.bInfoAddrLen == 2){//公共地址长度
		pData += 1;
	}
	if(m_pCfg.bCauseTransLen== 2){//传送原因长度
		pData += 1;
	}
	
	yk_argu.addr = MAKEWORD(pData[0],pData[1]);
	if(m_pCfg.bInfoAddrLen == 3){//3个字节信息体地址,
		pData += 1;
	}
	if(yk_argu.addr < m_pCfg.ykStartAddr){//比较遥控地址
		return ERROR;
	}	
	yk_argu.cmd=pData[2];//遥控命令
	
	if(bReason==6){ //单点遥控命令激活
		switch((BYTE)(yk_argu.cmd & 0x83)){//遥控命令
		case 0x80: //预置控分
		case 0x81: //预置控合
			SET_USER_FLAG(UF_YK_SELECT); //遥控预置标志位
			break;
		case 0x00: //执行控分
		case 0x01: //执行控合
			SET_USER_FLAG(UF_YK_EXCU);//遥控执行标志
			SET_USER_FLAG(UF_YK_EXCU_FINISH);//执行结束标志
			break;
		default:
			logMsg(logErr,"yk command error!");
			return ERROR;
		}
	}
	else if(bReason==8){//撤销单点遥控命令
		SET_USER_FLAG(UF_YK_CANCLE);
	}
	else{
		return ERROR;
	}
	//回复确认帧
	FixAck();
	return OK;
}
/**
 * @description: 判断上一帧报文是否被对方正确接收
 * @param {type} 
 * @return: 
 */
int CheckAndRetrans(BYTE bControl)
{
	BYTE funcCode;
	BYTE bNowFcbFcv;

	funcCode = bControl & BIT_FUNC;//功能码
	bNowFcbFcv = bControl & BIT_FCBFCV; //本次FCBFCV
	if(funcCode==0 || funcCode==1){//复位帧计数，期待下一个FCB=1,FCV=1；
		UF_NEXT_FCBFCV = BIT_FCBFCV;//
		return ERROR;
	}
	if(!(bNowFcbFcv & BIT_FCV)){//本次FCV无效,不需要重发数据
		//UF_NEXT_FCBFCV = 0X00;
		return ERROR;
	}	
	if(UF_NEXT_FCBFCV != bNowFcbFcv){  //实际FCBFCV与预期的不一样，重发
		if(m_Txd.len > 4){   //判断报文是否正确
			TxdRetry();  //重发上一次数据帧
			return OK;
		}
	}
	UF_NEXT_FCBFCV=((~bNowFcbFcv) & BIT_FCB) | BIT_FCV; //下次接收的FCB位翻转，FCV位置1
	return ERROR;
}

/**
 * @description: 保存报文控制码
 * @param {} 
 * @return: none
 */
void SaveCtrlCode(BYTE funCode)
{
    UF_RXDCONTROL = funCode;   //保存本次控制码
}
