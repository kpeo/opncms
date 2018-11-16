////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_DATA_SQL_H
#define OPNCMS_M_DATA_SQL_H

#include <cppdb/frontend.h>
#include <cppdb/utils.h> //cppdb::connection_info

class DataSql : public IData {
public:
	DataSql();
	bool init(const std::string& /*driver*/, tools::map_str& /*params*/);
	cppdb::session session();
	bool is_sql();
	bool exists(const std::string& /*storage*/);
	bool exists(const std::string& /*storage*/, const std::string& /*key*/);
	bool create(const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/, const std::string& /*key*/);
	std::string get(const std::string& /*storage*/, const std::string& /*key*/);
	std::string get(const std::string& /*storage*/);
	bool set(const std::string& /*storage*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*value*/);
	bool erase(const std::string& /*storage*/, const std::string& /*key*/);
	bool clear(const std::string& /*storage*/);
private:
	// set cache each key till target key with value from root or second value (after first dot) (if is_second is true)
	void set_cache(const std::string& /*storage*/, const std::string& /*key*/, const cppcms::json::value& /*value*/, bool is_second=false);
	bool get_dotted(const std::string& /*key*/, std::string& /*first*/, std::string& /*second*/);
	cppdb::connection_info conn_;
	bool storage_changed_;
};

#endif
