////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_SITE_H
#define OPNCMS_SITE_H

#include <iostream>
#include <map>
#include <booster/log.h>
#include <cppcms/application.h>
#include <cppcms/url_mapper.h>
#include <cppcms/json.h>
#include <opncms/ioc.h>

#include <opncms/module/data.h>
#include <opncms/module/auth.h>
#include <opncms/module/media.h>
#include <opncms/module/view.h>
#include <opncms/module/plugin.h>

#include <opncms/rpc.h>

///
/// \cond internal
///

//extern Plug plugins;

struct null_op {
      void operator()(const void *) {} // do-nothing operator()
};

namespace apps {

class site : public cppcms::application {
	friend class apps::rpc;

public:
	site(cppcms::service &, int, char**);
	~site();
	void display(std::string upath="home");
#ifdef _DEB_ENV
	virtual void main(std::string url);
#endif
private:
	int argc_;
	char** argv_;
	apps::rpc* rpc_;
	//WARN: using this will cause in segmentation fault
	//due to delete in CppCMS application destructor
	//std::auto_ptr<apps::rpc> rpc_; 

	Data db_;
	Auth ab_;
	Media mb_;
	View vb_;

	std::string upath;
	bool first_request_;

	template <typename T>
	void attach_plugin(T, tools::vec_str &);
	void attach_plugins(Plug &);
};

}

///
/// \endcond
///

#endif
