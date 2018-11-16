////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/bcrypt.h>

std::string bcrypt_salt(unsigned long count)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	char entropy[16];

	std::ifstream f(RANDOM_DEVICE);
	f.read(entropy, sizeof(entropy));
	if (f.gcount() != sizeof(entropy)) {
		BOOSTER_LOG(error,__FUNCTION__) << "Unable to get an entropy";
		f.close();
		return "";
	}
	f.close();
	
	char* retval = crypt_gensalt(CRYPT_PREFIX, count, entropy, sizeof(entropy));
	
	if ( (retval == NULL) || ((retval != NULL) && (retval[0] == '\0')))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Unable to generate a salt";
		return "";
	}
	return std::string(retval);
}

std::string bcrypt_hash(const std::string& key,const std::string& salt)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	void* data = NULL;
	int size;
	
	if(key.empty() && salt.empty())
		return "";
	
	BOOSTER_LOG(debug,__FUNCTION__) << "generate hash";
	//char* retval = crypt_ra(key.c_str(),salt.c_str(),&data,&size);
	std::string ret = crypt_ra(key.c_str(),salt.c_str(),&data,&size);
	//BOOSTER_LOG(debug,__FUNCTION__) << retval;
	//std::string ret(retval);
	free(data);
	if ( ret.empty() )
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Unable to generate a hash";
		return "";
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "hash(" << ret << ")";
	return ret;
}

