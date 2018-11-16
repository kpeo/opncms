////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/view.h>

///
/// \cond internal
///

namespace {
	cppcms::xss::rules const& xss_filter()
	{
		static cppcms::xss::rules r;
		static bool initialized = false;
		if (initialized)
			return r;
		using namespace cppcms::xss;

		r.html(rules::xhtml_input);
		r.add_tag("ol",rules::opening_and_closing);
		r.add_tag("ul",rules::opening_and_closing);
		r.add_tag("li",rules::opening_and_closing);
		r.add_tag("p",rules::opening_and_closing);
		r.add_property("p","style",booster::regex("\\s*text-align\\s*:\\s*(center|left|right)\\s*;?"));
		r.add_tag("b",rules::opening_and_closing);
		r.add_tag("i",rules::opening_and_closing);
		r.add_tag("tt",rules::opening_and_closing);
		r.add_tag("sub",rules::opening_and_closing);
		r.add_tag("sup",rules::opening_and_closing);
		r.add_tag("blockquote",rules::opening_and_closing);
		r.add_tag("strong",rules::opening_and_closing);
		r.add_tag("em",rules::opening_and_closing);
		r.add_tag("h1",rules::opening_and_closing);
		r.add_tag("h2",rules::opening_and_closing);
		r.add_tag("h3",rules::opening_and_closing);
		r.add_tag("h4",rules::opening_and_closing);
		r.add_tag("h5",rules::opening_and_closing);
		r.add_tag("h6",rules::opening_and_closing);
		booster::regex cl_id(".*");
		r.add_property("h1","id",cl_id);
		r.add_property("h2","id",cl_id);
		r.add_property("h3","id",cl_id);
		r.add_property("h4","id",cl_id);
		r.add_property("h5","id",cl_id);
		r.add_property("h6","id",cl_id);
		r.add_tag("span",rules::opening_and_closing);
		r.add_property("span","id",cl_id);
		r.add_tag("code",rules::opening_and_closing);
		r.add_tag("pre",rules::opening_and_closing);
		r.add_property("pre","name",booster::regex("\\w+"));
		r.add_property("pre","class",booster::regex("\\w+"));
		r.add_tag("a",rules::opening_and_closing);
		r.add_uri_property("a","href");
		r.add_tag("hr",rules::stand_alone);
		r.add_tag("br",rules::stand_alone);
		r.add_tag("img",rules::stand_alone);
		r.add_uri_property("img","src");
		r.add_integer_property("img","width");
		r.add_integer_property("img","height");
		r.add_integer_property("img","border");
		r.add_property("img","alt",booster::regex(".*"));
		r.add_tag("table",rules::opening_and_closing);
		r.add_tag("tr",rules::opening_and_closing);
		r.add_tag("th",rules::opening_and_closing);
		r.add_tag("td",rules::opening_and_closing);
		r.add_integer_property("table","cellpadding");
		r.add_integer_property("table","cellspacing");
		r.add_integer_property("table","border");
		r.add_tag("center",rules::opening_and_closing);
		r.add_entity("nbsp");
		r.encoding("UTF-8");
		r.comments_allowed(true);

		initialized = true;
		return r;
	}

	struct init_it { init_it(){ xss_filter(); } } instance;

	std::string xssfilter(const std::string& s)
	{
		return cppcms::xss::filter(s,xss_filter(),cppcms::xss::escape_invalid);
	}

} //local namespace

namespace cppcms {
namespace json {
	template<>
	struct traits <content::base>{
		static content::base get(value const& v)
		{
			content::base b;

			if (v.object().size()!=8)
				throw bad_value_cast();

			b.doc=v.get<std::string>("file");

			for(tools::vec_str::const_iterator i=b.docs.begin();i!=b.docs.end();++i)
				b.docs.push_back(*i);
		}
		static void set(value &v,content::base const& b)
		{
			cppcms::json::array obj;
			v.set<std::string>("file",b.doc);

			for(tools::vec_str::const_iterator i=b.docs.begin();i!=b.docs.end();++i)
				obj.push_back(*i);

			v.set<cppcms::json::array>("files",obj);
		}
	};
} // json
} // cppcms

///
/// \endcond
///

