////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_TOOLS_H
#define OPNCMS_TOOLS_H

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

#include <sys/stat.h> //for stat struct
#include <sys/file.h> //for open files
#include <dirent.h> //for list of files
#include <errno.h> //for errno
#include <time.h> //for get_random
#ifdef OPNCMS_WIN_NATIVE
#include <Windows.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include <cppcms/json.h>
#include <cppcms/http_request.h>
#include <booster/log.h>
#include <booster/nowide/fstream.h>
#include <cppcms/crypto.h>
#include <booster/regex.h>

#include <netdb.h> //freeaddrinfo
#include <ifaddrs.h> //getifaddrs
#include <arpa/inet.h> //inet_ntop

#include <unistd.h> //chdir & get_current_dir_name & gethostname
#include <fmt/format.h>
#include <boost/filesystem/operations.hpp> //for file functions
#include <opncms/esmtp.h>

#define MAX_BUFFER 1024
#define MAX_HASH_SIZE 512
#define IPV6_ENABLE 0

#ifdef OPNCMS_WIN_NATIVE
#define SLH "\\"
#else
#define SLH "/"
#endif

#define PATH_DELIMITER "."

///
/// \cond inline
///

namespace tools {

typedef std::pair<std::string, std::string> pair_str;

typedef std::vector <std::pair <std::string, std::string> > vec_map;
typedef std::vector <std::string> vec_str;
typedef std::vector <vec_str> vec_vec;

typedef std::map<std::string, std::string> map_str;
typedef std::map<std::string, pair_str> map_pair;
typedef std::map<std::string, vec_str> map_vec;

static const std::string empty_string = std::string();

class parsing_error : public std::runtime_error {
public:
	parsing_error(std::string s) : std::runtime_error(s) {}
};

/* STRING functions */
///
/// \brief Trim whitespaces at begin of string
///
std::string& trim_left(std::string& /*s*/);
///
/// \brief Trim whitespaces at end of string
///
std::string& trim_right(std::string& /*s*/);
///
/// \brief Trim whitespaces at both siedes of string
///
std::string& trim(std::string& /*s*/);
///
/// \brief Convert string to hexademical string
///
void str2hex(const std::string& /*s*/, std::string& /*d*/);
///
/// \brief Convert hex string to string
bool hex2str(const std::string& /*s*/, std::string& /*d*/);

template <typename T>
T str2num ( const std::string &txt )//Text not by const reference so that the function can be used with a
{                               //character array as argument
	std::istringstream iss(txt) ;
	T res = 0;

	if( iss >> res )
	{
		if( res < std::numeric_limits<T>::min() || res > std::numeric_limits<T>::max() )
			throw std::out_of_range( "str2num: out of range" ) ;
	}
	else 
		throw std::invalid_argument( "str2num: invalid argument" ) ;
	return res;
}

std::string replace(const std::string& /*str*/, const std::string& /*from*/, const std::string& /*to*/);
std::string replace_any(const std::string& /*str*/, const std::string& /*from*/, const std::string& /*to*/);

//TODO: should it done with replace or split from boost?
///
/// \brief Performs splitting of string s, separeted with delim into elems
/// \param s string for splitting
/// \param delim string delimiter
/// \param elems vector of resulting strings
///
void split(const std::string& /*s*/, const std::string& /*delim*/, std::vector <std::string>& /*elems*/);
///
/// \brief Performs splitting of string s, returns the first part before delimiter
/// \param s string for splitting
/// \param delim string delimiter
///
std::string split_first(const std::string& /*s*/, const std::string& /*delim*/);
///
/// \brief Performs splitting of string s, returns the last part before delimiter
/// \param s string for splitting
/// \param delim string delimiter
///
std::string split_last(const std::string& /*s*/, const std::string& /*delim*/);
///
/// \brief Performs splitting of string s, returns the second part after delimiter
/// \param s string for splitting
/// \param delim string delimiter
///
std::string split_second(const std::string& /*s*/, const std::string& /*delim*/);

/*
void split(const std::string &s, char delim, std::vector<std::string>& elems);
*/
///
/// \brief Performs splitting of string s, separeted with delim into returning vector of strings
/// \param s string for splitting
/// \param delim string delimiter
///
std::vector<std::string> split(const std::string& /*s*/, const std::string& /*delim*/);

///
/// \brief Performs joining of vector using separator
/// \param v std::vector for joining
/// \param delim string delimiter
///
template <typename T>
std::string join(std::vector<T> v, const std::string& delim)
{
	std::stringstream ss;
	typedef typename std::vector<T>::iterator joinit;

	for(joinit it = v.begin(); it != v.end(); ++it)
	{
		if(it+1 != v.end())
			ss << *it << delim;
		else
			ss << *it;
	}
	return ss.str();
}

///
/// \brief Get both previous path and key
/// 
void path_prev(const std::string& /*path*/, const std::string& /*delim*/, std::string& /*prev_path*/, std::string& /*prev_key*/);

///
/// \brief Get previous path
/// 
std::string path_prev(const std::string& /*path*/);

///
/// \brief Get previous key
/// 
std::string key_prev(const std::string& /*path*/);

///
/// \brief Convert string to cppcms::json::value
/// 
cppcms::json::value string_to_json(const std::string& /*s*/);

///
/// \brief Convert cppcms::json::value to string
/// 
std::string json_to_string(cppcms::json::value const& /*v*/);

///
/// \brief Convert cppcms::json::array to string
/// 
std::string json_array_str(const cppcms::json::array& /*arr*/);

///
/// \brief Convert cppcms::json::value to string
/// 
std::string json_value_str(const cppcms::json::value& /*v*/);

///
/// \brief Convert cppcms::json::value to std::map (with full key)
/// 
void json_to_map(std::map<std::string, std::string>& /*resmap*/, const cppcms::json::value& /*v*/, const std::string& path="");

///
/// \brief Convert std:map (with full key) to cppcms::json::value
/// 
void map_to_json(const std::map<std::string, std::string>& /*resmap*/, cppcms::json::value& /*v*/, const std::string& /*path*/);

///
/// \brief Convert std:map (with full key) to cppcms::json::value
/// 
void map_to_json(const std::map<std::string, std::string>& /*resmap*/, cppcms::json::value& /*v*/);

///
/// \brief Add/replace cppcms::json::value at key
/// 
void json_set(cppcms::json::value& /*v*/, const std::string& /*key*/, cppcms::json::value const& /*value*/);

///
/// \brief Add/replace cppcms::json::value at key
/// 
void json_erase(const std::string& /*key*/, cppcms::json::value& /*v*/);

/* CONFIG functions */
namespace data 
{
	bool load(const std::string& /*file*/, const std::string& /*tmpl*/, cppcms::json::value &/*v*/);
	bool load(const std::string& /*file*/, cppcms::json::value &/*v*/);
	bool save(const std::string& /*file*/, const std::string& /*tmpl*/, cppcms::json::value &/*v*/, bool backup=true);
	bool save(const std::string& /*file*/, cppcms::json::value &/*v*/, bool backup=true);

