////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_BASE_H
#define OPNCMS_BASE_H

#include <cppcms/view.h>
#include <cppcms/form.h>
#include <booster/function.h>
#include <string>

//#include <opncms/tools.h>
#include <opncms/module/plugin.h>

///
/// \cond internal
///

//Bitmask can represents page in several menu
enum menu_mask
{
		MENU_HEADER		= 1,
		MENU_SIDEBAR	= 2,
		MENU_USERBAR	= 4
};

struct page_t
{
        int id;
        int order_id;
        std::string name;
        std::string icon;
        std::string url;
        std::string type;
        bool authed;
        bool active;
        int menu;
        cppcms::json::value data;
};

typedef std::vector< std::pair<std::string, std::string> > menu_t;

namespace apps { class site; }

namespace content {

struct base : public cppcms::base_content {
	std::string doc;
	std::vector<std::string> docs;
	std::string media;
	std::string cookie_prefix;
	std::string main_link;
	std::string main_local;
	std::string login_link;
	std::string changes;
	std::string edit_options;
	std::string contact;
	std::string site_title,about,copyright;
	std::map<std::string,std::string> languages;
	std::string locale;
	booster::function<std::string(const std::string& )> xssfilter;

	bool post_type;
	bool remind;
	bool authed; //is client authed?
	bool local; //is client local?
	bool homepage; //is this page - home?

	std::string conn_err;
	std::string username;
	std::string cur_user;
	std::string cur_page;
	std::string new_user;
	std::string user_url;
	std::string edit_url;

	std::string filename;

	std::string name, title, content, sidebar;

	menu_t menu_header;
	menu_t menu_sidebar;
	menu_t menu_userbar;

	std::vector<Plugin *> plugins;
	bool is_css;
	bool is_js_head;
	bool is_js_foot;
	bool is_left;
	bool is_right;
	bool is_top;
	bool is_bottom;
	bool is_menu;
	bool is_content;

	/* Bootstrap settings */
	bool navbar_inverse;
	bool container_fluid;

	//Alert alert;
	bool is_alert;
	bool alert_dismiss;
	std::string alert_text;
	std::string alert_type;

	bool is_search;
};

}

///
/// \endcond
///

#endif