View::View(cppcms::application& app, cppcms::service& srv)
: app_(app),
service_(srv),
menu_max_(0)
{
	BOOSTER_LOG(debug, __FUNCTION__);
	cookie_prefix_=app_.settings().get("session.cookies.prefix","cppcms_session")+"_";
	media_=app_.settings().get<std::string>("opncms.media");

// init menu
/*
	cppcms::json::object a = app_.settings().get<cppcms::json::object>("opncms.view.links");
	for(cppcms::json::object::iterator it = a.begin(); it != a.end(); ++it) {
		if (it->first.str().empty())
			continue; //just skip empty key elements (but empty url - is ok)
		links_[it->first.str()] = it->second.str();
		//BOOSTER_LOG(debug, __FUNCTION__) << "DBG: list[" << it->first.str() << "]=" << it->second.str();
	}
	menu_ = app_.settings().get<cppcms::json::object>("opncms.view.menu");
*/
	init_lang();
	BOOSTER_LOG(debug, __FUNCTION__) << "Init complete successfuly";
	alert("","",false,false);
}

View::~View() {}

//	---------------------------------- INIT ----------------------------------
//init() - This member function called when URL is dispatched to this application,
//	it is useful when member functions of applications are called, by default does nothing
void View::init(content::base &c)
{
	BOOSTER_LOG(debug, __FUNCTION__);
	c.site_title = "opnCMS";
	c.navbar_inverse = true;
	c.cookie_prefix = cookie_prefix_;
	c.xssfilter = xssfilter;
	c.media = media_;
	
	c.main_local = locale_name_;

	c.languages = languages;
	c.locale = locale_name_;

	c.authed = ioc::get<Auth>().auth();

	if (c.authed)
		c.cur_user = c.username = ioc::get<Auth>().id();
	else
		c.cur_user = c.username = "anonymous";

	if(menu_.empty())
	{
		BOOSTER_LOG(debug, __FUNCTION__) << "init menu from storage";
		cppcms::json::value pages;
		if(ioc::get<Data>().driver().get(pages,"pages"))
		{
			page_t page_tmp;
			std::map<int, std::pair<std::string,std::string> > menu_tmp;
	
			//collect menu from pages
			for(cppcms::json::object::iterator it = pages.object().begin(); it != pages.object().end(); ++it)
			{
				page_tmp = it->second.get_value<page_t>();
				page_tmp.id = tools::str2num<int>(it->first.str());
				pages_.push_back(page_tmp);
#if __cplusplus>=201103L
				menu_tmp[page_tmp.order_id] = std::make_pair(page_tmp.name,page_tmp.url);
#else
				menu_tmp[page_tmp.order_id] = std::make_pair<std::string,std::string>(page_tmp.name,page_tmp.url);
#endif
				menu_max_ = (page_tmp.order_id>menu_max_)?page_tmp.order_id:menu_max_;
				
				if(page_tmp.menu & MENU_HEADER)
					menu_["header"] = menu_tmp;
				if(page_tmp.menu & MENU_SIDEBAR)
					menu_["sidebar"] = menu_tmp;
				if(page_tmp.menu & MENU_USERBAR)
					menu_["userbar"] = menu_tmp;
					
				menu_tmp.clear();
			}
		} else {
			BOOSTER_LOG(debug, __FUNCTION__) << "pages storage not exists";
		}
	}
	
	
	if(!menu_.empty())
	{
		std::map<int, std::pair<std::string, std::string > >::iterator it = menu_["header"].begin();
		for(; it != menu_["header"].end(); ++it)
			c.menu_header.push_back(it->second);
		it = menu_["sidebar"].begin();
		for(; it != menu_["sidebar"].end(); ++it)
			c.menu_sidebar.push_back(it->second);
		it = menu_["userbar"].begin();
		for(; it != menu_["userbar"].end(); ++it)
			c.menu_userbar.push_back(it->second);
	}
/*
	c.menu_header = menu_[MENU_HEADER];
	c.menu_sidebar = menu_[MENU_SIDEBAR];
	c.menu_userbar = menu_[MENU_USERBAR];
*/	
	c.local = ioc::get<Auth>().local();

//TODO: it should be cashed
	if(c.plugins.empty())
	{
		//c.plugins.clear();
		//load plugins vector
		for(Plug::iterator p = ioc::get<Plug>().begin(); p!=ioc::get<Plug>().end(); ++p)
		{
			BOOSTER_LOG(debug,__FUNCTION__) << "Plugin " << p->first << ": add to base content";
			c.plugins.push_back(ioc::get<Plug>().get(p->first));
		}
	}
	c.homepage = false;

	//get positions of plugins
	c.is_css = ioc::get<Plug>().is_pos("css");
	c.is_js_head = ioc::get<Plug>().is_pos("js_head");
	c.is_js_foot = ioc::get<Plug>().is_pos("js_foot");
	c.is_left = ioc::get<Plug>().is_pos("left");
	c.is_right = ioc::get<Plug>().is_pos("right");
	c.is_top = ioc::get<Plug>().is_pos("top");
	c.is_bottom = ioc::get<Plug>().is_pos("bottom");
	c.is_menu = ioc::get<Plug>().is_pos("menu");
	c.is_content = ioc::get<Plug>().is_pos("content");

	//set default Bootstrap values
	c.navbar_inverse = true;
	c.container_fluid = false;
	c.is_search = false;

	c.is_alert = alert_enabled;
	c.alert_text = alert_text;
	c.alert_type = alert_type;
	c.alert_dismiss = alert_dismiss;
	app_.context().locale(locale_name_);
}

