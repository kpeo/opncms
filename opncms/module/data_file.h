////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_DATA_FILE_H
#define OPNCMS_M_DATA_FILE_H

class DataFile : public IData
{
public:
	DataFile();
	bool init(const std::string& /*driver*/, tools::map_str& /*params*/);
	bool is_sql();
	bool exists(const std::string& /*storage*/);
	bool exists(const std::string& /*storage*/, const std::string& /*key*/);
	int count(const std::string& /*storage*/, const std::string& /*key*/);
	bool create(const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/, const std::string& /*key*/);
	std::string get(const std::string& /*storage*/);
	std::string get(const std::string& /*storage*/, const std::string& /*key*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*value*/);
	bool erase(const std::string& /*storage*/, const std::string& /*key*/);
	bool clear(const std::string& /*storage*/);
protected:
	std::string path_;
};

#endif
