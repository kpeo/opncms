////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_RPC_H
#define OPNCMS_RPC_H

#include <cppcms/rpc_json.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_file.h>
#include <cppcms/http_context.h>
#include <cppcms/session_interface.h>

#if !(__cplusplus>=201103L)
#include <boost/assign/list_of.hpp>
#endif

///
/// \cond internal
///

#ifdef USE_STD_TR1_BIND
  #include <tr1/functional>
  using std::tr1::bind;
#elif defined USE_STD_BIND
  #include <functional>
  using std::bind;
#else
  #include <boost/bind.hpp>
  using boost::bind;
#endif

namespace apps {

class site;

class rpc : public cppcms::rpc::json_rpc_server
{
public:
	rpc(cppcms::service &, cppcms::application &);

	void health();
	void methods();
	void upload(std::string);
	void rss();
	void set_locale(std::string);
protected:
	cppcms::application& app_;
	cppcms::json::array methods_;
	cppcms::json::value items_;
};

}

///
/// \endcond
///

#endif
