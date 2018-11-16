////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include "test_data_model.h"

class DataDriver : public IData
{
public:
	virtual int get()
	{
		return 2;
	}
	virtual void set(int a)
	{
		std::cerr << "DataDriver a: " << a << std::endl;
	}
};

std::map< std::string, void* > ioc::iocmap_;

int main()
{
	Data d;
	ioc::add<Data>(d);
	DataDriver dd;
	int param = 2;
	ioc::get<Data>().driver("driver",dd, param);
	ioc::get<Data>().driver("driver").set(3);
}
