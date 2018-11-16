////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/module/auth.h>

AuthData::AuthData()
{}

Auth::Auth(cppcms::application &app)
: app_(app),
data_()
//type_done_(false),
//authed_(false),
//local_(false),
//user_type_(0)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	//FIXME: avoid hardcode values
	type_ = app_.settings().get<std::string>("opncms.auth.storage","data");
	if (!Auth::type_.empty()) {
		Auth::driver_ = app_.settings().get<std::string>(std::string("opncms.")+type_+".driver");
		BOOSTER_LOG(debug,__FUNCTION__) << "type(" << Auth::type_ << "), driver(" << Auth::driver_ << ")";
		if (Auth::type_ == "data") {
			BOOSTER_LOG(debug,__FUNCTION__) << "init data";
			ioc::add<IAuth>(Auth::data_, std::string("Auth:")+Auth::type_);
		}
	}
	method_ = app_.settings().get<std::string>("opncms.auth.method","bcrypt");
	BOOSTER_LOG(debug,__FUNCTION__) << "method(" << method_ << ")";
}

Auth::~Auth()
{
	BOOSTER_LOG(debug,__FUNCTION__);
}

//return reference to type instance
IAuth& Auth::ref()
{
	return ioc::get<IAuth>(std::string("Auth:")+Auth::type_);
}

//return setting's value "opncms._type_.driver"
const std::string& Auth::driver()
{
	return Auth::driver_;
}

std::string Auth::server()
{
	//FIXME: avoid hardcode config values
	BOOSTER_LOG(debug,__FUNCTION__) << app_.settings().get<std::string>(std::string("opncms.") + type_ + ".params.server");
	return app_.settings().get<std::string>(std::string("opncms.") + type_ + ".params.server");
}

const std::string& Auth::type()
{
	return type_;
}

const std::string& Auth::method()
{
	return method_;
}

bool AuthData::exists(const std::string& email)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	return AuthData::exists_user(email);
}

bool AuthData::active(const std::string& email)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::get_user(email,v,"status"))
		return false;

	return (v.get_value<int>() == 1);
}

bool AuthData::active(const std::string& email, bool flag)
{
	BOOSTER_LOG(debug,__FUNCTION__);

	cppcms::json::value v = (flag)?1:0;

	//v.at("status", (flag)?1:0);
	return AuthData::set_user(email,v,"status");
}

int AuthData::attempts(const std::string& email)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);
	if (email.empty())
		return -1;

	if(!AuthData::active(email))
		return -1;

	if(!AuthData::get_user(email,v,"attempts"))
		return -1;

	int cnt = v.get_value<int>();
	if(cnt >= 0 && cnt <= MAX_LOGIN_ATTEMPTS)
		return cnt;

	return -1;
}

int AuthData::attempts(const std::string& email, int count)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	if (email.empty())
		return -1;

	if(count < 0 || count > MAX_LOGIN_ATTEMPTS)
		return -1;

	if(!AuthData::active(email))
		return -1;

	cppcms::json::value v = count;
	//v["attempts"] = count;
	//ioc::get<Data>().driver().set("users", email, v);
	if( !AuthData::set_user(email,v,"attempts") )
		return -1;

	return count;
}

