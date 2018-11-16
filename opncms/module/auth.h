////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_AUTH_H
#define OPNCMS_M_AUTH_H

#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <cppcms/application.h>
#include <cppcms/crypto.h>

#include <booster/log.h> //for current log

#include <ifaddrs.h> //getifaddrs
#include <arpa/inet.h>

#include <opncms/module/data.h>
#include <opncms/bcrypt.h>
#include <fmt/format.h>

#define MAX_LOGIN_ATTEMPTS 5
#define SEED_DEFAULT 1024

class Auth;

enum usertype {
	UNAUTHED = 1,
	UNNAMED = 2,
	LOCAL = 4,
	AUTHED = 8
};

///
/// \class Interface for various implementations of Auth manager
///

class IAuth {
public:
	///
	/// \brief Create the new user
	/// \param email user email
	/// \param password user password
	/// \param name user name
	///
	virtual std::string create(const std::string& /*email*/, const std::string& /*password*/, const std::string& /*name*/) = 0;
	///
	/// \brief Reset the password
	/// \param email user email
	///
	virtual std::string reset(const std::string& /*email*/) = 0;
	///
	/// \brief Reset the password with new one
	/// \param email user email
	/// \param newpassword new user password
	///
	virtual bool password(const std::string& /*email*/, const std::string& /*newpassword*/) = 0;
	///
	/// \brief Change the old password to new one
	/// \param email user email
	/// \param oldpassword old user password
	/// \param newpassword new user password
	///
	virtual bool password(const std::string& /*email*/, const std::string& /*oldpassword*/, const std::string& /*newpassword*/) = 0;
	///
	/// \brief Change the old name to new one
	/// \param email user email
	/// \param password user password
	/// \param name new user name
	///
	virtual bool name(const std::string& /*email*/, const std::string& /*password*/, const std::string& /*login*/) = 0;
	///
	/// \brief Get the name of user
	/// \param email user email
	///
	virtual std::string name(const std::string& /*email*/) = 0;
	///
	/// \brief Check password of user with stored data
	/// \param email user email
	/// \param password user password
	///
	virtual bool authorize(const std::string& /*email*/,const std::string& /*password*/) = 0;
	///
	/// \brief Check if email of user exists
	/// \param email user email
	///
	virtual bool exists(const std::string& /*email*/) = 0;
	///
	/// \brief Returns hash for new user
	/// \param email user email
	///
	virtual std::string hash(const std::string& /*email*/) = 0;
	///
	/// \brief Returns token for the unique string
	/// \param uniq unique string (i.e. email)
	///
	virtual std::string token(const std::string& /*uniq*/) = 0;
	///
	/// \brief Returns email for the token
	/// \param token unique token
	///
	virtual std::string token_email(const std::string& /*token*/) = 0;
	///
	/// \brief Check if email for new user is verified by hash, returns email
	/// \param hash user hash
	///
	virtual std::string verify(const std::string& /*hash*/) = 0;
	///
	/// \brief Check email and password of user with current implementation
	/// \param email user email
	/// \param password user password
	///
	virtual bool check(const std::string& /*email*/, const std::string& /*password*/) = 0;
	///
	/// \brief Check if email is blocked
	/// \param email user email
	///
	virtual bool block(const std::string& /*email*/) = 0;
	///
	/// \brief Block/unblock the user
	/// \param email user email
	/// \param flag true to block user
	///
	virtual bool block(const std::string& /*email*/, bool /*flag*/) = 0;
	///
	/// \brief Check if user is activated
	/// \param email user email
	///
	virtual bool active(const std::string& /*email*/) = 0;
	///
	/// \brief Set activity of user
	/// \param email user email
	/// \param flag true to activate user
	///
	virtual bool active(const std::string& /*email*/, bool /*flag*/) = 0;
	///
	/// \brief Get count of failed login attempts
	/// \param email user email
	///
	virtual int attempts(const std::string& /*email*/) = 0;
	///
	/// \brief Get count of failed login attempts
	/// \param email user email
	///
	virtual int attempts(const std::string& /*email*/, int /*count*/) = 0;
};

class AuthData : public IAuth {
public:
	AuthData();
	std::string create(const std::string& /*email*/, const std::string& /*password*/, const std::string& /*login*/);
	std::string reset(const std::string& /*email*/);
	bool exists(const std::string& /*email*/);
	std::string hash(const std::string& /*email*/);
	std::string token(const std::string& /*uniq*/);
	std::string token_email(const std::string& /*token*/);
	std::string verify(const std::string& /*hash*/);
	bool check(const std::string& /*email*/, const std::string& /*password*/);
	bool password(const std::string& /*email*/, const std::string& /*newpassword*/);
	bool password(const std::string& /*email*/, const std::string& /*oldpassword*/, const std::string& /*newpassword*/);
	bool name(const std::string& /*email*/, const std::string& /*password*/, const std::string& /*email*/);
	std::string name(const std::string& /*email*/);
	bool block(const std::string& /*email*/);
	bool block(const std::string& /*email*/, bool /*block*/);
	bool active(const std::string& /*email*/);
	bool active(const std::string& /*email*/, bool /*active*/);
	int attempts(const std::string& /*email*/);
	int attempts(const std::string& /*email*/, int /*count*/);
private:
	bool authorize(const std::string& /*email*/, const std::string& /*password*/);
	bool get_user(const std::string& /*email*/, cppcms::json::value& /*v*/, const std::string& key = tools::empty_string);
	bool set_user(const std::string& /*email*/, cppcms::json::value& /*v*/, const std::string& key = tools::empty_string);
	bool exists_user(const std::string& /*email*/);
	std::string user_data(const std::string& /*email*/, const std::string& key = tools::empty_string);
};

