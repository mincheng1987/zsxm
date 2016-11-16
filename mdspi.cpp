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
	//cerr<<" ��Ӧ | �����ж�..." 
	//	<< " reason=" << nReason << endl;
	m_pBus->SyncSendReq<void, int>(nReason, "CQuoteDataCenter::OnFrontDisconnected");
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	//cerr<<" ��Ӧ | ������ʱ����..." 
	//	<< " TimerLapse = " << nTimeLapse << endl;
	m_pBus->SyncSendReq<void, int>(nTimeLapse, "CQuoteDataCenter::OnHeartBeatWarning");
}

void CtpMdSpi::OnFrontConnected()
{
	//cerr<<"MD ���ӽ���ǰ��OnFrontConnected()...�ɹ�"<<endl;
	//
	////��¼�ڻ��˺�
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
	cerr<<"MD ���� | ���͵�¼..."<<((ret == 0) ? "�ɹ�" :"ʧ��") << endl;	
	SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		//cerr<<"---"<<"�������Ϊ0��ʾ�ɹ���"<<pRspInfo->ErrorID<<",������Ϣ:"<<pRspInfo->ErrorMsg<<endl;

		//SubscribeMarketData_all();//����ȫ�г�����

		//SubscribeMarketData(m_instId);//���Ľ��׺�Լ������

		////���ĳֲֺ�Լ������
		//if(m_charNewIdList_holding_md)
		//{	
		//	cerr<<"m_charNewIdList_holding_md��С:"<<strlen(m_charNewIdList_holding_md)<<","<<m_charNewIdList_holding_md<<endl;

		//	cerr<<"�гֲ֣��������飺"<<endl;
		//	SubscribeMarketData(m_charNewIdList_holding_md);//����Ϊ6��/��,���û�гֲ֣��Ͳ�Ҫ����

		//	delete []m_charNewIdList_holding_md;//������ɣ��ͷ��ڴ�
		//}
		//else
		//	cerr<<"��ǰû�гֲ�"<<endl;


		//����������Ĭ�Ͻ�ֹ�����Ǹ��õķ��ϰ��
		//cerr<<endl<<endl<<endl<<"����Ĭ�Ͻ�ֹ���֣����������ף�������ָ��(������:yxkc, ��ֹ����:jzkc)��"<<endl;
		

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
	cerr<<" ���� | �������鶩��... "<<((ret == 0) ? "�ɹ�" : "ʧ��")<< endl;
	SetEvent(g_hEvent);
}



//����ȫ�г�����
void CtpMdSpi::SubscribeMarketData_all()
{
	SubscribeMarketData(m_charNewIdList_all);
	delete []m_charNewIdList_all;
}




void CtpMdSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr<<" ��Ӧ |  ���鶩��...�ɹ�"<<endl;

	//if(bIsLast)  SetEvent(g_hEvent);
	m_pBus->SyncSendReq<void, CThostFtdcSpecificInstrumentField *, CThostFtdcRspInfoField *, int, bool>
		(pSpecificInstrument, pRspInfo, nRequestID, bIsLast, "CQuoteDataCenter::OnRspSubMarketData");
}



void CtpMdSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr<<" ��Ӧ |  ����ȡ������...�ɹ�"<<endl;
	//if(bIsLast)  SetEvent(g_hEvent);
	m_pBus->SyncSendReq<void, CThostFtdcSpecificInstrumentField *, CThostFtdcRspInfoField *, int, bool>
						(pSpecificInstrument,pRspInfo,nRequestID,bIsLast,"CQuoteDataCenter::OnRspUnSubMarketData");
}


//�������ݵ�txt��csv
void SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//�������鵽txt
	string date = pDepthMarketData->TradingDay;
	string time = pDepthMarketData->UpdateTime;
	double open = pDepthMarketData->OpenPrice;
	int hight = pDepthMarketData->HighestPrice;
	double low = pDepthMarketData->LowestPrice;



	string instId = pDepthMarketData->InstrumentID;

	//����date�ĸ�ʽ
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
		cerr<<" ��Ӧ | "<<pRspInfo->ErrorMsg<<endl;
	}
	return ret;
}






void CtpMdSpi::setAccount(TThostFtdcBrokerIDType	appId1,	TThostFtdcUserIDType	userId1,	TThostFtdcPasswordType	passwd1)
{
	strcpy(m_appId, appId1);
	strcpy(m_userId, userId1);
	strcpy(m_passwd, passwd1);
}





//���ý��׵ĺ�Լ����
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

	cerr<<"�гֲֵĺ�Լ:"<<strlen(m_charNewIdList_holding_md)<<","<<sizeof(m_charNewIdList_holding_md)<<","<<_msize(m_charNewIdList_holding_md)<<endl<<m_charNewIdList_holding_md<<endl;

}



//����ȫ�г���Լ����TD����
void CtpMdSpi::set_instIdList_all(std::string instIdList_all)
{	
	int sizeIdList_all = instIdList_all.size();

	m_charNewIdList_all = new char[sizeIdList_all+1];

	memset(m_charNewIdList_all,0,sizeof(char)*(sizeIdList_all+1));

	strcpy(m_charNewIdList_all, instIdList_all.c_str());

	/*strcpy(m_instIdList_all, instIdList_all.c_str());*/

	if(!m_charNewIdList_all)//��strlenʱm_charNewIdList_all����Ϊ��
		cerr<<"�յ���ȫ�г���Լ:"<<strlen(m_charNewIdList_all)<<","<<sizeof(m_charNewIdList_all)<<","<<_msize(m_charNewIdList_all)<<endl<<m_charNewIdList_all<<endl;

}