std::string AuthData::create(const std::string& email, const std::string& password, const std::string& name)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	if (email.empty())
		return "";
	
	//create only if storage is not exists
	//TODO: we should get the storage names from user plugin config
	ioc::get<Data>().driver().create("users"); 
	ioc::get<Data>().driver().create("hashes");

	if(AuthData::exists_user(email))
		return "";

	BOOSTER_LOG(debug,__FUNCTION__) << "user is not exists";
	cppcms::json::value v,w;
	std::string method = ioc::get<Data>().settings().get<std::string>("opncms.auth.method","bcrypt");
	BOOSTER_LOG(debug,__FUNCTION__) << "method(" << method << ")";
	std::string gen_password;

	if(method == "bcrypt")
	{
		std::string salt = ioc::get<Auth>().bcrypt_salt(ioc::get<Data>().settings().get<int>("opncms.auth.rounds",CRYPT_ROUNDS));
		BOOSTER_LOG(debug,__FUNCTION__) << "salt(" << salt << ")";
		gen_password = ioc::get<Auth>().bcrypt_hash(password,salt);
	}
	else
		return "";
	//DEBUG
	/*
	else
		v["password"] = password;
	*/
	v["password"] = gen_password;
	v["name"] = name;
	v["status"] = 0;
	v["attempts"] = 0;
	w["email"] = email;

	std::string hash = std::string("0") + ioc::get<Auth>().sha512(email+name+gen_password+ioc::get<Auth>().seed(SEED_DEFAULT));
	BOOSTER_LOG(debug,__FUNCTION__) << "hash(" << hash << ")"; //WARN: should be hidden in logs
	if(!AuthData::set_user(email,v) || !(ioc::get<Data>().driver().set("hashes",hash,w)))
		return "";
	BOOSTER_LOG(debug,__FUNCTION__) << "user data saved to storage";
	return hash;
}

std::string AuthData::reset(const std::string& email)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	cppcms::json::value v,w;
	//Check the existance
	if(!AuthData::get_user(email, v))
		return "";

	BOOSTER_LOG(debug,__FUNCTION__) << "user is exists and has data";

	std::string name = v.get("name","undefined"); //it is better then empty for hashing
	std::string hash = ioc::get<Auth>().sha512(email+name+ioc::get<Auth>().seed(SEED_DEFAULT));
	//bit more secure and prevent collision with verify
	hash = std::string("1") + hash;
	BOOSTER_LOG(debug,__FUNCTION__) << "hash(" << hash << ")";
	w["email"] = email;
	
	if(!AuthData::set_user(email,v) || !ioc::get<Data>().driver().set("hashes",hash,w))
		return "";

	return hash;
}

//reset password
bool AuthData::password(const std::string& email, const std::string& newpassword)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";

	if( AuthData::exists_user(email) )
	{
		cppcms::json::value v;
		std::string gen_password;
		
		std::string method = ioc::get<Data>().settings().get<std::string>("opncms.auth.method","bcrypt");
		BOOSTER_LOG(debug,__FUNCTION__) << "method(" << method << ")";

		if(method == "bcrypt")
		{
			
			std::string salt = ioc::get<Auth>().bcrypt_salt(ioc::get<Data>().settings().get<int>("opncms.auth.rounds",CRYPT_ROUNDS));
			gen_password = ioc::get<Auth>().bcrypt_hash(newpassword,salt);
		}
		else
			return false;
		/*
		else
			v["password"] = newpassword;
		*/
		v["password"] = gen_password;
		v["attempts"] = 0;
		v["status"] = 1;
		std::string hash = ioc::get<Auth>().sha512(email+gen_password+ioc::get<Auth>().seed(SEED_DEFAULT));
		v["token"] = std::string("3") + hash;
		return AuthData::set_user(email,v);
	}
	return false;
}

//change password
bool AuthData::password(const std::string& email, const std::string& oldpassword, const std::string& newpassword)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";

	if( AuthData::check(email, oldpassword) )
	{
		cppcms::json::value v;
		
		std::string method = ioc::get<Data>().settings().get<std::string>("opncms.auth.method","bcrypt");
		BOOSTER_LOG(debug,__FUNCTION__) << "method(" << method << ")";

		std::string gen_password;
		if(method == "bcrypt")
		{
			std::string salt = ioc::get<Auth>().bcrypt_salt(ioc::get<Data>().settings().get<int>("opncms.auth.rounds",CRYPT_ROUNDS));
			gen_password = ioc::get<Auth>().bcrypt_hash(newpassword,salt);
		}
		else
			return false;
		/*
		else
			v["password"] = newpassword;
		*/
		v["password"] = gen_password;
		v["attempts"] = 0;
		std::string hash = ioc::get<Auth>().sha512(email+gen_password+ioc::get<Auth>().seed(SEED_DEFAULT));
		v["token"] = std::string("3") + hash;

		//change the password is not the way to unblock the account
		return AuthData::set_user(email,v);
	}
	return false;
}

