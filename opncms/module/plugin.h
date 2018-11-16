////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_PLUG_H
#define OPNCMS_M_PLUG_H

#ifdef OPNCMS_WIN_NATIVE
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <memory> //for unique_ptr

#include <opncms/tools.h>

#include <cppcms/cppcms_error.h>

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/rpc_json.h>

#include <booster/nowide/fstream.h>
#include <booster/log.h> //for plugin implementations & current log
#include <sstream>
#include <map>
#include <booster/shared_object.h>
//#include <version.h>
#include <fmt/format.h>

const std::string api_version = "0.0.1";
class Plug;

class Plugin: public cppcms::application
{
public:
	Plugin(cppcms::service &srv)
	: cppcms::application(srv)
	{}
	// use virtual - otherwise linker will try to perform static linkage
	virtual void prepare() = 0;
	virtual void display(std::string upath="home") = 0;

	virtual std::string skin() = 0; //skin name (plugin name + "_view")
	virtual std::string view(std::string const &) = 0; //view name (plugin name + input string)
//	virtual std::string id() = 0; //id (priority)
	virtual std::string name() const = 0; //description
	virtual std::string shortname() const = 0; //shortname (latin)
	virtual std::string slug() const = 0; //path for url
	virtual std::string version() const = 0;
	virtual tools::vec_str& map() = 0;

	virtual cppcms::base_content& html_css() = 0;
	virtual cppcms::base_content& html_js_head() = 0;
	virtual cppcms::base_content& html_js_foot() = 0;
	virtual cppcms::base_content& html_menu() = 0;
	virtual cppcms::base_content& html_left() = 0;
	virtual cppcms::base_content& html_right() = 0;
	virtual cppcms::base_content& html_top() = 0;
	virtual cppcms::base_content& html_bottom() = 0;
	virtual cppcms::base_content& html_content() = 0;
	
	virtual bool is_css() = 0;
	virtual bool is_js_head() = 0;
	virtual bool is_js_foot() = 0;
	virtual bool is_menu() = 0;
	virtual bool is_left() = 0;
	virtual bool is_right() = 0;
	virtual bool is_top() = 0;
	virtual bool is_bottom() = 0;
	virtual bool is_content() = 0;
};

class PluginRpc: public cppcms::rpc::json_rpc_server
{
public:
	PluginRpc(cppcms::service &srv)
	: cppcms::rpc::json_rpc_server(srv)
	{}

	virtual void methods() = 0;
	virtual tools::vec_str& map() = 0;
};

struct err_policy
{
	struct excpt : public std::exception {
		const char* what() const throw() {
			return "factory called with unknown type";
		}
	};
};

//Plugins container
class Plug : public err_policy
{
public:
	//type for plugins: name => (Plugin*,PluginRpc*)
	typedef std::pair<Plugin*,PluginRpc*> pltp;
	typedef std::map< std::string, pltp > plt;

	typedef plt::const_iterator const_iterator;
	typedef plt::iterator iterator;
	typedef plt::reverse_iterator reverse_iterator;
	typedef plt::reference reference;
	typedef plt::const_reference const_reference;

	// standart container's members
	iterator begin() { return pl_.begin(); }
	iterator end() { return pl_.end(); }
	const_iterator begin() const { return pl_.begin(); }
	const_iterator end() const { return pl_.end(); }

	bool empty() const { return pl_.empty(); }
	size_t count ( const std::string& _s ) const {return pl_.count(_s);}
	size_t size () const {return pl_.size();}

