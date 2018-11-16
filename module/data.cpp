////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/data.h>

#ifdef _OPNCMS_CPPCMS_OLD
data_cache::data_cache(cppcms::application& app)
:app_(app),enabled_(true)
{}
void data_cache::rise(std::string const &trigger) {
	if(enabled_)
		app_.cache().rise(trigger);
}
void data_cache::add_trigger(std::string const &trigger) {
	if(enabled_)
		app_.cache().add_trigger(trigger);
}
void data_cache::clear() {
	if(enabled_)
		app_.cache().clear();
}
bool data_cache::stats(unsigned &keys,unsigned &triggers) {
	if(enabled_)
		return app_.cache().stats(keys, triggers);
	return false;
}
bool data_cache::has_cache() {
	if(enabled_)
		return app_.cache().has_cache();
	return false;
}
bool data_cache::nocache() {
	if(enabled_)
		return app_.cache().nocache();
	return false;
}
bool data_cache::fetch_page(std::string const &key) {
	if(enabled_)
		return app_.cache().fetch_page(key);
	return false;
}
void data_cache::store_page(std::string const &key,int timeout){
	if(enabled_)
		app_.cache().store_page(key, timeout);
}
bool data_cache::fetch_frame(std::string const &key,std::string &result, bool notriggers) {
	if(enabled_)
		return app_.cache().fetch_frame(key, result, notriggers);
	return false;
}
void data_cache::store_frame(std::string const &key,
							std::string const &frame,
							std::set<std::string> const &triggers,
							int timeout,
							bool notriggers) {
	if(enabled_)
		app_.cache().store_frame(key, frame, triggers, timeout, notriggers);
}
void data_cache::store_frame(std::string const &key,
							std::string const &frame,
							int timeout,
							bool notriggers) {
	if(enabled_)
		app_.cache().store_frame(key, frame, timeout, notriggers);
}
void data_cache::enable() { data_cache::enabled_ = true; }
void data_cache::disable() { data_cache::enabled_ = false; }
bool data_cache::enabled() { return data_cache::enabled_; }

#endif
// ----------------------------------------- Data -----------------------------------------
//
// default:
//	connection_string = sqlite3:db=./db/site.db;@pool_size=16
//	file.params.path = "./store"
//	store = "sql"
//	sql.driver = "sqlite3"

Data::Data(cppcms::application& app)
#ifdef _OPNCMS_CPPCMS_OLD
:app_(app), dsql_(), dfile_(), dmongodb_(), cache_(app)
#else
:app_(app), dsql_(), dfile_(), dmongodb_(), cache_(app.service())
#endif
{
	BOOSTER_LOG(debug,__FUNCTION__);
	Data::driver("sqlite3", dsql_);
	Data::driver("postgresql", dsql_);
	Data::driver("odbc", dsql_);
	Data::driver("mysql", dsql_);
	Data::driver("file", dfile_);
	Data::driver("mongodb", dmongodb_);
}

Data::~Data()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	/*
	ioc::del<IData>("Data:mongodb");
	ioc::del<IData>("Data:file");
	ioc::del<IData>("Data:mysql");
	ioc::del<IData>("Data:odbc");
	ioc::del<IData>("Data:postgresql");
	ioc::del<IData>("Data:sqlite3");
	*/
}

IData& Data::driver(const std::string& driver, IData& driverclass)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "Register new driver: " << driver;
	ioc::add<IData>(driverclass, std::string("Data:")+driver);
	return driverclass;
}

IData& Data::driver(const std::string& driver, IData& driverclass, tools::map_str& params)
{
	if (params.size()) {
		for(tools::map_str::const_iterator p=params.begin(); p!=params.end(); ++p) {
			if (!p->second.empty())
				Data::params_[p->first] = p->second;
		}
	}
	if(!driverclass.init(driver, Data::params_))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can't init driver";
		throw std::bad_alloc();
	}
	Data::driver_ = driver;
	return Data::driver(driver, driverclass);
}

IData& Data::driver(const std::string& driver, tools::map_str& params)
{
	if (params.size()) {
		for(tools::map_str::const_iterator p=params.begin(); p!=params.end(); ++p) {
			if (!p->second.empty())
				Data::params_[p->first] = p->second;
		}
	}
	Data::driver_ = driver;
	if(!Data::driver().init(driver, params_))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can't init driver";
		throw std::bad_alloc();
	}		
	return Data::driver();
}

//return reference to driver instance
IData& Data::driver(const std::string& d)
{
	return ioc::get<IData>(std::string("Data:")+d);
}

//return reference to current driver
IData& Data::driver()
{
	return ioc::get<IData>(std::string("Data:")+Data::driver_);
}

//return the name of current driver
const std::string& Data::driver_name()
{
	return driver_;
}

const std::string& Data::path()
{
	return params_["path"];
}

const cppcms::json::value& Data::settings()
{
	return Data::app_.settings();//return settings_;
}

bool Data::is_sql()
{
	return driver().is_sql();
}

#ifdef _OPNCMS_CPPCMS_OLD
data_cache& Data::cache()
#else
cppcms::cache_interface& Data::cache()
#endif
{
	return Data::cache_;
}
cppcms::session_interface& Data::session()
{
	return Data::app_.session();
}

cppcms::http::request& Data::request()
{
	return Data::app_.request();
}