std::string AuthData::hash(const std::string& email)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";
	if (email.empty())
		return "";
	
	std::string h = ioc::get<Data>().driver().get("hashes");

	if(h.empty())
		return "";

	BOOSTER_LOG(debug,__FUNCTION__) << "hash(" << h << ")";
	cppcms::json::value v = tools::string_to_json(h);
	for(cppcms::json::object::const_iterator it = v.object().begin(); it != v.object().end(); it++)
	{
		if(it->second["email"] == email)
			return it->first;
	}
	return "";
}

std::string AuthData::token(const std::string& uniq)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "uniq(" << uniq << ")";
	if (uniq.empty())
		return "";
	
	return ioc::get<Auth>().sha512(uniq+ioc::get<Auth>().seed(SEED_DEFAULT));
}

std::string AuthData::token_email(const std::string& token)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "token(" << token << ")";
	if (token.empty())
		return "";
	
	std::string h = ioc::get<Data>().driver().get("hashes", token);
	cppcms::json::value v = tools::string_to_json(h);

	std::string email = v.get<std::string>("email","");

	if(!AuthData::get_user(email,v))
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "user is not found for email(" << email << ")";	
		return "";
	}
	int status = v.get<int>("status",0);
	BOOSTER_LOG(debug,__FUNCTION__) << "found user with email(" << email << ") and status(" << status << ")";	
	return email;
}

std::string AuthData::verify(const std::string& hash)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "hash(" << hash << ")";
	
	std::string h = ioc::get<Data>().driver().get("hashes",std::string(hash));
	cppcms::json::value v = tools::string_to_json(h);

	std::string email = v.get<std::string>("email","");

	if(!AuthData::get_user(email,v))
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "user is not found for email(" << email << ")";	
		return "";
	}
	int status = v.get<int>("status",0);
	BOOSTER_LOG(debug,__FUNCTION__) << "found user with email(" << email << ") and status(" << status << ")";

	switch(status)
	{
		case 0:
			AuthData::active(email,true);
		case 1:
			//we should erase hash to block its usage in user app
			return email;
	}
	return "";
}

bool AuthData::authorize(const std::string& email, const std::string& password)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::get_user(email,v))
		return false;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "Find user with email";
	
	if(v.get<int>("status",0) != 1)
		return false;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "User is active";
	
	std::string stored_password = v.get<std::string>("password","");
	if(stored_password == "")
		return false;

	BOOSTER_LOG(debug,__FUNCTION__) << "Process with password";

	std::string method = ioc::get<Data>().settings().get<std::string>("opncms.auth.method","bcrypt");
	BOOSTER_LOG(debug,__FUNCTION__) << "method(" << method << ")";
	if(method == "bcrypt")
	{
		//BOOSTER_LOG(debug,__FUNCTION__) << "stored[" << stored_password.substr(0,4) << "], hash[" << ioc::get<Auth>().bcrypt_hash(password,stored_password) << "] = stored[" << stored_password << "]";
		if( (stored_password.substr(0,4) == "$2a$") && (ioc::get<Auth>().bcrypt_hash(password,stored_password) == stored_password) )
			return true;
		else
			return false;
	}
	/*
	//DEBUG
	else
		if(stored_password == password)
			return true;
	*/
	return false;
}

