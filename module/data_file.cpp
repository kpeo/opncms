////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/data.h>

DataFile::DataFile()
{
	BOOSTER_LOG(debug,__FUNCTION__);
}

bool DataFile::init(const std::string& driver, tools::map_str &params)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "driver(" << driver << ")";
	path_ = params["path"];
	
	if (path_.empty())
		path_ = OPNCMS_DATA_FILE_PATH; //use default
		
	BOOSTER_LOG(debug,__FUNCTION__) << "path(" << path_ << ")";
	return true;
}

bool DataFile::is_sql()
{
	return false;
}

bool DataFile::exists(const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";
	return boost::filesystem::exists(boost::filesystem::path(storage));
}

bool DataFile::exists(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;
	std::stringstream ss;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() || key.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage/key are empty";
		return "";
	}
	if (!tools::data::load(storage, v)) {
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage(" << storage << ")";
		return "";
	}
	if (v.is_undefined())
		return "";
		
	ss << v.at(key);
	return ( (ss.str().empty() || ss.str() == "null") ? false : true );
}

bool DataFile::create(const std::string& storage)
{
	std::ofstream f(storage.c_str());

	//WARN: we don't make the necessary directories
	if(!f || !f.is_open())
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t create storage(" << storage << ")";
		return false;
	}
	f << std::flush;
	f.close();
	return true;
}

bool DataFile::set(const std::string& storage, const std::string& key, cppcms::json::value& value)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << "), value(" << tools::json_to_string(value) << ")";

	if ( storage.empty() || storage == "" || key.empty() || key == "")
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage/key arguments are empty, check them please";
		return false;
	}

	tools::data::load(storage, v);

	//save only different values
	if(v.find(key) == value)
		return true;

	tools::json_set(v, key, value);

	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value";
	return tools::data::save(storage, v, true);
}

bool DataFile::set(const std::string& storage, cppcms::json::value& value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), value(" << tools::json_to_string(value) << ")";

	if ( storage.empty() || storage == "")
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage/key arguments are empty, check them please";
		return false;
	}

	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value";
	return tools::data::save(storage, value, true);
}

bool DataFile::set(const std::string& storage, const std::string& key, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataFile::set(storage,key,v);
}

bool DataFile::set(const std::string& storage, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataFile::set(storage,v);
}

bool DataFile::get(cppcms::json::value& v, const std::string& storage, const std::string& key)
{
	std::stringstream ss;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if (storage.empty() || key.empty()) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage/key are empty";
		return false;
	}

	if (!tools::data::load(storage, v)) {
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage";
		return false;
	}
	v = v.find(key);
	if (v.is_undefined())
		return false;
	return true;
}

std::string DataFile::get(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;

	if(!DataFile::get(v, storage, key))
		return "";

	std::stringstream ss;
	ss << v;
	return ss.str();
}

bool DataFile::get(cppcms::json::value& v, const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if (storage.empty()) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage is empty";
		return false;
	}
	if (!tools::data::load(storage, v)) {
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage";
		return false;
	}
	if (v.is_undefined())
		return false;
	
	return true;
}

std::string DataFile::get(const std::string& storage)
{
	cppcms::json::value v;
	
	if(!DataFile::get(v, storage))
		return "";

	std::stringstream ss;
	ss << v;
	return ss.str();
}

bool DataFile::erase(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() || storage == "" || key.empty() || key == "")
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage/key arguments are empty, check it please";
		return false;
	}

	//save only different values
	if (tools::data::load(storage, v) && (!v.is_undefined()))
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "Previous file is not empty - operate on it";

		if(!v.find(key).is_undefined())
			tools::json_erase(key, v);
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value";
	return tools::data::save(storage, v, true);
}

bool DataFile::clear(const std::string& storage)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "]";

	if ( storage.empty() || storage == "")
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage argument is empty, check it please";
		return false;
	}
	boost::filesystem::remove( boost::filesystem::path(storage) );
	return true;
}