	Plugin* get(std::string const &id)
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		const_iterator i = pl_.find(id);
		if(i != pl_.end())
			return i->second.first;
		return NULL;
	}
	PluginRpc* get_rpc(std::string const &id)
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		const_iterator i = pl_.find(id);
		if(i != pl_.end())
			return i->second.second;
		return NULL;
	}

	bool del(std::string const &id)
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		const_iterator i = pl_.find(id);
		if(i != pl_.end())
		{
			if(i->second.first != NULL)
				delete i->second.first;
			if(i->second.second != NULL)
				delete i->second.second;
			return pl_.erase(id) != 0;
		}
		else
		{
			return false;
		}
	}

	void clear()
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		for(iterator p = pl_.begin(); p!=pl_.end(); p++)
		{
			if(p->second.first)
				delete p->second.first;
			if(p->second.second)
				delete p->second.second;
		}
		pl_.clear();
	}

	// specific functions

	Plug(const std::string& conf, plt& pl)
	: config_path_(conf), pl_(pl)
	{}

	~Plug()
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		//Unfortunately, we can't delete the plugin pointers, 
		//because of handling it in cppcms::application
		//clear();
	}

	template<typename T>
	T get_config(const std::string &file_name, const std::string &path)
	{
		T obj;

		// if config already loaded - use loaded
		if(config_.is_undefined())
		{
			BOOSTER_LOG(debug, __FUNCTION__) << "Loading config file " << file_name;
			booster::nowide::ifstream fin((file_name).c_str());

			if(!fin)
				throw cppcms::cppcms_error("Failed to open filename: "+file_name);

			int line_no=0;

			if(!config_.load(fin,true,&line_no))
			{
				std::ostringstream ss;
				ss<<"Error reading configurarion file "<<file_name<<" in line:"<<line_no;
				throw cppcms::cppcms_error(ss.str());
			}
			if(config_.is_undefined())
			{
				throw cppcms::cppcms_error("No configuration defined");
			}
		}

		try
		{
			obj = config_.get(path, T() );
		}
		catch(std::exception const &e)
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Can`t get path, error: " << e.what();
			return obj; //FIXME: must return NULL pointer
		}
		return obj;
	}

	void prepare_pos()
	{
		pos_["css"] = 0;
		pos_["js_head"] = 0;
		pos_["js_foot"] = 0;
		pos_["left"] = 0;
		pos_["right"] = 0;
		pos_["top"] = 0;
		pos_["bottom"] = 0;
		pos_["menu"] = 0;
		pos_["content"] = 0;
		
		for(plt::const_iterator it = pl_.begin(); it != pl_.end(); it++)
		{
			//WARN: what if second.first is undefined?
			BOOSTER_LOG(debug, __FUNCTION__) << "get positions for plugin " << it->first << " at URL pointer " << it->second.first;
			BOOSTER_LOG(debug, __FUNCTION__) << std::string(it->second.first->shortname());
			if(it->second.first->is_css())
				pos_["css"]++;
			if(it->second.first->is_js_head())
				pos_["js_head"]++;
			if(it->second.first->is_js_foot())
				pos_["js_foot"]++;
			if(it->second.first->is_left())
				pos_["left"]++;
			if(it->second.first->is_right())
				pos_["right"]++;
			if(it->second.first->is_top())
				pos_["top"]++;
			if(it->second.first->is_bottom())
				pos_["bottom"]++;
			if(it->second.first->is_menu())
				pos_["menu"]++;
			if(it->second.first->is_content())
				pos_["content"]++;
		}
	}

	bool is_pos(std::string const &p)
	{
		if(pos_.find(p) != pos_.end())
			return(pos_[p]>0);
		else
			BOOSTER_LOG(error, __FUNCTION__) << "Incorrect position name: " << p;
			
		return false;
	}

	/* return pointer to shared class */
	bool load_lib(const std::string& lib, int type)
	{
		BOOSTER_LOG(debug, __FUNCTION__) << "library(" << lib << "), type(" << type << ")";
		if(lib.empty())
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Library not defined!";
			return false;
		}
		std::string path = config_.get("plugins.root", tools::empty_string );
		BOOSTER_LOG(debug, __FUNCTION__) << "path(" << path << ")";// << ", pointers: Plug=" << this;
#ifdef OPNCMS_WIN_NATIVE
		std::string libf = path + "\\" + lib + ".dll";
#else
		std::string libf = tools::empty_string + "." + SLH+path+SLH + lib + ".so";
#endif
		std::string error;
		if(!plugin_holder.open(libf, error, booster::shared_object::load_now)) {
			throw cppcms::cppcms_error("plugin: failed to load shared object " + libf + ": " + error);
		}
		//TODO: while CppCMS can't attach plugins by application without config
		BOOSTER_LOG(debug, __FUNCTION__) << ((type) ? "Get specific members (type>0)" : "Get standart url/rpc members (type=0)");
		
		//try to resolve URL-object/skin in library
		void (*create_)(); //FIXME: create_ must be deleted afterwards
		*(void**)&create_ = (type) ? plugin_holder.resolve_symbol("loader") : plugin_holder.resolve_symbol("plugin");

		if(create_ != NULL)
		{
			BOOSTER_LOG(debug, __FUNCTION__) << "Plugin created successfully, run it`s initialization";
			create_();
		}
		else
		{
			BOOSTER_LOG(error, __FUNCTION__) << ((type) ? "Can`t load skin from " : "Can`t load URL-object from ") << libf << " error: " << dlerror();
			return false;
		}
		return true;
	}

	bool load_plugins(tools::vec_str &ob)
	{
		tools::vec_str::iterator p;
		BOOSTER_LOG(debug, __FUNCTION__);

		if(ob.empty())
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Enabled plugins in config are empty" << std::endl;
			return false;
		}
		for(p = ob.begin(); p != ob.end(); p++)
		{
			BOOSTER_LOG(debug, __FUNCTION__) << "Loading plugin " << *p;
			if(get(*p) != NULL || get_rpc(*p) != NULL)
			{
				BOOSTER_LOG(debug, __FUNCTION__) << "Plugin already loaded";
				return true;
			}
			if(!load_lib(*p,0))
			{
				BOOSTER_LOG(error, __FUNCTION__) << "Plugin " << *p << " has no any objects";
			}
			/* FIXME: see TODO below
			if(!load_lib(std::string("lib")+(*p)+"_view",1)) {
				BOOSTER_LOG(error, __FUNCTION__) << "Plugin " << *p << " has no any skin";
			}
			*/
			BOOSTER_LOG(debug, __FUNCTION__) << "Plugin " << *p << " loaded successfully";
		}
		return true;
	}

	//load objects URL/RPC to appropriate factories:
	//conf - location of configuration file
	//path - path to "plugins" part of config
	//srv - service object passed into new plugin objects
	bool load(std::string conf, std::string path)
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		tools::vec_str obj = get_config<tools::vec_str>(conf, path+".enabled");

		if(obj.empty())
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Can`t get path " << path << " in config " << conf;
			return false;
		}
		root_ = path;

		if(!load_plugins(obj))
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Can`t load plugins";
			return false;
		}
		prepare_pos();
		return true;
	}
	//TODO: reload plugin & all plugins

	//register plugin itself way
	//bool add(std::string name, booster::shared_ptr<Plugin>& pl, booster::shared_ptr<PluginRpc>& plr)
	bool add(const std::string& name, Plugin* pl, PluginRpc* plr, const std::string& version)
	{
		BOOSTER_LOG(debug, __FUNCTION__) << "Add plugin " << name << ", pointers: URL=" << pl << ", RPC=" << plr << ", version: " << version;
		
		//Just to be sure. We should not get here
		if(get(name) != NULL)
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Plugin " << name << "already added";
			delete pl;
			return true;
		}
		if(get_rpc(name) != NULL)
		{
			BOOSTER_LOG(error, __FUNCTION__) << "PluginRpc " << name << "already added";
			delete plr;
			return true;
		}

		//check minor digit of version
		bool match = false;
		std::string v_plugin = tools::split_first(tools::split_second(version,"."), ".");
		std::string v_core = tools::split_first(tools::split_second(api_version,"."), ".");
		//std::string v_core = fmt::format("{}", opncms_VERSION_MINOR);
		if( !v_plugin.empty() && !v_core.empty() )
			match = (v_plugin == v_core);
		
		if(!match)
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Can't load plugin - minor plugin API version(" << v_plugin << ") is not compatible to minor core API version(" << v_core << ")";
			delete pl;
			delete plr;
			//BOOSTER_LOG(error, __FUNCTION__) << "Plugin API version(" << version << ") is not compatible to core API version(" << opncms_VERSION_MAJOR << "." << opncms_VERSION_MINOR << "." << opncms_VERSION_PATCH << ")";
			return false;
		}
		pl_.insert(plt::value_type(name,pltp(pl,plr)));
		
		//plugin's menu items added by plugin itself
		return true;
	}

