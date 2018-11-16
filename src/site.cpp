////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/site.h>

#ifdef _DEB_ENV
#include <cppcms/http_request.h> //debug for environment
#endif

#include <iostream> //for istream
#include <cppcms/url_dispatcher.h>

///
/// \cond internal
///

std::map< std::string, void* > ioc::iocmap_;

//We should hold the plugins' pointers till the end 
//of application lifetime regarding the cppcms::application
static Plug::plt plugins_pointers;
static Plug pb_("plugins.js", plugins_pointers);

namespace content {

} //content

namespace apps {

/*
WARN: aware of using context (cache,session,locale,etc.) here.
		It is not exists yet.
*/
site::site(cppcms::service& srv, int argc, char** argv)
:
	cppcms::application(srv),
	argc_(argc),
	argv_(argv),
	rpc_(new rpc(srv, *this)),

	//we need reference to the application for cache, sessions, etc.
	db_(*this),
	ab_(*this),
	mb_(*this),
	vb_(*this, srv),
	first_request_(true)
{
	BOOSTER_LOG(debug, __FUNCTION__);
	ioc::add<Data>(db_);
	ioc::add<Auth>(ab_);
	ioc::add<Media>(mb_);
	ioc::add<View>(vb_);
	ioc::add<Plug>(pb_);

	//Init storage with config parameters
	tools::map_str params;
	std::string data_driver = settings().get<std::string>(std::string(OPNCMS_CONF)+"data.driver",OPNCMS_DATA_DRIVER);
	std::istringstream ss(OPNCMS_DATA_SQL_PARAM_JSON);
	cppcms::json::value data_settings_default;
	data_settings_default.load(ss,true);
	cppcms::json::object data_settings = settings().get<cppcms::json::object>(std::string(OPNCMS_CONF)+"data.settings");
	if(data_settings.empty())
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "data.settings is empty - init with default";
		data_settings = data_settings_default.object();
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "data.driver: " << data_driver;

	for(cppcms::json::object::const_iterator it=data_settings.begin(); it != data_settings.end(); ++it) {
		if (it->second.type()==cppcms::json::is_string)
			params[it->first.str()] = it->second.str();
		BOOSTER_LOG(debug,__FUNCTION__) << "data.settings: " << it->first.str() << "=" << it->second.str();
	}
	db_.driver(data_driver, params);

//-------------------- URL HANDLERS -------------------------------------------

	BOOSTER_LOG(debug, __FUNCTION__) << "Attaching application's handlers";

	dispatcher().assign("/", &site::display,this,0);
	mapper().assign("/");

	dispatcher().assign("/home", &site::display,this,0);
	mapper().assign("home","/home");

	mapper().root("");

//-------------------- PAGES HANDLERS -------------------------------------------

	//Context is not exists yet, so we should disable cache for storage requests
	//for old library
#ifdef _OPNCMS_CPPCMS_OLD
	db_.cache().disable();
#endif
	BOOSTER_LOG(debug, __FUNCTION__) << "Attaching pages' handlers";
	cppcms::json::value v;
	if(ioc::get<Data>().driver().get(v, "pages"))
	{
		if(v.is_undefined() || v.is_null()) {
			BOOSTER_LOG(debug, __FUNCTION__) << "There is no data for pages";
		}
		else if(v.type() != cppcms::json::is_object) {
			BOOSTER_LOG(debug, __FUNCTION__) << "Wrong type of pages' data";
		}
		else {
			BOOSTER_LOG(debug, __FUNCTION__) << "Proceed with pages' data"; // v = "id" : {...}
			std::string page_name, page_url;
			int page_type;
			cppcms::json::object::const_iterator it = v.object().begin();
			for(; it != v.object().end(); ++it )
			{
				page_name = it->second.get<std::string>("name");
				page_url = it->second.get<std::string>("url");
				page_type = it->second.get<int>("type");
				BOOSTER_LOG(debug, __FUNCTION__) << "name(" << page_name << "), url(" << page_url << "), type(" << page_type << ")";
				if(!page_url.empty() && !page_name.empty())
				{
					//we should render depending on page type
					dispatcher().assign(page_url, &site::display,this,0);
					mapper().assign(page_name,page_url);
				}
			}
		}
	}
	else
		BOOSTER_LOG(debug, __FUNCTION__) << "Can't find pages in storage";
#ifdef _OPNCMS_CPPCMS_OLD
	db_.cache().enable();
#endif
//-------------------- RPC HANDLERS -------------------------------------------

	BOOSTER_LOG(debug, __FUNCTION__) << "Attaching application's rpc handlers";

	attach( &*rpc_,
		"rpc",
		"/rpc/{1}",
		"/rpc(/(\\d+))?", 2 );

//-------------------- PLUGINS ------------------------------------------------
	BOOSTER_LOG(debug, __FUNCTION__) << "Attaching pluggable child applications";

	if (pb_.load("plugins.js","plugins")) {

		BOOSTER_LOG(debug, __FUNCTION__) << "Plugins loaded successfully";

		//Try to attach all URL & RPC objects
		attach_plugins(pb_);
	}
}

void site::display(std::string upath)
{
	BOOSTER_LOG(debug,__FUNCTION__) << " upath=" << upath;

	content::base c;
	vb_.post(c);

	int utype = ioc::get<Auth>().user_type();
	BOOSTER_LOG(debug,__FUNCTION__) << " user_type=" << utype;

	const std::string key="user_"+ab_.id()+"-"+fmt::format("{}",utype)+"-"+((c.remind)?std::string("1"):std::string("0"))+":"+vb_.locale_name()+":"+upath;

	BOOSTER_LOG(debug,__FUNCTION__) << " search cache base with key: " << key;
	if (cache().fetch_page(key)) {
		BOOSTER_LOG(debug,__FUNCTION__) << " base with key " << key << " already cached";
		return; //we already display base
	}

	BOOSTER_LOG(debug,__FUNCTION__) << " base with key: " << key << " not cached";

	vb_.init(c);
	vb_.load(c);
	c.homepage = true;
	//c.username = ab_.id();
	//c.local = ab_.local();

	BOOSTER_LOG(debug,__FUNCTION__) << "render base";

	render("base",c);
	cache().store_page(key);
}

template <typename T>
void site::attach_plugin(T plug, tools::vec_str &m)
{
	BOOSTER_LOG(debug, __FUNCTION__);
	if (plug != NULL) {
		try {
			attach( &*plug, //take a normal pointer - like .get()
				m[0],
				m[1],
				m[2],
				tools::str2num<int>(m[3])
			);
		}
		catch(std::exception const& e) {
			BOOSTER_LOG(error, __FUNCTION__) << "Error in attaching: " << e.what();
		}
	}
}

void site::attach_plugins(Plug& pb)
{
	Plug::iterator p;

	BOOSTER_LOG(debug, __FUNCTION__);

	for(p = pb.begin(); p!=pb.end(); p++)
	{
		std::string path = "plugins." + p->first + ".url";
		std::string path_rpc = "plugins." + p->first + ".rpc";

		tools::vec_str m, mr;

		pb.attach_params(p->first, path, m, 0);
		pb.attach_params(p->first, path_rpc, mr, 1);

		BOOSTER_LOG(debug, __FUNCTION__) << "Params for URL: name=" << m[0] << ", var1=" << m[1] << ", var2=" << m[2] << ", var3=" << m[3];
		BOOSTER_LOG(debug, __FUNCTION__) << "Params for RPC: name=" << mr[0] << ", var1=" << mr[1] << ", var2=" << mr[2] << ", var3=" << mr[3];

		Plugin* tmp = pb.get(p->first);
		PluginRpc* tmp_rpc = pb.get_rpc(p->first);

		BOOSTER_LOG(debug, __FUNCTION__) << "Plugin " << p->first << " pointers: URL=" << tmp << ", RPC=" << tmp_rpc;

		//init plugin content
		tmp->prepare();

		attach_plugin<Plugin*>(tmp,m);
		attach_plugin<PluginRpc*>(tmp_rpc,mr);

		m.clear();
		mr.clear();
	}
}

#ifdef _DEB_ENV
void site::main(std::string url)
{

/*
	std::map<std::string,std::string> env=request().getenv();
	std::map<std::string,std::string>::const_iterator i;
	for(i = env.begin(); i != env.end(); ++i)
		BOOSTER_LOG(debug,"ENV") << (*i).first << ":" << (*i).second;
*/
	BOOSTER_LOG(debug,__FUNCTION__) << "PATH_INFO=" << request().path_info();
	BOOSTER_LOG(debug,__FUNCTION__) << "SCRIPT_NAME=" << request().script_name();
	BOOSTER_LOG(debug,__FUNCTION__) << "QUERY_STRING=" << request().query_string();
}
#endif

site::~site()
{
	BOOSTER_LOG(debug, __FUNCTION__);
	/*
	ioc::del<Plug>();
	ioc::del<View>();
	ioc::del<Media>();
	ioc::del<Auth>();
	ioc::del<Data>();
	 */
	ioc::clear();
}

} // apps

///
/// \endcond
///
