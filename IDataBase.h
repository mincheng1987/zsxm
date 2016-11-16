#pragma once
#include "ThostFtdcUserApiDataTypeEx.h"
#include "ThostFtdcUserApiDataTypeEx.h"
#include "ThostFtdcUserApiStructEx.h"

//数据存储的表头，通过表头可以查看当前分类的总体信息
struct DbHead
{
	char obj[31];
	PeriodType type;
	long long count = 0;
	time_t begin = 0;
	time_t end = 0;	
	DbHead()
	{
		memset(obj, 0, 31);

	}
};

namespace at_datacenter
{
class IDataBase
{
public:
	virtual ~IDataBase(){};

	virtual bool Init() = 0;

	virtual bool Uninit() = 0;

	virtual bool GetKlines(Klines& data) = 0;

	virtual bool SetKlines(const Klines& data) = 0;

	virtual bool DeleteKlines(const Klines& data) = 0;

	virtual bool GetKline(const std::string& obj, PeriodType period,long long time, KlineElem& data) = 0;

	virtual bool SetKline(const std::string& obj, PeriodType period, long long time, const KlineElem& data) = 0;

	virtual bool DeleteKline(const std::string& obj, PeriodType period, long long time) = 0;

	virtual bool GetDepthDatum(DepthDatum& data) = 0;

	virtual bool SetDepthDatum(const DepthDatum& data) = 0;

	virtual bool DeleteDepthDatum(const DepthDatum& data) = 0;

	virtual bool GetDepthData(const std::string& obj, long long time, DepthElem& data) = 0;

	virtual bool SetDepthData(const std::string& obj, long long time, DepthElem& data) = 0;

	virtual bool DeleteDepthData(const std::string& obj, long long time) = 0;
};
}
