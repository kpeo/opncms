////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <string>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <sys/wait.h>
#include <booster/log.h>
#include <cppcms/service.h>
#include <cppcms/applications_pool.h>
#include <opncms/tools.h>
#include <opncms/module/auth.h>


std::map< std::string, void* > ioc::iocmap_;

namespace apps {

class test : public cppcms::application
{
public:  
	test(cppcms::service &/*srv*/);
	//virtual void main(std::string /*url*/);
	virtual void main(std::string /*url*/);
	bool produce(Auth& /*a*/);
	bool test_sha512();
private:
	Data d_;
	Auth a_;
};

bool test::test_sha512()
{
	std::string t = a_.sha512("test");
	//std::cerr << t << std::endl;
	return (t == "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff");
}

bool test::produce(Auth& a)
{
	std::string hash;
	std::cerr << "driver(" << a.type() << ")" << std::endl;

	std::string test_email = "testuser@a.com";
	std::string test_password = "testpassword";
	std::string test_password_wrong = "testpassword123";
	std::string test_password_new = "testpassword1";
	std::string test_name = "testuser";
	std::string test_name_new = "testuser1";

	std::cerr << "sha512: ";
	bool res = test::test_sha512();
	if (!res) return res;

	std::cerr << "Ok" << std::endl << "create: ";
	hash = a.ref().create(test_email, test_password, test_name);
	if( (hash == "") || !a.ref().exists(test_email) )
		return false;
	
	std::cerr << "Ok" << std::endl << "active(new): ";
	if(a.ref().active(test_email))
		return false;
	
	std::cerr << "Ok" << std::endl << "attempts(new): ";
	if(a.ref().attempts(test_email) != -1)
		return false;
	
	std::cerr << "Ok" << std::endl << "check(inactive): ";
	if(a.ref().check(test_email, test_password_wrong))
		return false;
	
	std::cerr << "Ok" << std::endl << "verify: ";
	if(hash == test_email)
		a.ref().verify(a.ref().hash(test_email));
	else
		a.ref().verify(hash);
	if(!a.ref().active(test_email))
		return false;
	
	std::cerr << "Ok" << std::endl << "check(wrong): ";
	if(a.ref().check(test_email, test_password_wrong))
		return false;

	std::cerr << "Ok" << std::endl << "check(correct): ";
	if(!a.ref().check(test_email, test_password))
		return false;

	std::cerr << "Ok" << std::endl << "attempts(reset): ";
	if(a.ref().attempts(test_email) != 0)
		return false;

	//check 5 attempts
	std::cerr << "Ok" << std::endl << "block(attempts): ";
	a.ref().check(test_email, test_password_wrong);
	a.ref().check(test_email, test_password_wrong);
	a.ref().check(test_email, test_password_wrong);
	a.ref().check(test_email, test_password_wrong);
	a.ref().check(test_email, test_password_wrong);
	if(!a.ref().block(test_email))
		return false;

	std::cerr << "Ok" << std::endl << "unblock: ";
	a.ref().block(test_email, false);
	if(a.ref().block(test_email))
		return false;

	std::cerr << "Ok" << std::endl << "block: ";
	a.ref().block(test_email, true);
	if(!a.ref().block(test_email))
		return false;

	a.ref().block(test_email, false); //unblock user
	a.ref().check(test_email, test_password); //reset attempts by successful login

	std::cerr << "Ok" << std::endl << "change password(wrong): ";
	if( a.ref().password(test_email, test_password_wrong, test_password_new) )
		return false;

	std::cerr << "Ok" << std::endl << "change password(correct): ";
	if( !a.ref().password(test_email, test_password, test_password_new) )
		return false;

	std::cerr << "Ok" << std::endl << "check: ";
	if( !(a.ref().check(test_email, test_password_new)) )
		return false;

	std::cerr << "Ok" << std::endl << "change name(wrong): ";
	if( a.ref().name(test_email, test_password, test_name_new) )
		return false;

	std::cerr << "Ok" << std::endl << "change name(correct): ";
	if( !a.ref().name(test_email, test_password_new, test_name_new) )
		return false;

	std::cerr << "Ok" << std::endl << "get email: ";
	if( a.ref().name(test_email) != test_name_new)
		return false;

	std::cerr << "Ok" << std::endl;
	return true;
}

test::test(cppcms::service& srv)
:cppcms::application(srv),
d_(*this),
a_(*this)
{
	ioc::add<Data>(d_);
	ioc::add<Auth>(a_);
}

void test::main(std::string url)
{
	std::map<std::string, std::string> params;
	std::string test_email = "testuser@a.com";
	params["db"] = "test_data.db";
	params["@pool_size"] = "16";
	d_.driver("sqlite3", params);
	d_.driver().erase("users",tools::replace(test_email,".","#"));
	d_.driver().erase("hashes",a_.ref().hash(test_email));
	bool tst_data = test::produce(a_);

	if (tst_data) {
		std::cout << "Success" << std::endl;
		exit(EXIT_SUCCESS);
	} else {
		std::cerr << "Fail " << std::endl;
		exit(EXIT_FAILURE);
	}
}

} //apps

int main()
{
	pid_t child_pid;
	try {
		//cppcms::json::value s = tools::string_to_json("{\"opncms\":{\"auth\":{\"storage\":\"data\",\"method\":\"bcrypt\",\"rounds\":15}, \"data\":{\"driver\":\"sqlite3\"}},\"service\":{\"api\":\"http\",\"port\":8080,\"ip\":\"127.0.0.1\"},\"http\":{\"script\":\"/test\"},\"cache\":{\"backend\":\"process_shared\"},\"logging\":{\"level\":\"info\",\"stderr\":false}}");
		cppcms::json::value s = tools::string_to_json("{\"opncms\":{\"auth\":{\"storage\":\"data\",\"method\":\"bcrypt\",\"rounds\":15}, \"data\":{\"driver\":\"sqlite3\"}},\"service\":{\"api\":\"http\",\"port\":8080,\"ip\":\"127.0.0.1\"},\"http\":{\"script\":\"/test\"},\"cache\":{\"backend\":\"process_shared\"},\"logging\":{\"level\":\"debug\",\"stderr\":true}}");
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