bool AuthData::check(const std::string& email, const std::string& password)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";

	if (!AuthData::active(email))
		return false;
	
	BOOSTER_LOG(debug,__FUNCTION__) << "user is active, process authorize";	
	if (AuthData::exists(email)) {
		if (AuthData::authorize(email, password)) {
			BOOSTER_LOG(debug,__FUNCTION__) << "Successfully sign in";
			//session updates on a higher level (Auth)
			AuthData::attempts(email,0);
			return true;
		}
	}
	BOOSTER_LOG(error,__FUNCTION__) << "Can't sign in";

	int count = AuthData::attempts(email);
	if(count<0)
		return false;

	count++;
	if (count >= MAX_LOGIN_ATTEMPTS) {
		BOOSTER_LOG(error,__FUNCTION__) << "User should be locked out due to excessive invalid logins";
		AuthData::block(email, true);
	}
	else
		AuthData::attempts(email,count);

	return false;
}

bool AuthData::name(const std::string& email, const std::string& password, const std::string& name)
{
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::exists_user(email))
		return false;

	if( AuthData::check(email, password) )
	{
		cppcms::json::value v;
		v["name"] = name;
		return AuthData::set_user(email,v);
	}
	return false;
}

std::string AuthData::name(const std::string& email)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::get_user(email,v,"name"))
		return "";

	//return v.get("name", "");
	return v.get_value<std::string>();
}

bool AuthData::block(const std::string& email)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::get_user(email,v))
		return false;

	return (v.get("status", 0) == 2);
}

bool AuthData::block(const std::string& email, bool flag)
{
	cppcms::json::value v;
	BOOSTER_LOG(debug,__FUNCTION__);

	if(!AuthData::get_user(email,v))
		return false;

	v["status"] = (flag)?2:1; //inactive user can't be active by attempts not incremented
	return AuthData::set_user(email, v);
}

bool AuthData::get_user(const std::string& email, cppcms::json::value& v, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << "), key(" << key << ")";

	if ( email.empty() )
		return false;

	std::string u = key.empty() ? user_data(email) : user_data(email, key);
	BOOSTER_LOG(debug,__FUNCTION__) << "value(" << u << ")";

	if( u.empty() )
		return false; //user is not exists

	v = tools::string_to_json(u);

	if(v.is_undefined() || v.is_null())
		return false;

	return true;
}

bool AuthData::set_user(const std::string& email, cppcms::json::value& v, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";
	return key.empty() ? ioc::get<Data>().driver().set("users",tools::replace(email,".","#"),v) : ioc::get<Data>().driver().set("users",tools::replace(email,".","#")+"."+key,v);
}

bool AuthData::exists_user(const std::string& email)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";

	if (email.empty() || email == "")
		return false;

	return ioc::get<Data>().driver().exists("users",tools::replace(email,".","#"));
}

std::string AuthData::user_data(const std::string& email, const std::string& key)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	return (key.empty())? ioc::get<Data>().driver().get("users",tools::replace(email,".","#")) : ioc::get<Data>().driver().get("users",tools::replace(email,".","#")+"."+key);
}
// -------------------------------- Auth --------------------------------
std::string Auth::id()
{
	if ( !app_.session().is_set("id") )
		return "";
	return app_.session().get("id");
}

void Auth::id(std::string const& u)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "id(" << u << ")";
	if(!u.empty())
	{
		app_.session().set("id", u);
		app_.session().expose("id", true);
	}
	else
		app_.session().hide("id");
}

bool Auth::auth()
{
	/*
	UNAUTHED = 1,
	UNNAMED = 2,
	LOCAL = 4,
	AUTHED = 8
	*/
	int utype = Auth::user_type();
	BOOSTER_LOG(debug,__FUNCTION__) << "user_type=" << utype;

	if(utype == AUTHED) // || (utype == LOCAL && cppcms.auth.local) ) //TODO: settings for passing LOCAL users
		return true;
	
	if(utype == UNAUTHED || !utype)
	{
		Auth::do_auth();
		return (Auth::user_type() == AUTHED);
	}
	return false;
}

