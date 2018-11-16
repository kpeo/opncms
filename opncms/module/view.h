////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_VIEW_H
#define OPNCMS_M_VIEW_H

#include <opncms/base.h>
#include <cppcms/localization.h>
#include <cppcms/xss.h>

#include <cppcms/http_file.h> //for ava upload
#include <booster/log.h> //for current log

#include <opncms/module/data.h>
#include <opncms/module/auth.h>
#include <opncms/module/media.h>
#include <cppcms/service.h>

#define _(X) ::cppcms::locale::translate(X)
#define N_(S,P,N)  ::cppcms::locale::translate(S,P,N)

namespace cppcms {  
namespace json {  
	template<>
	struct traits<page_t> {
	
	static void set(value &v, page_t const &in)
	{
		v.set("order_id", in.order_id);
		v.set("name", in.name);
		v.set("icon", in.icon);
		v.set("url", in.url);
		v.set("type", in.type);
		v.set("authed", in.authed);
		v.set("active", in.active);
		v.set("is_header", in.menu);
		v.set("data", in.data);
	}
	
	static page_t get(value const &v)
	{
		page_t p;
		if(v.type() != is_object)
			throw bad_value_cast();

		p.order_id = v.get<int>("order_id");
		p.name = v.get<std::string>("name");
		p.icon = v.get<std::string>("icon");
		p.url = v.get<std::string>("url");
		p.type = v.get<int>("type");
		p.authed = (v.get<int>("authed")) ? true : false;
		p.active = (v.get<int>("active")) ? true : false;
		p.menu = v.get<int>("menu");
		p.data = v.find("data");
		 
		return p;
	}
	};
}}

///
/// \brief View Manager - performs various tasks for preparing and rendering data of base template
/// \class View Manager - performs various tasks for preparing and rendering data of base template
///
class View : public cppcms::base_content
{

public:
	typedef std::map <int, std::pair<std::string,std::string> > menu_list;
	typedef std::map<std::string, menu_list > menu_t;
	///
	/// \brief Constructor
	///
	View(cppcms::application& /*app*/, cppcms::service& /*srv*/);
	///
	/// \brief Destructor
	///
	~View();
	///
	/// \brief Performs data initialiation for base view
	/// \param c content for base view
	///
	void init(content::base& /*c*/);
	///
	/// \brief Check post data for media or login and call it's handlers
	/// \param c content for base view
	///
	void post(content::base& /*c*/);
	///
	/// \brief Returns locale for current user
	///
	const std::string& locale_name();
	///
	/// \brief Returns path to media
	///
	const std::string& media();
	///
	/// \brief Set Alert struct
	///
	void alert(std::string const& /*text*/, std::string const& /*type*/, bool /*enabled*/, bool /*dismiss*/);
	///
	/// \brief Performs initialization of language 
	///
	void init_lang();
	void locale(std::string const&);
	std::string locale();
	void load_form(content::base& /*c*/);
	bool load(content::base &/*c*/);
	//void set_menu(const cppcms::json::value& /*v*/, tools::vec_map& /*menu*/);
	void menu_add(const std::string& /*menu*/, const std::string& /*name*/, const std::string& /*url*/);
	void menu_add(const std::string& /*menu*/, const std::string& /*name*/, const std::string& /*url*/, int /*order_id*/);

	std::string brand();
	std::string url(const std::string& /*s*/);
	cppcms::service& service();
	cppcms::http::context& context();
	cppcms::http::request& request();
	cppcms::http::response& response();
	cppcms::json::value const& settings();

private:
	cppcms::application& app_;
	cppcms::service& service_;
	cppcms::json::value *settings_;

	std::string media_;
	std::string alert_text;
	std::string alert_type;
	bool alert_enabled;
	bool alert_dismiss;

	std::string cookie_prefix_;
	std::string locale_name_;
	std::string file_;
	tools::map_str languages;

	//static & default menu items
	//cppcms::json::value menu_;
	//tools::map_str links_;
	
	//already ordered
	//tools::map_str page_header_;
	//tools::map_str page_sidebar_;
	//tools::map_str page_userbar_;
	
	//menu_t -> order_id -> name,url
	menu_t menu_;
	//max order_id in map
	int menu_max_;
	std::vector<page_t> pages_;
};

#endif
