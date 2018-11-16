////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/data.h>


/* we assume the first key as id column */
DataSql::DataSql()
:storage_changed_(false)
{
	BOOSTER_LOG(debug,__FUNCTION__);
}

bool DataSql::init(const std::string& driver, tools::map_str &params)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "driver(" << driver << ")";
	std::string conn = driver+":";
	int count = 0;
	if (params.size()) {
		for(tools::map_str::const_iterator p=params.begin(); p!=params.end(); ++p) {

			if (!p->second.empty()) {
				conn += ((count)?";":"") + p->first + "=" + p->second;
				count++;
			}
		}
		conn_.connection_string = conn;
	} else {
		//use default parameters
		conn_.connection_string = OPNCMS_DATA_SQL_PARAM;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "connection_string(" << conn << ")";
	conn_.connection_string = conn;
	return true;
}

bool DataSql::is_sql()
{
	return true;
}

cppdb::session DataSql::session()
{
	try {
		cppdb::session s(DataSql::conn_.connection_string);
		return s;
	}
	catch(std::exception const& e) {
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
	}
	return cppdb::session(); //empty
}

//User can attach own table processing - Adjacency List Model (ALM) or Nested Set Model (NSM)
//or use the database JSON objects.

bool DataSql::exists(const std::string& storage)
{
	std::string cache_data;
	int cnt = -1;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if(storage.empty()) {
		BOOSTER_LOG(notice,__FUNCTION__) << "storage is empty";
		return false;
	}

	std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage + "_rows";
	
	if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
		return true;
	
	try
	{
		std::stringstream ss;
		std::string c;
		ss << std::string("SELECT count(*) FROM ") << storage << ";";
		
		cppdb::statement st = DataSql::session() << ss.str();
		cppdb::result r = st.row();

		if (!r.empty()) {
			r >> c;
			std::istringstream iss(c);
			iss >> cnt;
		}
		else
			BOOSTER_LOG(debug,__FUNCTION__) << "table is empty or not exists";

		ioc::get<Data>().cache().store_frame(k,c,OPNCMS_DATA_CACHE_TIMEOUT);
	}
	catch(std::exception const& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "table has " << cnt << " rows";
	storage_changed_ = true;
	return (cnt>=0);
}

bool DataSql::exists(const std::string& storage, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";
	std::string s = DataSql::get(storage, key);

	return (s.empty() || s == "")?false:true;
}

bool DataSql::create(const std::string& storage)
{
	std::string cache_data;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if(storage.empty())
		return false;

	std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;

	if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
		return true;

	try {
		std::stringstream ss;
		//we should consider maximum hash size (512) + type (1) for key
		//TODO: handle JSON type since PostgreSQL v9.2
		ss << "CREATE TABLE IF NOT EXISTS " << storage << "(key VARCHAR(513) NOT NULL ";
		if(ioc::get<Data>().driver_name() == "sqlite3")
			ss << "PRIMARY KEY ASC, data VARCHAR(65532));";
		else if(ioc::get<Data>().driver_name() == "mysql")
			ss << ", data VARCHAR(65532), PRIMARY KEY(key));";
		else if(ioc::get<Data>().driver_name() == "odbc" || ioc::get<Data>().driver_name() == "postgresql")
			ss << ", data VARCHAR(65532), CONSTRAINT key_pk PRIMARY KEY (key));";
		cppdb::statement st = DataSql::session() << ss.str();
		st.exec();

		ioc::get<Data>().cache().store_frame(k,std::string(""),OPNCMS_DATA_CACHE_TIMEOUT);
	}
	catch(std::exception const& e) {
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	return true;
}

//WARN
//- key may be composite, so we need iterate over it
//- result may have multiple rows (parent_id is same) - parse it like array
//- result may have leafs(parent_id is not null) - parse it like arrays and maps
bool DataSql::get(cppcms::json::value& v, const std::string& storage, const std::string& key)
{
	std::string cache_data, s;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";
	
	if( storage.empty() || key.empty() )
		return false;

	std::string key_first, key_second;
	bool dotted = DataSql::get_dotted(key, key_first, key_second);

	std::string ks = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	std::string k = ks + ":" + key;
	BOOSTER_LOG(debug,__FUNCTION__) << "cache key(" << k << ")";

	if(!storage_changed_)
	{
		if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
		{
			v = tools::string_to_json(cache_data);
			return true;
		}
		BOOSTER_LOG(debug,__FUNCTION__) << "cache miss for key(" << k << ")";
	}
	
	try
	{
		std::stringstream ss;
		ss << std::string("SELECT data FROM ") << storage << " WHERE key=?;";

		cppdb::statement st = DataSql::session() << ss.str();
		st.bind(1, key_first);
		cppdb::result r = st.row();

		do
		{
			if (r.empty()) {
				BOOSTER_LOG(debug,__FUNCTION__) << "value is empty or not exists";
				return false;
			}
			r >> s;
			BOOSTER_LOG(debug,__FUNCTION__) << "value(" << s << ")";
			s = (tools::trim(s) == "null") ? "" : s;
			if(s.empty() || s == "")
			{
				BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ") at key(" << key_first << ") is empty";
				ioc::get<Data>().cache().store_frame(k,std::string(""),OPNCMS_DATA_CACHE_TIMEOUT);
				ioc::get<Data>().cache().rise(ks); //clear storage
				return false;
			}

			v = tools::string_to_json(s);

			if(!dotted)
			{
				ioc::get<Data>().cache().store_frame(k, s, OPNCMS_DATA_CACHE_TIMEOUT);
				ioc::get<Data>().cache().rise(ks);
				storage_changed_ = false;
				return true;
			}
			BOOSTER_LOG(debug,__FUNCTION__) << "key is in dot-notation, get nested part at key(" << key_second << ")";
			v = v.find(key_second);
			if(v.is_undefined() || v.is_null())
				return false;
			
			s = tools::json_to_string( v );
			BOOSTER_LOG(debug,__FUNCTION__) << "get value at extended key(" << key_second << ")(" << s << ")";
			ioc::get<Data>().cache().store_frame(k, s, OPNCMS_DATA_CACHE_TIMEOUT);
			storage_changed_ = false;

			return true;
		}
		while(r.next());
	}
	catch(std::exception const& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	BOOSTER_LOG(error,__FUNCTION__) << "Can't find the data at key";
	return false;
}

//- key may be composite, so we need iterate over it
//- result may have multiple rows (parent_id is same) - parse it like array
//- result may have leafs(parent_id is not null) - parse it like arrays and maps
std::string DataSql::get(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;
	std::string cache_data, s;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";
	
	if( storage.empty() || key.empty() )
		return "";

	std::string key_first, key_second;
	bool dotted = DataSql::get_dotted(key, key_first, key_second);

	std::string ks = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	std::string k = ks + ":" + key;
	BOOSTER_LOG(debug,__FUNCTION__) << "cache key(" << k << ")";

	if(!storage_changed_)
	{
		if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
			return cache_data;
		BOOSTER_LOG(debug,__FUNCTION__) << "cache miss for key(" << k << ")";
	}
	
	try
	{
		std::stringstream ss;
		ss << std::string("SELECT data FROM ") << storage << " WHERE key=?;";

		cppdb::statement st = DataSql::session() << ss.str();
		st.bind(1, key_first);
		cppdb::result r = st.row();

		do
		{
			if (r.empty()) {
				BOOSTER_LOG(debug,__FUNCTION__) << "value is empty or not exists";
				return "";
			}
			r >> s;
			BOOSTER_LOG(debug,__FUNCTION__) << "value(" << s << ")";
			s = (tools::trim(s) == "null") ? "" : s;
			if(s.empty() || s == "")
			{
				BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ") at key(" << key_first << ") is empty";
				ioc::get<Data>().cache().store_frame(k,std::string(""),OPNCMS_DATA_CACHE_TIMEOUT);
				ioc::get<Data>().cache().rise(ks); //clear storage
				storage_changed_ = false;
				return "";
			}
			if(!dotted)
			{
				ioc::get<Data>().cache().store_frame(k, s, OPNCMS_DATA_CACHE_TIMEOUT);
				ioc::get<Data>().cache().rise(ks);
				storage_changed_ = false;
				return s;
			}
			BOOSTER_LOG(debug,__FUNCTION__) << "key is in dot-notation, get nested part at key(" << key_second << ")";
			v = tools::string_to_json(s);
			v = v.find(key_second);
			s = tools::json_to_string( v );
			if(s == "null")
				s = "";
			if(!s.empty())
			{
				BOOSTER_LOG(debug,__FUNCTION__) << "value at extended key(" << key_second << ")(" << s << ")";
				ioc::get<Data>().cache().store_frame(k, s, OPNCMS_DATA_CACHE_TIMEOUT);
				storage_changed_ = false;
				return s;
			}
		}
		while(r.next());
	}
	catch(std::exception const& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return "";
	}
	BOOSTER_LOG(error,__FUNCTION__) << "Can't find the data at key";
	return "";
}

bool DataSql::get(cppcms::json::value &v, const std::string& storage)
{
	std::string cache_data, r_key, r_data, r_value;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";
	
	if( storage.empty() )
		return false;
	
	if(!DataSql::exists(storage))
		return false;

	std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	
	if(!storage_changed_)
	{
		if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
		{
			v = tools::string_to_json(cache_data);
			return true;
		}
		BOOSTER_LOG(debug,__FUNCTION__) << "cache miss for key(" << k << ")";
	}
	
	try
	{
		std::stringstream ss;
		ss << std::string("SELECT key,data FROM ") << storage << " WHERE key IS NOT NULL;";
		
		cppdb::statement st = DataSql::session() << ss.str();
		cppdb::result r = st.row();

		if (r.empty()) {
			BOOSTER_LOG(debug,__FUNCTION__) << "value is empty or not exists";
			return false;
		}
		r >> r_key >> r_data;
		
		r_value = std::string("{\"") + r_key + "\":" + r_data + "}";
		BOOSTER_LOG(debug,__FUNCTION__) << "value(" << r_value << ")";

		r.next();
		if(!r.empty())
			BOOSTER_LOG(debug,__FUNCTION__) << "Warning: there are more than one row for key";

		ioc::get<Data>().cache().store_frame(k,r_value,OPNCMS_DATA_CACHE_TIMEOUT);
		storage_changed_ = false;
		
		v = tools::string_to_json(r_value);
		return (v.is_null() || v.is_undefined())?false:true;
	}
	catch(std::exception const& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	BOOSTER_LOG(error,__FUNCTION__) << "There is no data";
	return false;
}

std::string DataSql::get(const std::string& storage)
{
	std::string cache_data, r_key, r_data, v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";
	
	if( storage.empty() )
		return "";
	
	std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	
	if(!storage_changed_)
	{
		if(ioc::get<Data>().cache().fetch_frame(k,cache_data))
			return cache_data;
		BOOSTER_LOG(debug,__FUNCTION__) << "cache miss for key(" << k << ")";
	}

	try
	{
		std::stringstream ss;
		ss << std::string("SELECT key,data FROM ") << storage << " WHERE key IS NOT NULL;";
		
		cppdb::statement st = DataSql::session() << ss.str();
		cppdb::result r = st.row();

		v = std::string("{");
		do
		{
			if (r.empty()) {
				BOOSTER_LOG(debug,__FUNCTION__) << "value is empty or not exists";
				return "";
			}
			
			r >> r_key >> r_data;
		
			v += std::string("\"") + r_key + "\":" + r_data + ",";
		}
		while(r.next());
		v = v.substr(0, v.size()-1);
		v += std::string("}");
		BOOSTER_LOG(debug,__FUNCTION__) << "value(" << v << ")";

		ioc::get<Data>().cache().store_frame(k,v,OPNCMS_DATA_CACHE_TIMEOUT);
		storage_changed_ = false;
		return ((tools::trim(v) == "null")?"":v);
	}
	catch(std::exception const& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return "";
	}
	BOOSTER_LOG(error,__FUNCTION__) << "There is no data";
	return "";
}

bool DataSql::set(const std::string& storage, cppcms::json::value& value)
{
	std::stringstream ss;
	std::string cache_data, s;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), value(" << tools::json_to_string(value) << ")";

	if(!DataSql::exists(storage) || value.is_undefined() || value.is_null())
		return false;

	std::string ks = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;

	for(cppcms::json::object::const_iterator it=value.object().begin(); it != value.object().end(); ++it)
	{
		std::string it_first = it->first.str();
		std::string it_second = tools::json_to_string(it->second);
		
		std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage + ":" + it_first;
		BOOSTER_LOG(debug,__FUNCTION__) << "key(" << k << ")";

		if( !storage_changed_ && ioc::get<Data>().cache().fetch_frame(k,cache_data) )
		{
			BOOSTER_LOG(debug,__FUNCTION__) << "cached data: (" << cache_data << ")";
			if(cache_data.empty() || cache_data=="")
				ss << std::string("INSERT INTO ") << storage << "(data,key) VALUES(?,?)";
			else
				ss << std::string("UPDATE ") << storage << " SET data=? WHERE key=?;";
		} else {
			s = DataSql::get( storage, it_first );

			if(s.empty() || s == "")
			{
				BOOSTER_LOG(debug,__FUNCTION__) << "insert new value";
				ss << std::string("INSERT INTO ") << storage << "(data,key) VALUES(?,?)";
			} else {
				BOOSTER_LOG(debug,__FUNCTION__) << "update value: (" << s << ")";
				ss << std::string("UPDATE ") << storage << " SET data=? WHERE key=?;";
			}
		}
		
		try {
			cppdb::statement st = DataSql::session() << ss.str();
			st.bind(1, it_second);
			st.bind(2, it_first);
			st.exec();
		
			ioc::get<Data>().cache().store_frame(k, it_second, OPNCMS_DATA_CACHE_TIMEOUT);
		}
		catch(std::exception const& e) {
			BOOSTER_LOG(error,__FUNCTION__) << e.what();
			return false;
		}
		ioc::get<Data>().cache().store_frame(ks, tools::json_to_string(value), OPNCMS_DATA_CACHE_TIMEOUT);
		storage_changed_ = true;
	}
	return true;
}

bool DataSql::set(const std::string& storage, const std::string& key, cppcms::json::value& value)
{
	std::string cache_data, newdata_first, newdata_second;
	cppcms::json::value v;
	bool dotted_exists = false;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << "), value(" << tools::json_to_string(value) << ")";
	
	if ( storage.empty() || key.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage arguments are empty, check it please";
		return false;
	}

	std::string key_first, key_second;
	bool dotted = DataSql::get_dotted(key, key_first, key_second);

	std::string ks = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	std::string k = ks + ":" + key;
	BOOSTER_LOG(debug,__FUNCTION__) << "key(" << k << ")";
	std::stringstream ss;

	//check full key
	if( !storage_changed_ && ioc::get<Data>().cache().fetch_frame(k,cache_data) && !cache_data.empty() )
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "cached data: (" << cache_data << ")";
		v = tools::string_to_json(cache_data);

		if(v.find(key) == value)
			return true;
	}

	DataSql::get(v, storage, key_first); //it is probably cached for the root key
	BOOSTER_LOG(debug,__FUNCTION__) << "data at key_first(" << key_first << ")(" << tools::json_to_string(v) << ")";

	if(dotted) //find data for the second part of the key
	{
		cppcms::json::value vv = v.find(key_second);
		if(!vv.is_undefined() && !vv.is_null())
			dotted_exists = true;
		BOOSTER_LOG(debug,__FUNCTION__) << "data at key_second is " << ( (dotted_exists) ? "exists" : "absent" );
	}

	if(v.find( (dotted) ? key_second : key_first ) == value) {
		BOOSTER_LOG(debug,__FUNCTION__) << "data at key is equal to input";
		return true;
	}
	
	//Prepare the SQL-query - for new (insert) and exists (update) data
	if(v.is_undefined() || v.is_null() || (dotted && !dotted_exists))
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "insert new value";
		ss << std::string("INSERT INTO ") << storage << "(data, key) VALUES(?,?)";
		tools::json_set(v, key, value); //create the new value at full key
		newdata_first = tools::json_to_string(v.find(key_first));
	} else {
		bool matched = (dotted) ? (v.find(key_second) == value) : (v == value);
		
		if(matched) {
			BOOSTER_LOG(debug,__FUNCTION__) << "database value at key is equal to the input value";
			return true;
		}
		BOOSTER_LOG(debug,__FUNCTION__) << "update value";
		ss << std::string("UPDATE ") << storage << " SET data=? WHERE key=?;";

		cppcms::json::value vv;
		vv.at(key_first,v); //TODO: make json_set with no key value
		tools::json_set(vv, key, value);
		newdata_first = tools::json_to_string(vv.find(key_first));
		v = vv;
	}
	try {
		cppdb::statement st = DataSql::session() << ss.str();
		BOOSTER_LOG(debug,__FUNCTION__) << "actual value(" << newdata_first << ")";
		st.bind(1, newdata_first );
		st.bind(2, key_first );
		st.exec();

		DataSql::set_cache(storage, key, v);
	}
	catch(std::exception const& e) {
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	storage_changed_ = true;
	return true;
}

bool DataSql::set(const std::string& storage, const std::string& key, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataSql::set(storage,key,v);
}

bool DataSql::set(const std::string& storage, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataSql::set(storage,v);
}

bool DataSql::erase(const std::string& storage, const std::string& key)
{
	std::string key_first, key_second;
	bool dotted = DataSql::get_dotted(key, key_first, key_second);

	cppcms::json::value v = tools::string_to_json(DataSql::get(storage, key_first));
	if(v.is_undefined() || v.is_null())
		return false; //we even can't get the value at root key

	std::stringstream ss;

	if(dotted)
	{
		tools::json_erase(key_second, v);
		ss << std::string("UPDATE ") << storage << " SET data=? WHERE key=?;";
	}
	else
	{
		//tools::json_erase("", v);
		ss << std::string("DELETE FROM ") << storage << " WHERE key=?;";		
	}
	std::string newdata_first = tools::json_to_string(v);

	try {
		cppdb::statement st = DataSql::session() << ss.str();
		BOOSTER_LOG(debug,__FUNCTION__) << "actual value(" << newdata_first << ")";
		if(dotted) {
			st.bind(1, newdata_first );
			st.bind(2, key_first );
		} else {
			st.bind(1, key_first );
		}
		st.exec();		
	}
	catch(std::exception const& e) {
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	
	if(dotted)
		DataSql::set_cache(storage,key,v,true); //NOTE: the value at key is exists and empty
	else
		ioc::get<Data>().cache().rise(std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage + ":" + key);
	storage_changed_ = true;
	return true;
}

bool DataSql::clear(const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if(storage.empty())
		return false;

	std::string k = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	
	try {
		std::stringstream ss;
		ss << std::string("DROP TABLE IF EXISTS ") << storage;
		if(ioc::get<Data>().driver_name() != "sqlite3")
			ss << " CASCADE;";
		else
			ss << ";";
		cppdb::statement st = DataSql::session() << ss.str();
		st.exec();

		ioc::get<Data>().cache().clear(); //TODO: regex for driver key
		//ioc::get<Data>().cache().rise(k);
	}
	catch(std::exception const& e) {
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
		return false;
	}
	return true;
}

void DataSql::set_cache(const std::string& storage, const std::string& key, const cppcms::json::value& value, bool is_second)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "key(" << key << "), value(" << tools::json_to_string(value) << ")";
	
	if ( key.empty() || value.is_undefined() || value.is_null() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "arguments are empty, check it please";
		return;
	}
	std::string key_first, key_second;
	bool dotted = DataSql::get_dotted(key, key_first, key_second);

	//cache the value for the root/full key
	std::string k_storage = std::string("data:sql:") + ioc::get<Data>().driver_name() + ":" + storage;
	std::string k = k_storage + ":" + key_first;
	std::string k_data = (is_second) ? tools::json_to_string(value) : tools::json_to_string( value.find(key_first) );
	BOOSTER_LOG(debug,__FUNCTION__) << "refresh cache for the root key(" << k << ") = " << k_data;
	ioc::get<Data>().cache().store_frame(k, k_data, OPNCMS_DATA_CACHE_TIMEOUT);

	if(dotted) //cache the rest of next keys 
	{
		while(!key_second.empty())
		{
			k = k_storage + ":" + key_first + "." + key_second;
			k_data = (is_second) ? tools::json_to_string(value.find(key_second)) : tools::json_to_string( value.find(key_first+"."+key_second) );
			BOOSTER_LOG(debug,__FUNCTION__) << "refresh cache for the prev key(" << k << ") = " << k_data;
			ioc::get<Data>().cache().store_frame(k, k_data, OPNCMS_DATA_CACHE_TIMEOUT);
			
			key_second = tools::path_prev(key_second);
		}
	}
	ioc::get<Data>().cache().rise(k_storage); //clear cache for the storage
}

bool DataSql::get_dotted(const std::string& key, std::string& first, std::string& second)
{
	bool dotted = false;
	
	//we should split key into two part - for the root (first) and the rest (second)
	first = tools::split_first(key, ".");
	second = tools::split_second(key, ".");
	
 	if(first.empty())
		first = key;
	else
		dotted = true;
	BOOSTER_LOG(debug,__FUNCTION__) << "first(" << first << "), second(" << second << "), dotted(" << std::boolalpha << dotted << ")";
	return dotted;
}
