////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/data.h>

#define OPNCMS_MONGODB "opncms"

DataMongodb::DataMongodb()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	mongo::client::initialize();
}

DataMongodb::~DataMongodb()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	mongo::Status s = mongo::client::shutdown();
}

bool DataMongodb::init(const std::string& driver, tools::map_str &params)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "driver(" << driver << ")";
	path_ = params["path"];
	host_ = params["host"];
	db_ = params["db"];
	
	//use default values
	if (path_.empty())
		path_ = OPNCMS_DATA_FILE_PATH;
	if (db_.empty())
		path_ = OPNCMS_MONGODB;
	if (host_.empty())
		host_ = "localhost";
	
	try {
		conn_.connect(host_);
		BOOSTER_LOG(debug,__FUNCTION__) << "Connected successfully";
	}
	catch( const mongo::DBException &e ) {
		BOOSTER_LOG(debug,__FUNCTION__) << "Caught " << e.what();
		return false;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "path(" << path_ << ")";
	return true;
}

bool DataMongodb::is_sql()
{
	return false;
}

bool DataMongodb::exists(const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";
	return conn_.exists(db_+"."+storage);
}

bool DataMongodb::exists(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;
	std::stringstream ss;
	mongo::BSONObj b;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() || key.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage/key are empty";
		return "";
	}

	if( conn_.exists(db_+"."+storage) )
	{
		std::string key_first = tools::split_first(key,".");
		if(key_first.empty())
			return !conn_.findOne( db_+"."+storage, mongo::Query("{}") ).hasField(key);
		else
		{
			b = conn_.findOne( db_+"."+storage, mongo::Query("{}") ).getField(key_first).Obj();
			if(b.isEmpty())
				return false;
			return !b.getFieldDotted(tools::split_second(key,".")).isNull();
		}
	}
	return false;
}

bool DataMongodb::create(const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";
	if( conn_.exists(db_+"."+storage) )
		return DataMongodb::clear(storage);
	else
		return conn_.createCollection(db_+"."+storage);
	return false;
}

bool DataMongodb::set(const std::string& storage, const std::string& key, cppcms::json::value& value)
{
	cppcms::json::value v;
	mongo::BSONObj b;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << "), value(" << tools::json_to_string(value) << ")";

	if ( storage.empty() || key.empty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage/key arguments are empty, check them please";
		return false;
	}

	//save only different values
	if( conn_.exists(db_+"."+storage) )
	{
		b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );
		v = tools::string_to_json(b.jsonString());
		if(!v.find(key).is_undefined())
		{
			if((value.type() == cppcms::json::is_array) && (v.at(key).type() == cppcms::json::is_array))
			{
				BOOSTER_LOG(debug,__FUNCTION__) << "Value at key is array(" << tools::json_to_string(v.at(key)) << ")";
				for(cppcms::json::array::const_iterator it=value.array().begin(); it != value.array().end();it++)
				{
					if (std::find(v.at(key).array().begin(), v.at(key).array().end(), *it) == v.at(key).array().end())
					{
						BOOSTER_LOG(debug,__FUNCTION__) << "Add value(" << *it << ")";
						v.at(key).array().push_back(*it);
					}
				}
			} else {
				if((value.type() == cppcms::json::is_object) && (v.at(key).type() == cppcms::json::is_object))
				{
					BOOSTER_LOG(debug,__FUNCTION__) << "Value at key is object(" << tools::json_to_string(v.at(key)) << ")";
					for(cppcms::json::object::const_iterator it=value.object().begin();it!=value.object().end();it++)
					{
						if(v.at(key).object().find(it->first) == v.at(key).object().end())
						{
							BOOSTER_LOG(debug,__FUNCTION__) << "Add value(" << it->first << ":" << it->second << ")";
							v.at(key).object().insert(std::pair<std::string,cppcms::json::value>(it->first,it->second));
						}
						else
							v.at(key).object()[it->first] = it->second;
					}
				}
				else
					BOOSTER_LOG(error,__FUNCTION__) << "Expected array or object value type, current is (" << value.type() << ")";
			}
		} 
		else
			v.set<cppcms::json::value>(key,value); //can't load value or it is undefined - set new one
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value: " << tools::json_to_string(v).substr(0,32)+"...";
	b = mongo::fromjson(tools::json_to_string(v));
	conn_.save(db_+"."+storage, b);
	return true;
}

bool DataMongodb::set(const std::string& storage, cppcms::json::value& value)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), value(" << tools::json_to_string(value) << ")";

	if ( storage.empty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage/key arguments are empty, check them please";
		return false;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value";
	mongo::BSONObj b = mongo::fromjson(tools::json_to_string(value));
	conn_.save(db_+"."+storage, b);
	return true;
}

bool DataMongodb::set(const std::string& storage, const std::string& key, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataMongodb::set(storage,key,v);
}

bool DataMongodb::set(const std::string& storage, const std::string& value)
{
	cppcms::json::value v;
	v = tools::string_to_json(value);
	return DataMongodb::set(storage,v);
}