	template <typename T>
	T get(const std::string& file, const std::string& tmpl, const std::string& path)
	{
		cppcms::json::value v;
		if(load(file,tmpl,v))
			return v.get<T>(path,T());
	}

	template <typename T>
	T get(const std::string& file,const std::string& path)
	{
		cppcms::json::value v;
		if(load(file,v))
			return v.get<T>(path,T());
		return T();
	}

	template <typename T>
	bool set(const std::string& file,const std::string& tmpl,const std::string& path,T const& value)
	{
		cppcms::json::value v;
		if(load(file,v)) {
			v.set<T>(path, value);
			return save(file,tmpl,v);
		}
		return false;
	}

	template <typename T>
	bool set(const std::string& file,const std::string& path,T const& value)
	{
		cppcms::json::value v;
		if(load(file,v)) {
			v.set<T>(path, value);
			return save(file,v);
		}
		return false;
	}

}

/* FILE functions */
time_t get_mtime(const std::string& /*filename*/);

/* HASH functions */
std::string to_str(unsigned char */*ptr*/, size_t /*n*/);
std::string string_hash(const std::string& /*s*/, const std::string& /*htype*/);
std::string stream_hash(std::istream &/*is*/, const std::string& /*htype*/);
std::string file_hash(const std::string& /*file*/, const std::string& /*htype*/);
std::string magnet(const std::string& /*str*/, const std::string& /*tag*/);
void init_random();
unsigned long get_random();
std::string get_random(size_t /*size*/);

/*
inline unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}
*/

/* IP functions */
class if_ip_list
{
public:
	if_ip_list();
	void init();
	bool find(const std::string& s);
	bool empty();
	~if_ip_list();
private:
	std::string get_ip(struct in_addr address);
	std::string get_ip(struct in6_addr address);

	struct ifaddrs *ifaddrs_;
	std::vector<std::string> ip_;
};

std::string get_hostname();
bool getfullbyname(vec_str &/*hlist*/);
std::string get_ip(cppcms::http::request &/*req*/);
bool is_ip(const std::string& ip);
bool is_local(if_ip_list& /*ip_list*/, const std::string& /*ip*/);
bool is_local(if_ip_list& /*ip_list*/, cppcms::http::request &/*req*/);
std::string get_directmail(const std::string& /*mail*/);
bool send_email(const std::string& /*user*/, const std::string& /*password*/, const std::string& /*mail*/, const std::string& /*subj*/, const std::string& /*msg*/, bool /*direct*/);

/* EXEC functions */
std::string vexec(const std::string& /*cmd*/, unsigned int /*timeout*/);

}

///
/// \endcond
///

#endif
