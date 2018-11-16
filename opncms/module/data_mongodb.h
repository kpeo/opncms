////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_DATA_MONGODB_H
#define OPNCMS_M_DATA_MONGODB_H

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#if __cplusplus>=201103L
#pragma GCC diagnostic ignored "-Wpedantic"
#else
#pragma GCC diagnostic ignored "-pedantic"
#endif
#include "mongo/client/dbclient.h"
#include "mongo/db/json.h"
#include "mongo/bson/bsonobj.h"
#include "mongo/bson/bsonelement.h"
#include "mongo/bson/bsonobjiterator.h"
#include "mongo/bson/bsonobjbuilder.h"
#pragma GCC diagnostic pop

//NOTE: The scope of BSONElement based on its BSONObj
namespace cppcms {
namespace json {
	template<>
	struct traits<mongo::BSONObj> {

	static void set(value &v, mongo::BSONObj const &in)
	{
		mongo::BSONElement el = in.firstElement();
		mongo::BSONType t = el.type();
		
		if(in.isEmpty())
			return;
		if(el.isNull()) {
		std::cerr << "null" << std::endl;
			v.set(el.fieldName(),null());
			return;
		}
		
		switch(t)
		{
			case (mongo::Array): {
				std::vector<value> a;
				value b;
				std::vector<mongo::BSONElement> arr = el.Array();
				for(std::vector<mongo::BSONElement>::iterator it = arr.begin(); it != arr.end(); ++it) {
					//here we don't handle the recursion cases
					if(it->type() == mongo::NumberDouble || it->type() == mongo::NumberLong || it->type() == mongo::NumberInt)
						b.number( it->Number() );
					else
						b.str(it->toString());
					a.push_back(b);
				}
				v.set(el.fieldName(),a);
				break;
			}
			case (mongo::String): {
				v.set(el.fieldName(),el.String());
				break;
			}
			case (mongo::NumberDouble):
			case (mongo::NumberLong):
			case (mongo::NumberInt): {
				v.set(el.fieldName(),el.Number());
				break;
			}
			case(mongo::Bool): {
				v.set(el.fieldName(),el.Bool());
				break;
			}
			default: {
				//TODO: see mongo::BSONElement::Hasher for the Obj() case
				int line = 0;
				std::string s = in.jsonString(mongo::Strict, false);
				std::istringstream ss(s);
				if(!v.load(ss,true,&line)) {
					std::ostringstream tmp;
					tmp << "Parsing error of " << s << " in line " << line;
					throw bad_value_cast(tmp.str());
				}
			}
		}
	}
	
	static mongo::BSONObj get(value const &v)
	{
		mongo::BSONObj obj;
		mongo::BSONElement el;
		mongo::BSONObjBuilder objb;

		switch(v.type())
		{
			/* TODO
			case is_array:
				std::vector<mongo::BSONElement> arr;
				mongo::BSONElement tmp_el;
				for(std::vector<value>::const_iterator it = v.array().begin(); it != v.array().end(); ++it) {
					tmp_el = get(*it);
					arr.push_back(tmp_el);
				}
				el.Val(arr); //not a way
				break;
			 */
			case is_string: {
				std::string str = v.get_value<std::string>();
				el.Val(str);
				objb.append(el);
				break;
			}
			case is_number: {
				double d = v.get_value<double>();
				el.Val(d);
				objb.append(el);
				break;
			}
			case is_boolean: {
				bool b = v.get_value<bool>();
				el.Val(b);
				objb.append(el);
				break;
			}
			case is_null: {
				el.Null();
				objb.append(el);
				break;
			}
			case is_object: //TODO: see mongo::BSONElement::Hasher
			case is_array:
			default: {
				std::ostringstream ss;
				ss << v;
				obj = mongo::fromjson(ss.str());
			}
		}
		return obj;
	}
	};
}}

class DataMongodb : public IData
{
public:
	DataMongodb();
	~DataMongodb();
	bool init(const std::string& /*driver*/, tools::map_str& /*params*/);
	bool is_sql();
	bool exists(const std::string& /*storage*/);
	bool exists(const std::string& /*storage*/, const std::string& /*key*/);
	int count(const std::string& /*storage*/, const std::string& /*key*/);
	bool create(const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/);
	bool get(cppcms::json::value& /*value*/, const std::string& /*storage*/, const std::string& /*key*/);
	std::string get(const std::string& /*storage*/, const std::string& /*key*/);
	std::string get(const std::string& /*storage*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, cppcms::json::value& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*value*/);
	bool set(const std::string& /*storage*/, const std::string& /*value*/);
	bool erase(const std::string& /*storage*/, const std::string& /*key*/);
	bool clear(const std::string& /*storage*/);
protected:
	mongo::DBClientConnection conn_;
	std::string path_;
	std::string host_;
	std::string db_;
};

#endif