bool DataMongodb::get(cppcms::json::value& v, const std::string& storage, const std::string& key)
{
	std::stringstream ss;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage is empty";
		return false;
	}

	if( conn_.exists(db_+"."+storage) )
	{
		mongo::BSONObj b;
		if ( key.empty() )
			b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );
		else
		{
			b = conn_.findOne( db_+"."+storage, mongo::Query("{}") ).getFieldDotted(key).Obj();
			if(!b.isEmpty())
			{
				if(b.firstElement().type() != mongo::Object)
				{
					BOOSTER_LOG(debug,__FUNCTION__) << "BSON Type is not an Object";
					v.set_value<mongo::BSONObj>(b);
					return true;
				}
				/*
				if(el.type() == mongo::Array)
				{
					cppcms::json::array a;
					std::vector<mongo::BSONElement> vv = el.Array();

					for(std::vector<mongo::BSONElement>::iterator it = vv.begin(); it != vv.end(); ++it)
						//a.push_back(it->)
						s += it->jsonString(mongo::Strict, false)+",";
					//cppcms::json::value v = tools::string_to_json(s);
					//v = v.at(tools::split_last(key,"."));
					//return tools::json_to_string(v);
					
					return true;
					return s.substr(0,s.size()-1)+"]";
				}

				if(el.type() != mongo::Object)
					return el.jsonString(mongo::Strict);
				*/
				return true;
			}
			else
				return false; //even if result="{}"
		}
		
		BOOSTER_LOG(debug,__FUNCTION__) << "Get object(" << b.jsonString() << ")";

		if(b.hasField("_id"))
			v = tools::string_to_json(b.removeField("_id").jsonString());
		else
			v = tools::string_to_json(b.jsonString());

		if(v.is_null() || v.is_undefined())
		{
			BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage at key";
			return false;
		}
	}
	return true;
}

std::string DataMongodb::get(const std::string& storage, const std::string& key)
{
	cppcms::json::value v;
	std::stringstream ss;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage is empty";
		return "";
	}

	if( conn_.exists(db_+"."+storage) )
	{
		mongo::BSONObj b;
		if ( key.empty() )
			b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );
		else
		{
			mongo::BSONElement el = conn_.findOne( db_+"."+storage, mongo::Query("{}") ).getFieldDotted(key);
			if(el.ok())
			{
				if(el.type() == mongo::Array)
				{
					std::string s = "[";
					//std::string s = el.jsonString(mongo::Strict);
					//TODO: look for the better solution to get exact value w/o key
					std::vector<mongo::BSONElement> vv = el.Array();

					for(std::vector<mongo::BSONElement>::iterator it = vv.begin(); it != vv.end(); ++it)
						s += it->jsonString(mongo::Strict, false)+",";
					//cppcms::json::value v = tools::string_to_json(s);
					//v = v.at(tools::split_last(key,"."));
					//return tools::json_to_string(v);
					return s.substr(0,s.size()-1)+"]";
				}

				if(el.type() != mongo::Object)
					return el.jsonString(mongo::Strict);
			}
			else
				return ""; //even if result="{}"
			b = el.Obj();
		}
		
		BOOSTER_LOG(error,__FUNCTION__) << "Get object(" << b.jsonString() << ")";

		if(b.hasField("_id"))
			v = tools::string_to_json(b.removeField("_id").jsonString());
		else
			v = tools::string_to_json(b.jsonString());

		if(v.is_null() || v.is_undefined())
		{
			BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage at key";
			return "";
		}
	}
	ss << v;
	return ss.str();
}

bool DataMongodb::get(cppcms::json::value& v, const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if ( storage.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "There is no storage";
		return false;
	}

	if( !conn_.exists(db_+"."+storage) )
		return false;

	BOOSTER_LOG(debug,__FUNCTION__) << "Try to get data";
	mongo::BSONObj b;
	b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );
	
	if(b.isEmpty())
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Storage is empty";
		return false;
	}
	b.removeField("_id");
	v.set_value<mongo::BSONObj>(b);

	return true;
}

std::string DataMongodb::get(const std::string& storage)
{
	cppcms::json::value v;
	std::stringstream ss;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if ( storage.empty() ) {
		BOOSTER_LOG(error,__FUNCTION__) << "storage is empty";
		return "";
	}

	if( !conn_.exists(db_+"."+storage) )
		return "";

	BOOSTER_LOG(debug,__FUNCTION__) << "Try to get data";
	
	mongo::BSONObj b;
	b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );
	v = tools::string_to_json(b.removeField("_id").jsonString());

	if(v.is_null() || v.is_undefined())
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t load storage at key";
		return "";
	}
	ss << v; //load whole storage
	return ss.str();
}

bool DataMongodb::erase(const std::string& storage, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << "), key(" << key << ")";

	if ( storage.empty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage argument is empty, check it please";
		return false;
	}

	if ( key.empty() )
		return conn_.dropCollection(db_+"."+storage);

	if( !conn_.exists(db_+"."+storage) )
		return false;

	mongo::BSONObj b = conn_.findOne( db_+"."+storage, mongo::Query("{}") );

	if( b.isEmpty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can`t get value at key";
		return false;
	}
	//b = b.removeFieldDotted(key); //TODO
	cppcms::json::value v = tools::string_to_json(b.jsonString());
	tools::json_erase(key, v);
	BOOSTER_LOG(debug,__FUNCTION__) << "Try to save changed value";
	b = mongo::fromjson(tools::json_to_string(v));
	conn_.save(db_+"."+storage, b);
	return true;
}

bool DataMongodb::clear(const std::string& storage)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__) << "storage(" << storage << ")";

	if ( storage.empty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "The storage argument is empty, check it please";
		return false;
	}
	conn_.dropCollection(db_+"."+storage);
	return true;
}
