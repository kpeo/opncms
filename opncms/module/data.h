////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_DATA_H
#define OPNCMS_M_DATA_H

#include <cppcms/cppcms_error.h>
#include <cppcms/application.h>
#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <cppcms/http_request.h>

#include <opncms/defs.h>
#include <opncms/ioc.h>
#include <cppcms/json.h>
#include <opncms/tools.h>

#ifdef _OPNCMS_CPPCMS_OLD
class data_cache {
public:
	data_cache(cppcms::application& /*app_*/);
	void rise(std::string const &/*trigger*/);
	void add_trigger(std::string const &/*trigger*/);
	void clear();
	bool stats(unsigned &/*keys*/,unsigned &/*triggers*/);
	bool has_cache();
	bool nocache();
	bool fetch_page(std::string const &/*key*/);
	void store_page(std::string const &/*key*/,int /*timeout=-1*/);
	bool fetch_frame(std::string const &/*key*/,std::string &/*result*/,bool notriggers=false);
	void store_frame(std::string const &/*key*/,
                                 std::string const &/*frame*/,
                                 std::set<std::string> const &triggers=std::set<std::string>(),
                                 int timeout=-1,
                                 bool notriggers=false);
	void store_frame(std::string const &/*key*/,
                                 std::string const &/*frame*/,
                                 int /*timeout*/,
                                 bool notriggers=false);
	template<typename Serializable>
	bool fetch_data(std::string const &key,Serializable &data,bool notriggers=false)
	{
		if(enabled_)
			return app_.cache().fetch_data<Serializable>(key, data, notriggers);
		return false;
	}

	template<typename Serializable>
	void store_data(std::string const &key,Serializable const &data,
				std::set<std::string> const &triggers=std::set<std::string>(),
				int timeout=-1,bool notriggers=false)
	{
	if(enabled_)
		app_.cache().store_data<Serializable>(key, data, triggers, timeout, notriggers);
	}

	template<typename Serializable>
	void store_data(std::string const &key,Serializable const &data,int timeout, bool notriggers=false)
	{
		if(enabled_)
			app_.cache().store_data<Serializable>(key, data, timeout, notriggers);
	}

	void enable();
	void disable();
	bool enabled();

private:
	cppcms::application& app_;
	bool enabled_;
};

#endif

///
/// \class IData data.h opncms/module/data.h
/// Interface for various implementations of Data Manager
///

class IData {
public:
	///
	/// \brief Initialize Data implementation with new settings (driver and params)
	/// \brief Return true if succeded
	/// \param driver driver name (sqlite,mysql,etc..)
	/// \param params params map for driver
	virtual bool init(const std::string& /*driver*/, tools::map_str& /*params*/) = 0;
	///
	/// \brief Checks the driver is sql supported
	///	
	virtual bool is_sql() = 0;
	///
	/// \brief Checks the storage existance
	/// \param storage name table or path to file with data
	///
	virtual bool exists(const std::string& /*storage*/) = 0;
	///
	/// \brief Checks the storage and key existance
	/// \param storage name table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	///
	virtual bool exists(const std::string& /*storage*/, const std::string& /*key*/) = 0;
	///
	/// \brief Performs creating of storage
	/// \param storage name table or path to file with data
	///
	virtual bool create(const std::string& /*storage*/) = 0;
	///
	/// \brief Performs loading of value for key in current storage
	/// \param storage name of table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	///
	virtual std::string get(const std::string& /*storage*/, const std::string& /*key*/) = 0;
	///
	/// \brief Performs loading of value of current storage
	/// \param storage name of table or path to file with data
	///
	virtual std::string get(const std::string& /*storage*/) = 0;
	///
	/// \brief Performs loading of value for key in current storage
	/// \param value cppcms::json::value to hold the results
	/// \param storage name of table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	///
	virtual bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/, const std::string& /*key*/) = 0;
	///
	/// \brief Performs loading of value of current storage
	/// \param value cppcms::json::value to hold the results
	/// \param storage name of table or path to file with data
	///
	virtual bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/) = 0;
	///
	/// \brief Performs setting of value for key in current storage
	/// \param storage name of table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	/// \param value string of value for key
	///
	virtual bool set(const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*value*/) = 0;
	///
	/// \brief Performs setting of value for key in current storage
	/// \param storage name of table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	/// \param value json value for key
	///
	virtual bool set(const std::string& /*storage*/, const std::string& /*key*/, cppcms::json::value& /*value*/) = 0;
	///
	/// \brief Performs setting of value in current storage
	/// \param storage name of table or path to file with data
	/// \param value string of value for key
	///
	virtual bool set(const std::string& /*storage*/, const std::string& /*value*/) = 0;
	///
	/// \brief Performs setting of value in current storage
	/// \param storage name of table or path to file with data
	/// \param value json value for key
	///
	virtual bool set(const std::string& /*storage*/, cppcms::json::value& /*value*/) = 0;
	///
	/// \brief Performs erasing of value for key in current storage
	/// \param storage name of table or path to file with data
	/// \param key path where the data is, with '.' delimiter
	///
	virtual bool erase(const std::string& /*storage*/, const std::string& /*key*/) = 0;
	///
	/// \brief Performs erasing of all data in storage
	/// \param storage name of table or path to file with data
	///
	virtual bool clear(const std::string& /*storage*/) = 0;
};

