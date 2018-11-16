////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <opncms/esmtp.h>

int test_open(const std::string& host, const std::string& user, const std::string& password, const std::string& tlspassword)
{
	std::cout << "open ";
	if(!ESMTP::open(host, user, password, tlspassword))
		return -1;
	std::cerr << "Ok" << std::endl;
	return 0;
}


int test_send(const std::string& user)
{
	std::cout << "send ";

	const char* r[] = {(user+"@localhost").c_str(),"root@localhost"};
	std::vector<std::string> v(r,r+2);
	bool res = ESMTP::send((user+"@localhost"),(user+"@localhost"), v, "opnCMS", "Hi! This is a test message");
	if(!res)
		return -1;
	std::cerr << "Ok" << std::endl;
	return 0;
}

int test_close()
{
	std::cout << "close ";
	ESMTP::close();
	std::cerr << "Ok" << std::endl;
	return 0;
}

std::string input_args(const std::string& name, bool empty)
{
	std::string s;
	std::cout << "Please enter localhost e-mail " << name << ((empty)?" (or '.' followed by enter for none): ":": ");
	std::cin >> s;
	return (empty && s == ".")?"":s;
}

int main(int argc, char* argv[])
{
	std::string user;
	std::string password;
	std::string tlspassword;

	switch(argc){
		case 1:
			user = input_args("username", false);
			password = input_args("password", false);
			tlspassword = input_args("TLS password", true);
			break;
		case 2:
			user = argv[1];
			password = input_args("password", false);
			tlspassword = input_args("TLS password", true);
			break;
		case 3:
			user = argv[1];
			password = argv[2];
			tlspassword = "";
			break;
		case 4:
			user = argv[1];
			password = argv[2];
			tlspassword = argv[3];
	}
	if (test_open("localhost:25", user, password, tlspassword)==0 && test_send(user)==0 && test_send(user)==0 && test_close()==0) {
		std::cout << "Success" << std::endl;
		return EXIT_SUCCESS;
	} else {
		std::cerr << "Fail " << std::endl;
		return EXIT_FAILURE;
	}
}
