////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_IOC_H
#define OPNCMS_IOC_H

#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <stdexcept>

class ioc
{
public:
	//WARN: typeid().name() is platform-dependent, rely on it very carefully
	template<class I>
	static void add(const I& obj, const std::string& id = typeid(I).name())
	{
		try
		{
			ioct::iterator it = iocmap_.find(id);
			if(it != iocmap_.end()) {
				if(iocmap_[id] != (void*)&obj)
					return;
			}
			iocmap_[id] = (void*)&obj;
		}
		catch(std::exception const& e) {
			throw std::runtime_error(std::string("Error while adding instance of id ")+id);
		}
	}

	template <class I>
	static I& get(const std::string& id = typeid(I).name())
	{
#ifdef _OPNCMS_DEBUG_
		std::cerr << "(";
		for(ioct::iterator it = iocmap_.begin(); it != iocmap_.end(); it++){
			std::cerr << "[" << it->first << "] ";
		}
		std::cerr << ")" << std::endl;
#endif
		ioct::iterator it = iocmap_.find(id);
		if(it == iocmap_.end()) {
			throw std::runtime_error(std::string("Can`t find instance with id ")+id);
		}
		return *((I*)it->second);
	}

	template <class I>
	static void del(const std::string& id = typeid(I).name())
	{
		ioct::iterator it = iocmap_.find(id);
		if(it == iocmap_.end()) {
			throw std::runtime_error(std::string("Can`t find instance with id ")+id);
		}
		delete ((I*)it->second);
	}

	static void clear()
	{
		iocmap_.clear();
	}

private:
	typedef std::map< std::string, void* > ioct;
	static std::map< std::string, void* > iocmap_;
};

#endif