#include <opncms/module/data_sql.h>
#include <opncms/module/data_file.h>
#include <opncms/module/data_mongodb.h>

///
/// \class Data data.h opncms/module/data.h
/// \brief Data Manager - class to operate on various databases' data,
/// 	using storage/key/value principle.
/// \details
/// Sample usage:
///	Data d;
///	std::map<std::string, std::string> params;
///	params["db"] = "./test_data.db";
///	params["@pool_size"] = "16";
///	d.init("sql","sqlite3",params);
///
///	if(d.insert("test","key","value"))
///		d.get("test","");
///

class Data
{
public:
	///
	/// \brief Constructor
	///
	Data(cppcms::application& /*app*/);
	///
	/// \brief Destructor
	///
	~Data();
	///
	/// \brief Add new driver and return its reference
	/// \param driver driver name (sqlite,mysql,etc..)
	/// \param driverclass class for driver (DataSql,DataFile,etc..)
	///
	IData& driver(const std::string& /*driver*/, IData& /*driverclass*/);
	///
	/// \brief Add new driver with parameters and return its reference
	/// \param driver driver name (sqlite,mysql,etc..)
	/// \param params params map for driver
	///
	IData& driver(const std::string& /*driver*/, tools::map_str& /*params*/);
	///
	/// \brief Add new driver with parameters and return its reference
	/// \param driver driver name (sqlite,mysql,etc..)
	/// \param driverclass class for driver (DataSql,DataFile,etc..)
	/// \param params params map for driver
	///
	IData& driver(const std::string& /*driver*/, IData& /*driverclass*/, tools::map_str& /*params*/);
	///
	/// \brief Returns the driver using its name
	/// \param driver driver name (sqlite,mysql,etc..)
	///
	IData& driver(const std::string& /*driver*/);
	///
	/// \brief Returns the current driver
	///
	IData& driver();
	///
	/// \brief Returns the string of current driver
	///
	const std::string& driver_name();
	///
	/// \brief Returns the path of storage
	///
	const std::string& path();
	const cppcms::json::value& settings();
	bool is_sql();
#ifdef _OPNCMS_CPPCMS_OLD
	data_cache& cache();
#else
	cppcms::cache_interface& cache();
#endif
	cppcms::session_interface& session();
	cppcms::http::request& request();

private:
	cppcms::application& app_;
	//cppcms::json::value settings_;
	std::string driver_;
	std::map <std::string, std::string> params_;

	DataSql dsql_;
	DataFile dfile_;
	DataMongodb dmongodb_;

#ifdef _OPNCMS_CPPCMS_OLD
	data_cache cache_;
#else
	cppcms::cache_interface cache_;
#endif
};

#endif