//check if user exists in session & set id
void Auth::do_auth()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	std::string uid = Auth::id();
	int utype = Auth::user_type();

	//check if user saved to session and set authed
	if (!uid.empty() && ioc::get<Auth>().ref().exists(uid))
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "id(" << uid << ") authed";
	}
	else if ( utype == UNAUTHED || !utype )
	{
		std::string ip = tools::get_ip(app_.request());//TEST: = request_.remote_addr();
		if (tools::is_local(ip_list_, ip)) {
			//TODO: if IP is local - get id from the config
			Auth::id("local");
			Auth::user_type(LOCAL);
		}
		else {
			Auth::id(ip);
			Auth::user_type(UNNAMED);
		}
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "id(" << uid << "), user_type(" << utype << ")";
}


void Auth::deauth()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	id(std::string());
	app_.session().erase("id");
	app_.session().expose("id");
	remind(false);
	user_type(0);
	//?
	app_.session().clear();
	app_.session().reset_session();
}

std::string Auth::token(const std::string& uniq)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "uniq(" << uniq << ")";
	std::string t = ioc::get<Auth>().ref().token(uniq);
	app_.session().set( "t", t );
	app_.session().expose("t");
	return t;
}

bool Auth::check(const std::string& email, const std::string& password, const std::string& token)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "token(" << token << ")";

	std::string cur_email = ioc::get<Auth>().ref().token_email(token);

	if(cur_email.empty())
	{
		BOOSTER_LOG(error,__FUNCTION__) << "Can't get email for token";
		return false;
	}
	
	if(cur_email != email)
	{
		BOOSTER_LOG(error,__FUNCTION__) << "E-mails are not equal";
		return false;
	}
	
	bool chk = Auth::check(cur_email,password);

	if (chk)
		Auth::token(email); //make new token and store it in storage and cookie

	return chk;
}


bool Auth::check(const std::string& email, const std::string& password)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "email(" << email << ")";
	
	bool chk = ioc::get<Auth>().ref().check(email,password);

	if (chk) {
		Auth::id(email);
		Auth::user_type(AUTHED);
		Auth::remind(false);
	}
	else {
		Auth::remind(true);
	}
	return chk;
}

bool Auth::remind()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	return ( app_.session().is_set("remind") && !app_.session().get("remind").empty() );
}

void Auth::remind(bool val)
{
	BOOSTER_LOG(debug,__FUNCTION__) << std::string(val?"true":"false");
	app_.session().set("remind", val?"1":"");
	app_.session().expose("remind", true);
	//remind_ = val;
}

bool Auth::local()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	//auth();
	//return local_;
	return false;
	//return ( app_.session().is_set("user_type") && (app_.session().get("user_type") == fmt::format("{}", LOCAL)) );
}

void Auth::user_type(int utype)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "user_type=" << utype;
	
	if(utype)
		app_.session().set( "user_type", fmt::format("{}",utype) );
	else
		ioc::get<Data>().session().erase("user_type");

	app_.session().expose( "user_type",true );
}

int Auth::user_type()
{
	BOOSTER_LOG(debug,__FUNCTION__);

	if ( app_.session().is_set("user_type") )
		return atoi( app_.session().get("user_type").c_str() );
	BOOSTER_LOG(debug,__FUNCTION__) << "user is not authenticated";
	return UNAUTHED;
}

std::string Auth::bcrypt_hash(const std::string& password, const std::string& salt)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	return ::bcrypt_hash(password, salt);
}
std::string Auth::bcrypt_salt(int rounds) {
	BOOSTER_LOG(debug,__FUNCTION__) << "rounds=" << rounds;
	return ::bcrypt_salt(rounds);
}

std::string Auth::bcrypt_salt() {
	BOOSTER_LOG(debug,__FUNCTION__);
	return ::bcrypt_salt();
}

std::string Auth::seed(size_t size)
{
	return tools::get_random(size);
}

std::string Auth::sha512(std::string const& key)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	return tools::string_hash(key, "sha512");
}
