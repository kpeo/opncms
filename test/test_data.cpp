////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <sys/mman.h>
#include <sys/file.h>
#include <sys/wait.h>
 
#include <cppcms/service.h>
#include <cppcms/applications_pool.h>

#include <booster/log.h>

#include <opncms/module/data.h>
//#include <opncms/module/data_mongodb.h>

std::map< std::string, void* > ioc::iocmap_;

namespace apps {

//file test data
std::string test1_all = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\"]}}}";
std::string test1_key = "a.b.c";
std::string test1_value = "[\"foo\",\"bar\"]";

std::string test2_all = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\"],\"d\":\"tst1\",\"e\":\"tst2\"}}}";
std::string test2_set = "{\"a\":{\"b\":{\"d\":\"tst1\",\"e\":\"tst2\"}}}";
std::string test2_add = "{\"c\":[\"foo\",\"bar\"]}";
std::string test2_key = "a.b";
std::string test2_value = "{\"d\":\"tst1\",\"e\":\"tst2\"}";

std::string test3_all = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\"],\"d\":\"tst1\",\"e\":[1,2.5,3.7],\"f\":true}}}";

int test_to_mongo_check(const std::string& str, const std::string& check)
{
	cppcms::json::value v = tools::string_to_json(str);
	mongo::BSONObj b = v.get_value<mongo::BSONObj>();
	if(b.isEmpty())
	{
		std::cerr << "Result is empty" << std::endl;
		return false;
	}
	std::string s = tools::replace_any(b.jsonString(), " ", "");
	if (s != check) {
		std::cerr << "Converted string (" << s << ") != (" << check << ")" << std::endl;
		return false;
	}
	std::cerr << ".";
	return true;
}

bool test_to_json_check(const std::string& str, const std::string& check)
{
	cppcms::json::value v;
	mongo::BSONObj b = mongo::fromjson(str);
	v.set_value<mongo::BSONObj>(b);
	if(v.is_undefined() || v.is_null())
	{
		std::cerr << "Result is empty" << std::endl;
		return false;
	}
	std::string s = tools::json_to_string(v);
	if (s != check) {
		std::cerr << "Converted string (" << s << ") != (" << check << ")" << std::endl;
		return false;
	}
	std::cerr << ".";
	return true;
}

int test_mongo()
{
	BOOSTER_LOG(debug,__FUNCTION__) << "test_to_mongo";

	if(!test_to_mongo_check(test3_all,test3_all) ||
		!test_to_mongo_check("{\"a\":true}","{\"a\":true}") ||
		!test_to_mongo_check("{\"a\":1.5}","{\"a\":1.5}") ||
		!test_to_mongo_check("{\"a\":\"test\"}","{\"a\":\"test\"}") ||
		!test_to_mongo_check("{\"a\":[1,2.5]}","{\"a\":[1,2.5]}") ||
		!test_to_mongo_check("{\"a\":null}","{\"a\":null}")
		)
		return false;

	std::cerr << "Ok" << std::endl;
	BOOSTER_LOG(debug,__FUNCTION__) << "test_to_json";

	if(!test_to_json_check(test3_all,test3_all) ||
		!test_to_json_check("{\"a\":true}","{\"a\":true}") ||
		!test_to_json_check("{\"a\":1.5}","{\"a\":1.5}") ||
		!test_to_json_check("{\"a\":\"test\"}","{\"a\":\"test\"}") ||
		!test_to_json_check("{\"a\":[1,2.5]}","{\"a\":[1,2.5]}") ||
		!test_to_json_check("{\"a\":null}","{\"a\":null}")
		)
		return false;

	std::cerr << "Ok" << std::endl;
	return true;
}

void error_and_die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

class test : public cppcms::application
{
public:  
	test(cppcms::service &/*srv*/);
	virtual void main(std::string /*url*/);
	int create(Data& /*d*/, const std::string& /*storage*/);
	int exists(Data& d, const std::string& /*storage*/);
	int set(Data& d, const std::string& /*storage*/, const std::string& /*value*/);
	int set(Data& /*d*/, const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*value*/);
	int get(Data& /*d*/, const std::string& /*storage*/, const std::string& /*key*/, const std::string& /*test_value*/);
	int get(Data& /*d*/, const std::string& /*storage*/, const std::string& /*test_value*/);
	int erase(Data& /*d*/, const std::string& /*storage*/, const std::string& /*key*/);
	int clear(Data& /*d*/, const std::string& /*storage*/);
	bool produce(Data& /*d*/, const std::string& /*storage*/);
private:
	Data d_;
};

int test::create(Data& d, const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "]";

	if (d.driver().create(storage)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in create ";
	return 1;
}

int test::exists(Data& d, const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "]";

	if (d.driver().exists(storage)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in exists ";
	return 1;
}

int test::set(Data& d, const std::string& storage, const std::string& key, const std::string& value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "], key[" << key << "], value[" << value << "]";

	if (d.driver().set(storage,key,value)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in saving ";
	return 1;
}

int test::set(Data& d, const std::string& storage, const std::string& value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "], value[" << value << "]";

	if (d.driver().set(storage,value)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in saving ";
	return 1;
}

int test::get(Data& d, const std::string& storage, const std::string& key, const std::string& test_value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "], key[" << key << "], test[" << test_value << "]";
	
	std::string s = d.driver().get(storage,key);
	
	if (s == test_value) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << std::string("Result(")+s+") != Test("+test_value+") ";
	return 1;
}

int test::get(Data& d, const std::string& storage, const std::string& test_value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "], test[" << test_value << "]";
	
	std::string s = d.driver().get(storage);
	
	if (s == test_value) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << std::string("Result(")+s+") != Test("+test_value+") ";
	return 1;
}

int test::erase(Data& d, const std::string& storage, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "], key[" << key << "]";

	if (d.driver().erase(storage,key)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in erase ";
	return 1;
}

int test::clear(Data& d, const std::string& storage)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "storage[" << storage << "]";

	if (d.driver().clear(storage)) {
		std::cerr << "Ok ";
		return 0;
	}
	std::cerr << "Error in clear ";
	return 1;
}

bool test::produce(Data& d, const std::string& storage)
{
	std::cerr << "driver[" << d.driver_name() << "]" << std::endl;

	if(test::exists(d,storage)==0)
		test::clear(d,storage);

	std::cerr << std::endl << "create: ";
	bool res = (test::create(d,storage)==0 && test::exists(d,storage)==0);
	if (!res) return res;

	std::cerr << std::endl << "set (storage): ";
	res = (test::set(d,storage,test1_all)==0 && test::get(d,storage,test1_all)==0);
	if (!res) return res;

	std::cerr << std::endl << "set (add): ";
	res = res && (test::set(d,storage,test2_key,test2_value)==0 && test::get(d,storage,test2_all)==0);
	if (!res) return res;

	std::cerr << std::endl << "erase: ";
	res = res && (test::erase(d,storage,test1_key)==0 && test::get(d,storage,test1_key,"")==0);
	if (!res) return res;

	std::cerr << std::endl << "set (replace): ";
	res = res && (test::set(d,storage,test2_key,test2_add)==0 && test::get(d,storage,test2_all)==0);
	if (!res) return res;

	std::cerr << std::endl << "clear: ";
	res = res && (test::clear(d,storage)==0 && test::get(d,storage,"")==0);
	if (!res) return res;

	std::cerr << std::endl << "create: ";
	res = (test::create(d,storage)==0 && test::exists(d,storage)==0);
	if (!res) return res;
	
	std::cerr << std::endl << "get: ";
	res = res && (test::set(d,storage,test1_key,test1_value)==0 && test::get(d,storage,test1_key,test1_value)==0);
	if (!res) return res;

	std::cerr << std::endl << "set (twice): ";
	res = res && (test::set(d,storage,test1_key,test1_value)==0 && test::get(d,storage,test1_all)==0);
	if (!res) return res;

	std::cerr << std::endl;
	return res;
}

test::test(cppcms::service& srv)
:cppcms::application(srv),
d_(*this)
{
	ioc::add<Data>(d_);
}

void test::main(std::string url)
{	
	BOOSTER_LOG(debug,__FUNCTION__) << "url(" << url << ")";
	
	if ( !test_mongo() )
	{
		std::cerr << "Fail " << std::endl;
		exit(EXIT_FAILURE);
	}

	std::map<std::string, std::string> params;

	//test file store
	params["path"] = "/tmp";
	d_.driver("file",params);
	bool tst_file = test::produce(d_,"test_data.js");

	params.clear();

	//test sql store
	params["db"] = "test_data.db";
	params["@pool_size"] = "16";
	d_.driver("sqlite3", params);
	bool tst_sql = test::produce(d_,"test");

	params.clear();

	//test mongodb store
	params["host"] = "localhost";
	params["db"] = "opncms";
	d_.driver("mongodb",params);
	bool tst_mongodb = test::produce(d_,"test");

	if (tst_file && tst_sql && tst_mongodb) {
		std::cout << "Ok" << std::endl;
		exit(EXIT_SUCCESS);
	} else {
		std::cerr << "Fail " << std::endl;
		exit(EXIT_FAILURE);
	}
}
} //test

int main()
{
	pid_t child_pid;
	try 
	{
#ifdef _OPNCMS_DEBUG_
		cppcms::json::value s = tools::string_to_json("{\"service\":{\"api\":\"http\",\"port\":8080,\"ip\":\"127.0.0.1\"},\"http\":{\"script\":\"/test\"},\"cache\":{\"backend\":\"process_shared\"},\"logging\":{\"level\":\"debug\",\"stderr\":true}}");
#else
		cppcms::json::value s = tools::string_to_json("{\"service\":{\"api\":\"http\",\"port\":8080,\"ip\":\"127.0.0.1\"},\"http\":{\"script\":\"/test\"},\"cache\":{\"backend\":\"process_shared\"},\"logging\":{\"level\":\"debug\",\"stderr\":false}}");
#endif
		cppcms::service app(s);
		app.applications_pool().mount(cppcms::applications_factory<apps::test>() );
		switch(child_pid = fork())
		{
			case -1:
				 perror("fork");
				 exit(1);
			case 0:
				sleep(1);
				std::cerr<<tools::vexec("wget -q --no-cache --tries=1 http://127.0.0.1:8080/test",0);
				sleep(1);
				exit(0);
			default:
				app.run();
				wait(NULL);
		}
		//app.shutdown();
	}
	catch(std::exception const &e) {
		BOOSTER_ERROR("opncms") << e.what();
		std::cerr<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
