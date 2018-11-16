////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/tools.h>

const std::string s = "   test   ";

int test_replace()
{
	std::cerr << "replace";
	const std::string tag = " ,tag1, ";
	const std::string tag_res = " .tag1. ";
	const std::string tag2 = ",tag1,";
	const std::string tag2_res = ".tag1.";
	const std::string tag3 = "tag.tag.tag";
	const std::string tag3_res = "#.#.#";
	std::string t;

	t = tools::replace(tag,",",".");
	if (t != tag_res) {
		std::cerr << "Result tag: [" << t << "]" << std::endl;
		return 1;
	}

	std::cerr << ".";
	t = tools::replace(tag2,",",".");
	if (t != tag2_res) {
		std::cerr << "Result tag2: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	t = tools::replace(tag3,"tag","#");
	if (t != tag3_res) {
		std::cerr << "Result tag3: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_replace_any()
{
	std::cerr << "replace_any";
	const std::string tag = "   tag1, tag2 ,	tag3   ,       tag4 ";
	const std::string tag_res = "tag1+tag2+tag3+tag4";
	std::string t;

	t = tools::replace_any(tag,", 	","+");
	if (t != tag_res) {
		std::cerr << "Result tag: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_trim_left()
{
	std::cerr << "trim_left";
	std::string t = s;
	t = tools::trim_left(t);
	if (t != "test   ") {
		std::cerr << "Result ltrim: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_trim_right()
{
	std::cerr << "trim_right";
	std::string t = s;
	t = tools::trim_right(t);
	if (t != "   test") {
		std::cerr << "Result rtrim: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_trim()
{
	std::cerr << "trim";
	std::string t = s;
	t = tools::trim(t);
	if (t != "test") {
		std::cerr << "Result trim: [" << t << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_str2hex()
{
	std::cerr << "str2hex";
	std::string s("09aAzZ.:");
	std::string d;
	tools::str2hex(s,d);

	if (d != "303961417a5a2e3a")
	{
		std::cerr << "Result str2hex: [" << d << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_hex2str()
{
	std::cerr << "hex2str";
	std::string s("303961417a5a2e3a");
	std::string d;
	
	if (!tools::hex2str(s,d))
	{
		std::cerr << "Error convetring hex2str: [" << s << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	if (d != "09aAzZ.:")
	{
		std::cerr << "Result hex2str: [" << d << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	s.erase(0,1);
	if (tools::hex2str(s,d))
	{
		std::cerr << "Error convetring hex2str: [" << s << "]" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_split_first()
{
	std::cerr << "split_first";
	std::string s(" .tag1.tag2.tag3.");
	std::string s2("tag1.tag2.tag3.");

	std::string res = tools::split_first(s,".");

	if (res != " ")
		return 1;

	std::cerr << ".";
	res = tools::split_first(s2,".");

	if (res != "tag1")
		return 1;

	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_split_second()
{
	std::cerr << "split_second";
	std::string s(" .tag1.tag2.tag3.");
	std::string s2("tag1.tag2.tag3.");

	std::string res = tools::split_second(s,".");

	if (res != "tag1.tag2.tag3.")
		return 1;

	std::cerr << ".";
	res = tools::split_second(s2,".");

	if (res != "tag2.tag3.")
		return 1;

	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_split()
{
	std::cerr << "split";
	std::string s(" :tag1:tag2:tag3:");
	std::vector<std::string> tst;

	tst.push_back(" ");
	tst.push_back("tag1");
	tst.push_back("tag2");
	tst.push_back("tag3");
	tst.push_back("");
	std::vector<std::string> res = tools::split(s,":");
/*
	//DEBUG
	for(std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it) {
		std::cerr << "Result: [" << *it << "]" << std::endl;
	}
*/
	if ((res != tst) || (res.size() != tst.size()))
		return 1;
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_vexec()
{
	std::cerr << "vexec";
	std::string s("echo test;");
	std::string tst("test\n");
	std::string res = tools::vexec(s,0);
	
	if (res != tst)
	{
		std::cerr << "Result of vexec(" << res << ") doesn`t equal to test data(" << tst << ")" << std::endl;
		return 1;
	}
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_json_set()
{
	std::cerr << "json_set" << std::endl;
	std::string t = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\"],\"d\":\"tst1\",\"e\":\"tst2\"}}}";
	std::string t1 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\"],\"d\":\"tst1\",\"e\":\"tst2\"}}}";
	std::string t2 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\"],\"d\":\"tst1\",\"e\":\"tst2\",\"f\":\"tst3\"}}}";
	std::string t3 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\"],\"d\":\"tst2\",\"e\":\"tst2\",\"f\":\"tst3\"}}}";
	std::string t4 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\",[\"1\",\"2\"]],\"d\":\"tst2\",\"e\":\"tst2\",\"f\":\"tst3\"}}}";
	std::string t5 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\",[\"1\",\"2\"]],\"d\":\"tst2\",\"e\":\"tst2\",\"f\":[\"1\",\"2\"]}}}";
	std::string t6 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\",[\"1\",\"2\"],{\"x\":\"1\",\"y\":\"2\"}],\"d\":\"tst2\",\"e\":\"tst2\",\"f\":[\"1\",\"2\"]}}}";
	std::string t7 = "{\"a\":{\"b\":{\"c\":[\"foo\",\"bar\",\"baz\",[\"1\",\"2\"],{\"x\":\"2\",\"y\":\"2\"}],\"d\":\"tst2\",\"e\":\"tst2\",\"f\":[\"1\",\"2\"]}}}";
	cppcms::json::value v = tools::string_to_json(t);

	std::cerr << "  value->array(none): ";
	tools::json_set(v,"a.b.c","foo");
	if(tools::json_to_string(v) != t)
		return 1;
	std::cerr << "Ok" << std::endl;
	std::cerr << "  value->object(none): ";
	tools::json_set(v,"a.b.d","tst1");
	if(tools::json_to_string(v) != t)
		return 1;

	std::cerr << "Ok" << std::endl;
	std::cerr << "  value->array(add): ";
	tools::json_set(v,"a.b.c","baz");
	if(tools::json_to_string(v) != t1)
		return 1;

	std::cerr << "Ok" << std::endl;
	std::cerr << "  value->object(add): ";
	tools::json_set(v,"a.b.f","tst3");
	if(tools::json_to_string(v) != t2)
		return 1;

	std::cerr << "Ok" << std::endl;
	std::cerr << "  value->object(replace): ";
	tools::json_set(v,"a.b.d","tst2");
	if(tools::json_to_string(v) != t3)
		return 1;

	cppcms::json::array a;
	a.push_back("1");
	a.push_back("2");
	
	std::cerr << "Ok" << std::endl;
	std::cerr << "  array->array(add): "; //[x,y] -> [x,y,[]]
	tools::json_set(v,"a.b.c",a);
	if(tools::json_to_string(v) != t4)
		return 1;
	
	std::cerr << "Ok" << std::endl;
	std::cerr << "  array->object(replace): "; //{x:y} -> {x:[]}
	tools::json_set(v,"a.b.f",a);
	if(tools::json_to_string(v) != t5)
		return 1;

	cppcms::json::object obj;
	obj["x"] = "1";
	obj["y"] = "2";

	std::cerr << "Ok" << std::endl;
	std::cerr << "  object->array(add): "; //[x,y] -> [x,y,{}]
	tools::json_set(v,"a.b.c",obj);
	if(tools::json_to_string(v) != t6)
		return 1;

	cppcms::json::object obj1;
	obj1["x"] = "1";

	std::cerr << "Ok" << std::endl;
	std::cerr << "  object->array(none): ";
	tools::json_set(v,"a.b.c",obj1);
	if(tools::json_to_string(v) != t6)
		return 1;

	obj1["x"] = "2";
	std::cerr << "Ok" << std::endl;
	std::cerr << "  object->object(replace): "; //{x:y} -> {x:{}}
	tools::json_set(v,"a.b.c",obj1);
	if(tools::json_to_string(v) != t7)
		return 1;

	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_hash()
{
	std::cerr << "hash";
	std::auto_ptr<cppcms::crypto::message_digest> d(cppcms::crypto::message_digest::create_by_name("md5"));
	std::string test = " _01234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string test_res = "8a619013f55c75dfa600d94b68e0d60f";
	std::string res = tools::string_hash(test, "md5");

	if(res != test_res)
		return 1;

	std::cerr << ".";

	std::stringstream is(test);
	res = tools::stream_hash(is, "md5");

	if(res != test_res)
		return 1;

	std::cerr << ".";
	
	booster::nowide::ofstream ofs;
	ofs.open("test_hash");
	ofs << test;
	ofs.close();
	res = tools::file_hash("test_hash", "md5");

	if(res != test_res)
		return 1;

	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_random()
{
	std::cerr << "random";
	unsigned long rnd[10];
	for(int i=0; i<10; i++)
	{
		rnd[i] = tools::get_random();
		//std::cerr << i << "(" << rnd[i] << "): ";

		if(rnd[i] == 0)
			return 1;

		int j = i;
		while(j--)
		{
			//std::cerr << j << "(" << rnd[j] << ") ";
			if( (rnd[j] == rnd[i]) )
				return 1;
		}
		//std::cerr << std::endl;
	}
	std::cerr << ".";
	
	std::string a = tools::get_random(100000);
	std::string b = tools::get_random(100000);
	//TODO: check the correlation using Pearson's coefficient
	if(a==b)
		return 1;
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_local()
{
	std::cerr << "is_local";
	tools::if_ip_list ip_list;
	if(ip_list.empty())
		return 1;
	std::cerr << ".";
	if(!tools::is_ip("127.0.0.1"))
		return 1;
	std::cerr << ".";
	if(!ip_list.find("127.0.0.1"))
		return 1;
	std::cerr << ".";
	if(!tools::is_local(ip_list,"127.0.0.1"))
		return 1;
	std::cerr << ".";
	std::cerr << "Ok" << std::endl;
	return 0;
}

int main()
{
	if (test_replace()==0 && test_replace_any()==0 &&
	test_trim_left()==0 && test_trim_right()==0 && test_trim()==0 &&
	test_str2hex()==0 &&
	test_split()==0 && test_split_first()==0 && test_split_second()==0 &&
	test_vexec()==0 &&
	test_json_set()==0 &&
	test_hash()==0 &&
	test_random()==0 &&
	test_local()==0
	)
	{
		std::cout << "Success" << std::endl;
		return EXIT_SUCCESS;
	} else {
		std::cerr << "Fail" << std::endl;
		return EXIT_FAILURE;
	}
}
