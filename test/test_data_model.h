////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include "../opncms/ioc.h"
#include <iostream>
#include <map>

class IData
{
public:
	virtual void set(int /*a*/) = 0;
	virtual int get() = 0;
};

class Data
{
public:
	Data(){}
	~Data(){}
	virtual void set(int a){
		std::cerr << "Data set: " << a << std::endl;
	}
	IData& driver(const std::string& driver, IData& driverclass)
	{
		ioc::add<IData>(driverclass, std::string("Data:")+driver);
		return driverclass;
	}
	IData& driver(const std::string& driver, IData& driverclass, int param)
	{
		ioc::add<IData>(driverclass, std::string("Data:")+driver);
		param_ = param;
		return driverclass;
	}
	IData& driver(const std::string& driver)
	{
		return ioc::get<IData>(std::string("Data:")+driver);
	}
private:
	int param_;
};
