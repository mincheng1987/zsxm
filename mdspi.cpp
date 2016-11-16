#include "stdafx.h"
#include <fstream>
#include <string>
#include <sstream>
#include "traderspi.h"
#include "mdspi.h"
#include "windows.h"


#pragma warning(disable : 4996)

int requestId;  
HANDLE g_hEvent;



void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	//IsErrorRspInfo(pRspInfo);
	m_pBus->SyncSendReq<void, CThostFtdcRspInfoField*, int, bool>(pRspInfo, nRequestID, bIsLast, "CQuoteDataCenter::OnRspError");
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	//cerr<<" 响应 | 连接中断..." 
	//	<< " reason=" << nReason << endl;
	m_pBus->SyncSendReq<void, int>(nReason, "CQuoteDataCenter::OnFrontDisconnected");
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	//cerr<<" 响应 | 心跳超时警告..." 
	//	<< " TimerLapse = " << nTimeLapse << endl;
	m_pBus->SyncSendReq<void, int>(nTimeLapse, "CQuoteDataCenter::OnHeartBeatWarning");
}

void CtpMdSpi::OnFrontConnected()
{
	//cerr<<"MD 连接交易前置OnFrontConnected()...成功"<<endl;
	//
	////登录期货账号
	//ReqUserLogin(m_appId, m_userId, m_passwd);

	//SetEvent(g_hEvent);
	m_pBus->SyncSendReq<void>("CQuoteDataCenter::OnFrontConnected");
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId,
	TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = m_pUserApi_md->ReqUserLogin(&req, ++requestId);
	cerr<<"MD 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;	
	SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		//cerr<<"---"<<"错误代码为0表示成功："<<pRspInfo->ErrorID<<",错误信息:"<<pRspInfo->ErrorMsg<<endl;

		//SubscribeMarketData_all();//订阅全市场行情

		//SubscribeMarketData(m_instId);//订阅交易合约的行情

		////订阅持仓合约的行情
		//if(m_charNewIdList_holding_md)
		//{	
		//	cerr<<"m_charNewIdList_holding_md大小:"<<strlen(m_charNewIdList_holding_md)<<","<<m_charNewIdList_holding_md<<endl;

		//	cerr<<"有持仓，订阅行情："<<endl;
		//	SubscribeMarketData(m_charNewIdList_holding_md);//流控为6笔/秒,如果没有持仓，就不要订阅

		//	delete []m_charNewIdList_holding_md;//订阅完成，释放内存
		//}
		//else
		//	cerr<<"当前没有持仓"<<endl;


		//策略启动后默认禁止开仓是个好的风控习惯
		//cerr<<endl<<endl<<endl<<"策略默认禁止开仓，如需允许交易，请输入指令(允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
		

	}
	if(bIsLast) SetEvent(g_hEvent);

	m_pBus->SyncSendReq<void, CThostFtdcRspUserLoginField *, CThostFtdcRspInfoField *, int, bool>
		               (pRspUserLogin,pRspInfo,nRequestID,bIsLast,"CQuoteDataCenter::OnRspUserLogin");
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
	vector<char*> list;
	char *token = strtok(instIdList, ",");
	while( token != NULL ){
		list.push_back(token); 
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** pInstId = new char* [len];  
	for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i]; 
	int ret=m_pUserApi_md->SubscribeMarketData(pInstId, len);
	cerr<<" 请求 | 发送行情订阅... "<<((ret == 0) ? "成功" : "失败")<< endl;
	SetEvent(g_hEvent);
}



//订阅全市场行情
void CtpMdSpi::SubscribeMarketData_all()
{
	SubscribeMarketData(m_charNewIdList_all);
	delete []m_charNewIdList_all;
}




void CtpMdSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr<<" 响应 |  行情订阅...成功"<<endl;

	//if(bIsLast)  SetEvent(g_hEvent);
	m_pBus->SyncSendReq<void, CThostFtdcSpecificInstrumentField *, CThostFtdcRspInfoField *, int, bool>
		(pSpecificInstrument, pRspInfo, nRequestID, bIsLast, "CQuoteDataCenter::OnRspSubMarketData");
}



void CtpMdSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr<<" 响应 |  行情取消订阅...成功"<<endl;
	//if(bIsLast)  SetEvent(g_hEvent);
	m_pBus->SyncSendReq<void, CThostFtdcSpecificInstrumentField *, CThostFtdcRspInfoField *, int, bool>
						(pSpecificInstrument,pRspInfo,nRequestID,bIsLast,"CQuoteDataCenter::OnRspUnSubMarketData");
}


//保存数据到txt和csv
void SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//保存行情到txt
	string date = pDepthMarketData->TradingDay;
	string time = pDepthMarketData->UpdateTime;
	double open = pDepthMarketData->OpenPrice;
	int hight = pDepthMarketData->HighestPrice;
	double low = pDepthMarketData->LowestPrice;



	string instId = pDepthMarketData->InstrumentID;

	//更改date的格式
	string a = date.substr(0, 4);
	string b = date.substr(4, 2);
	string c = date.substr(6, 2);

	string date_new = a + "-" + b + "-" + c;


	ofstream fout_data("../output/depthdata" + instId + "_" + date + ".txt", ios::app);

	fout_data << date_new << "," << time << "," << open << "," << hight << "," << low << endl;

	fout_data.close();

}
void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//SaveDataTxtCsv(pDepthMarketData);
		//g_strategy->OnTickData(pDepthMarketData);
	m_pBus->SyncSendReq<void, CThostFtdcDepthMarketDataField *>(pDepthMarketData, "CQuoteDataCenter::OnRtnDepthMarketData");

}





bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		cerr<<" 响应 | "<<pRspInfo->ErrorMsg<<endl;
	}
	return ret;
}






void CtpMdSpi::setAccount(TThostFtdcBrokerIDType	appId1,	TThostFtdcUserIDType	userId1,	TThostFtdcPasswordType	passwd1)
{
	strcpy(m_appId, appId1);
	strcpy(m_userId, userId1);
	strcpy(m_passwd, passwd1);
}





//设置交易的合约代码
void CtpMdSpi::setInstId(std::string instId)
{
	strcpy(m_instId, instId.c_str());
}





void CtpMdSpi::setInstIdList_holding_md(std::string instId)
{
	//strcpy(m_instIdList_holding_md, instId.c_str());
	int sizeInstId = instId.size();

	m_charNewIdList_holding_md = new char[sizeInstId+1];

	memset(m_charNewIdList_holding_md,0,sizeof(char)*(sizeInstId+1));

	strcpy(m_charNewIdList_holding_md, instId.c_str());

	/*strcpy(m_instIdList_all, instIdList_all.c_str());*/

	cerr<<"有持仓的合约:"<<strlen(m_charNewIdList_holding_md)<<","<<sizeof(m_charNewIdList_holding_md)<<","<<_msize(m_charNewIdList_holding_md)<<endl<<m_charNewIdList_holding_md<<endl;

}



//保存全市场合约，在TD进行
void CtpMdSpi::set_instIdList_all(std::string instIdList_all)
{	
	int sizeIdList_all = instIdList_all.size();

	m_charNewIdList_all = new char[sizeIdList_all+1];

	memset(m_charNewIdList_all,0,sizeof(char)*(sizeIdList_all+1));

	strcpy(m_charNewIdList_all, instIdList_all.c_str());

	/*strcpy(m_instIdList_all, instIdList_all.c_str());*/

	if(!m_charNewIdList_all)//用strlen时m_charNewIdList_all不能为空
		cerr<<"收到的全市场合约:"<<strlen(m_charNewIdList_all)<<","<<sizeof(m_charNewIdList_all)<<","<<_msize(m_charNewIdList_all)<<endl<<m_charNewIdList_all<<endl;

}