std::string View::brand()
{
	return app_.settings().get<std::string>("opncms.brand","opnCMS");
}

std::string View::url(const std::string& s)
{
	return app_.url(s);
}

cppcms::service& View::service()
{
	return service_;
}

cppcms::http::context& View::context()
{
	return app_.context();
}

cppcms::http::request& View::request()
{
	return app_.request();
}

cppcms::http::response& View::response()
{
	return app_.response();
}

cppcms::json::value const& View::settings()
{
	return app_.settings();
}

const std::string& View::locale_name()
{
	return locale_name_;
}

const std::string& View::media()
{
	return media_;
}

void View::init_lang()
{
	std::locale l;
	cppcms::json::array langs = app_.settings().get("opncms.localization.locales",cppcms::json::array());


	for(cppcms::json::array::const_iterator it=langs.begin(); it!=langs.end(); ++it)
	{
		std::string ll,lname;
		// Translate as the target language
		// gettext("LANG")="localized language name"
		l = service_.generator().generate(it->str());
		ll = std::use_facet<booster::locale::info>(l).language();
		if (ll=="en")
			lname = "English";
		else
		{
			lname=_("LANG").str(l);
			if(lname=="LANG")
				lname=ll;
		}
		BOOSTER_LOG(debug,__FUNCTION__) << "language config(" << it->str() << "), full(" << ll << ")" << ", name(" <<  lname << ")";
		//languages[lname] = ll;
		languages[lname] = it->str();
	}

	// init locale_name_
	if (!languages.empty()) {
		std::map<std::string,std::string>::const_iterator p = languages.begin();
		locale_name_ = p->second;
		BOOSTER_LOG(debug,__FUNCTION__) << "languages is not empty, locale_name=" << locale_name_;
	} else {
		locale_name_ = "en_US.UTF-8"; //any other default language? )
		BOOSTER_LOG(debug,__FUNCTION__) << "languages is empty, locale_name=" << locale_name_;
	}
}

void View::locale(std::string const& lang)
{
	locale_name_ = lang;
	app_.context().locale(lang);
}

std::string View::locale()
{
	return locale_name_;
}

void View::load_form(content::base& c __attribute__((unused)))
{
	//Reserved for future use
	/*
	c.form.name.value(c.name);
	c.form.title.value(c.title);
	c.form.sidebar.value(c.sidebar);
	c.form.content.value(c.content);
	*/
}

bool View::load(content::base &c)
{
	//Reserved for future use
	BOOSTER_LOG(debug,__FUNCTION__);
	//BOOSTER_LOG(debug,__FUNCTION__) << "username=" << ioc::get<Auth>().id() << ", authed=" << (ioc::get<Auth>().auth()?"1":"0");
	//load_form(c);

	//load headers
	return true;
}

void View::post(content::base &c)
{
	c.authed = ioc::get<Auth>().auth();
	c.remind = ioc::get<Auth>().remind();
}

void View::alert(std::string const& text, std::string const& type, bool enabled, bool dismiss)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "text[" << text << "], type[" << type << "], enabled[" << enabled << "], dismiss[" << dismiss << "]";
	alert_text = text;
	alert_type = type;
	alert_enabled = enabled;
	alert_dismiss = dismiss;
}

void View::menu_add(const std::string& menu, const std::string& name, const std::string& url)
{
#if __cplusplus>=201103L
	menu_[menu][menu_max_++] = std::make_pair(name, url);
#else
	menu_[menu][menu_max_++] = std::make_pair<std::string, std::string>(name, url);
#endif
}

void View::menu_add(const std::string& menu, const std::string& name, const std::string& url, int order_id )
{
#if __cplusplus>=201103L
	menu_[menu][order_id] = std::make_pair(name, url);
#else
	menu_[menu][order_id] = std::make_pair<std::string, std::string>(name, url);
#endif
}
