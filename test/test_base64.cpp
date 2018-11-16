////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <algorithm>
#include <opncms/base64.h>

int main() {
	const std::string s = "opnCMS\nC++ frontend for CppCMS web-framework";

	std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
	std::string decoded = base64_decode(encoded);

	std::cout << "encoded: " << encoded << std::endl;
	std::cout << "decoded: " << decoded << std::endl;

	if (decoded == s)
	{
		std::cout << "Ok" << std::endl;
		return EXIT_SUCCESS;
	} else {
		std::cerr << "Fail " << std::endl;
		return EXIT_FAILURE;
	}
}