//        ------------ Plugins funstions ---------------------
	void attach_fill(tools::vec_str &m, std::string const &s)
	{
		BOOSTER_LOG(debug, __FUNCTION__);
		m.push_back(s);
		m.push_back(tools::empty_string + "/" + s + "/{1}");
		m.push_back(tools::empty_string + "/" + s + "((/.*)?)");
		m.push_back(tools::empty_string + "1");
	}

	void attach_params(std::string const &plug_name, std::string const &path, tools::vec_str &m, int type)
	{
		//1. plugin's map
		//2. plugin's config
		//3. don`t use any default variants - it's unsecure

		BOOSTER_LOG(debug, __FUNCTION__) << "Loading plugin's map from config";
		//priority 1 - try to get data from conf (path - deffer url & rpc)
		try
		{
			m = get_config<tools::vec_str>("plugins.conf", path);
		}
		catch(std::exception const &e)
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Can`t get plugin`s " << plug_name << " path " << path << ", error: " << e.what() << ", try to get it from plugin";
		}
		//priority 2 - try to get data from plugin (type - deffer url=0 & rpc=1)
		BOOSTER_LOG(debug, __FUNCTION__) << "Loading plugin's map from plugin";
		if(!type)
		{
			if (m.empty())
			{
				BOOSTER_LOG(debug, __FUNCTION__) << "Loading URL-map";
				Plugin* p = get(plug_name);
				if(p != NULL)
					m = p->map();
			}
		}
		else
		{
			if (m.empty())
			{
				BOOSTER_LOG(debug, __FUNCTION__) << "Loading RPC-map";
				PluginRpc* p = get_rpc(plug_name);
				if(p != NULL)
					m = p->map();
			}
		}

		//priority 3 - give plugin a chanse for load (type - deffer url=0 & rpc=1)
		if(m.empty())
		{
			BOOSTER_LOG(error, __FUNCTION__) << "Plugin " << plug_name << " haven`t any mapping parameters in config, get custom" << std::endl;
			//give plugin a second chance - make default mapping parameters
			if(!type)
				attach_fill(m, plug_name);
			else
				attach_fill(m, plug_name+"_rpc");
		}
	}

	const std::string& config_path()
	{
		return config_path_;
	}

	//get template
	std::string get_tmp(const std::string& plug, const std::string& tmp)
	{
		//1. check for tmp
		if(!tmp.empty())
			return tmp;
		
		//2. check for config and if empty - check for plug
		if(!plug.empty())
		{
			std::string c = get_config<std::string>(config_path(), plug+".template");
			if(!c.empty())
				return c;
			else
				return plug+".tmp";
		}
		return "";
	}

private:
	booster::shared_object plugin_holder;

	cppcms::json::value config_;
	std::string root_;
	std::string config_path_;
	plt& pl_;

	std::map<std::string,int> pos_;
};

#endif