///
/// \class Auth auth.h opncms/module/auth.h
/// \brief Authentication Manager
/// Sample usage:
///	Auth a;
///

class Auth
{
public:
	///
	/// \enum usertype
	/// Represent type of user authentication
	/// \var usertype UNAUTHED
	/// User not authenticated yet
	/// \var usertype UNNAMED
	/// User not authenticated yet, but we have it's IP
	/// \var usertype LOCAL
	/// User authenicate locally on the server
	/// \var usertype AUTHED
	/// User authenticated
	///
	enum usertype {
		UNAUTHED = 1,
		UNNAMED = 2,
		LOCAL = 4,
		AUTHED = 8
	};

	///
	/// \brief Constructor of AuthBase
	/// \param app - instance of main application,
	///
	Auth(cppcms::application& app);
	~Auth();

	///
	/// \brief Returns the authorization id for this session
	///
	std::string id();
	///
	/// \brief Sets the authorization id for this session
	/// \param id
	///
	void id(std::string const& /*id*/);
	///
	/// \brief Returns type of user (LOCAL | AUTHED | UNAUTHED)
	///
	int user_type();
	///
	/// \brief Sets current user_type
	/// \param type of user (LOCAL | AUTHED | UNAUTHED)
	///
	void user_type(int utype);
	///
	/// \brief Returns current authentication server 
	/// (value of opncms.[opncms.auth].params.server from application settings)
	///
	std::string server();
	///
	/// \brief Returns type of authentication (data,etc.)
	/// (value of opncms.auth from application settings)
	///
	const std::string& type();
	///
	/// \brief Returns method (algorithm) of authentication (bcrypt,scrypt,etc..)
	/// (value of opncms.auth.method from application settings)
	///
	const std::string& method();
	///
	/// \brief Returns driver for current type of authentication
	/// (value of opncms.[auth].driver from application settings)
	///
	const std::string& driver();
	///
	/// \brief Returns reference to instance of current type of authentication
	///
	IAuth& ref();
	///
	/// \brief Returns the token (hash) generated on unique string
	/// \param uniq unique string
	///
	std::string token(const std::string& /*uniq*/);
	///
	/// \brief Check user for correct login/password with selected authentication module 
	/// \param email user email
	/// \param password user password
	///
	bool check(const std::string& /*email*/, const std::string& /*password*/);
	///
	/// \brief Check user for correct token/password with selected authentication module
	/// \param email user email
	/// \param token user token
	/// \param password user password
	///
	bool check(const std::string& /*email*/, const std::string& /*password*/, const std::string& /*token*/);
	///
	/// \brief Returns authentication status for current user 
	/// or perform authentication if current user not authenticated yet
	///
	bool auth(); //use for returning authed_ as well
	///
	/// \brief Deauthenticates current user
	///
	void deauth();
	///
	/// \brief Returns location of current user
	/// - true, if user request server locally,
	/// - false, if user request server from remote station
	///
	bool local();
	///
	/// \brief Returns state of remid flag for login mistakes
	///
	bool remind();
	///
	/// \brief Set state of remind flag
	/// \param val=true, if user need to be reminded and false - otherwise
	///
	void remind(bool val);
	///
	/// \brief Performs generating of seed for auth
	///
	/*
	std::string seed();
	*/
	///
	/// \brief Generate BCrypt salt with custom count of rounds
	/// \param rounds count of rounds (between 4 and 31)
	///
	virtual std::string bcrypt_salt(int /*rounds*/);
	///
	/// \brief Generate BCrypt salt with default(12) count of rounds
	/// \param login user login
	/// \param password user password
	///
	virtual std::string bcrypt_salt();
	///
	/// \brief Generate BCrypt hash by salt
	/// \param password password
	/// \param salt hash or salt
	///
	std::string bcrypt_hash(const std::string& /*password*/, const std::string& /*salt*/);
	///
	/// \brief Performs generating of SHA512 hash
	/// \param key unique string
	///
	std::string sha512(std::string const& /*key*/);
	///
	/// \brief Performs generating pseudo-random string
	/// \param size size of string
	///
	std::string seed(size_t /*size*/);
private:
	std::string type_; //opncms.auth.storage
	std::string driver_; //opncms._type_.driver
	std::string method_; //opncms.auth.method

	cppcms::application& app_;
	AuthData data_;
	tools::if_ip_list ip_list_; //to check ip of local interfaces

	///
	/// \brief Performs authentication
	///
	void do_auth();

	///
	/// \brief Performs getting of user's type
	///
	int do_user_type();	

	///
	/// \brief Performs getting of user's data by email and key
	///
};

///
/// \cond inline
///


///
/// \endcond
///
#endif
