////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/tools.h>

///
/// \cond internal
///

namespace tools {

/* unusual direct smtp servers */
#define TOOLS_DIRECT_NUM 3
const std::string direct_smtp[TOOLS_DIRECT_NUM][2] = {
	{"gmail.com", "smtp.gmail.com:587"},
	{"yahoo.com", "smtp.mail.yahoo.com:25"},
	{"hotmail.com", "smtp.live.com:25"}
	};

/* STRING functions */
// trim from start
std::string& trim_left(std::string& s) {
	s.erase(s.begin(), std::find_if (s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
std::string& trim_right(std::string& s) {
	s.erase(std::find_if (s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
std::string& trim(std::string& s) {
	return trim_left(trim_right(s));
}

// convert string to hex string
void str2hex(const std::string& s, std::string& d)
{
	register unsigned char x,y;
	char *p = (char*)s.data();
	d.resize(s.size()*2);
	char *p2 = (char*)d.data();
	while(*p!=0) {
		x = *p++;
		y = (x & 0x0F) + 0x30;
		if (y > 0x39) y += 39;
		x = (x >> 4) + 0x30;
		if (x > 0x39) x += 39;
		*p2++ = (char)x;
		*p2++ = (char)y;
	}
}

// convert hex string to string
bool hex2str(const std::string& s, std::string& d) {
	register unsigned char x,y;
	char *p = (char*)s.data();
	if (s.size()%2) return false;
	d.resize(s.size() >> 1);
	char *p2 = (char*)d.data();
	while(*p!=0) {
		x = *p++;
		y = *p++;
		if (x > 0x39) x -= 39;
		if (y > 0x39) y -= 39;
		x = ((x & 0x0F) << 4) + (y & 0x0F);
		*p2++ = (char)x;
	}
	return true;
}

std::string replace(const std::string& str, const std::string& from, const std::string& to)
{
	std::string res;
	res.reserve( str.length() );  // avoids a few memory allocations

	std::string::size_type last_pos = 0;
	std::string::size_type find_pos;

	while( std::string::npos != ( find_pos = str.find( from, last_pos )))
	{
		res.append( str, last_pos, find_pos - last_pos );
		res += to;
		last_pos = find_pos + from.length();
	}

	// Care for the rest after last occurrence
	res += str.substr( last_pos );
	return res;
}

std::string replace_any(const std::string& str, const std::string& from, const std::string& to)
{
	//TODO: will use replace or split from boost
	std::string res;
	size_t pos, pos2;

	pos2 = str.find_first_not_of(from);
	pos = str.find_first_of(from,pos2);

	if (pos == std::string::npos)
		return str.substr(pos2,pos-pos2);

	while(pos != std::string::npos)
	{
		res = res+str.substr(pos2,pos-pos2);
		pos2 = str.find_first_not_of(from, pos);

		if (pos2 != std::string::npos)
			res = res + to;

		pos = str.find_first_of(from,pos2);
	}
	if (pos2 != std::string::npos) //last part
		res = res+str.substr(pos2);
	return res;
}

//TODO: should it done with replace or split from boost?
void split(const std::string& s, const std::string& delim, std::vector <std::string>& elems)
{
	size_t pos = 0;
	size_t pos2 = 0;

	while(pos != std::string::npos) {
		pos2 = s.find_first_of(delim,pos);
		elems.push_back(s.substr(pos,pos2-pos));
		pos = s.find_first_not_of(delim,pos2);
	}
	if (pos2 != std::string::npos)
		elems.push_back("");
}

std::string split_first(const std::string& s, const std::string& delim)
{
	size_t pos = s.find_first_of(delim,0);
	if(pos == std::string::npos)
		return "";
	return s.substr(0,pos);
}

std::string split_last(const std::string& s, const std::string& delim)
{
	size_t pos = s.find_last_of(delim);
	if(pos == std::string::npos)
		return "";
	return s.substr(pos);
}

std::string split_second(const std::string& s, const std::string& delim)
{
	size_t pos = s.find_first_of(delim,0);
	if(pos == std::string::npos)
		return "";
	pos = s.find_first_not_of(delim,pos);
	return s.substr(pos);
}

std::vector<std::string> split(const std::string& s, const std::string& delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void path_prev(const std::string& path, const std::string& delim, std::string& prev_path, std::string& prev_key)
{
	std::string s(path);
	size_t found = s.find_last_of(delim);
	BOOSTER_LOG(debug,__FUNCTION__) << "found[" << found << "]";
	if (found == std::string::npos)
	{
		prev_path = "";
		prev_key = path;
	} else {
		prev_path = s.substr(0,found);
		prev_key = s.substr(found+1);
	}
}

std::string path_prev(const std::string& path)
{
	std::string res, tmp;
	path_prev(path, PATH_DELIMITER, res, tmp);
	return res;
}

std::string key_prev(const std::string& path)
{
	std::string res, tmp;
	path_prev(path, PATH_DELIMITER, tmp, res);
	return res;
}

cppcms::json::value string_to_json(const std::string& s)
{
	cppcms::json::value v;
	int line = 0;
	if(s.empty() || s == "")
		return v;

	std::istringstream ss(s);
	if(!v.load(ss,true,&line)) {
		std::ostringstream tmp;
		tmp << "Parsing error of " << s << " in line " << line;
		throw parsing_error(tmp.str());
	}
	return v;
}

std::string json_to_string(cppcms::json::value const& v)
{
	std::ostringstream ss;
	if(!v.is_undefined())
		ss << v;
	return ss.str();
}

std::string json_array_str(const cppcms::json::array& arr)
{
	std::string s = "[";
	cppcms::json::array::const_iterator it;
	for( it = arr.begin(); it != arr.end(); ++it )
		s += json_value_str(*it) + ",";
	return (s.substr(0, s.size()-1) + "]");
}

std::string json_value_str(const cppcms::json::value& v)
{
	if(v.type() == cppcms::json::is_array)
		return json_array_str(v.array());
	else if(v.type() == cppcms::json::is_string)
		return v.str();
	else if(v.type() == cppcms::json::is_number)
		return std::string( fmt::format("{}",v.number()) );
	else if(v.type() == cppcms::json::is_boolean)
		return std::string( (v.boolean()) ? "true" : "false" );
	else if(v.type() == cppcms::json::is_null)
		return "null";
	return "";
}

void json_to_map(std::map<std::string, std::string>& resmap, const cppcms::json::value& v, const std::string& path)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "type(" << v.type() << ")";
	if(v.type() == cppcms::json::is_object)
	{
		for(cppcms::json::object::const_iterator it = v.object().begin(); it != v.object().end(); ++it)
		{
			BOOSTER_LOG(debug,__FUNCTION__) << "value(" << it->first.str() << ":" << tools::json_to_string(it->second) << ")";
			if(it->second.type()==cppcms::json::is_object)
				json_to_map(resmap, it->second.object(), path + it->first.str() + ".");
			else
				resmap[path + it->first.str()] = json_value_str(it->second);
		}
	} else {
		std::string s = json_value_str(v);
		if(!s.empty())
			resmap[path] = s;
	}
}

void map_to_json(const std::map<std::string, std::string>& resmap, cppcms::json::value& v)
{
	std::map<std::string, std::string>::const_iterator it = resmap.begin();
	for(; it != resmap.end(); ++it)
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "value(" << it->first << ":" << it->second << ")";
		tools::json_set( v, it->first, tools::string_to_json(it->second) );
	}
}


// Add the unique values only, replace regarding the syntax
// To replace the value at key it should be deleted before that

// TODO: should be optimized, considering the logic in test_tools
void json_set(cppcms::json::value& v, const std::string& key, cppcms::json::value const& value)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "value[" << key << ":" << tools::json_to_string(value) << "]";
	const cppcms::json::value& vv = v.find(key);
	if(!vv.is_undefined())
	{
		switch(value.type())
		{
			// Adding Array
			case cppcms::json::is_array: {
				if(vv.type() == cppcms::json::is_array) {
					// Array -> Array (add)
                                        cppcms::json::array& t = v.at(key).array();
					t.push_back(value);
				}
				else {
				
					// Array -> Array (replace)
					v.at(key,value);
				}
				break;
			}
			// Adding Object
			case cppcms::json::is_object: {
				// Object -> Object
				if(vv.type() == cppcms::json::is_object)
				{
					cppcms::json::object& t = v.at(key).object();
					//std::cerr << "Value at key is object[" << tools::json_to_string(v.at(key)) << "]";
					BOOSTER_LOG(debug,__FUNCTION__) << "Value at key is object[" << tools::json_to_string(v.at(key)) << "]";
					//add only unique keys
					for(cppcms::json::object::const_iterator it=value.object().begin();it!=value.object().end();it++)
					{
						if( t.find(it->first) == t.end() )
						{
							//std::cerr << "Add value[" << it->first << ":" << it->second << "]";
							BOOSTER_LOG(debug,__FUNCTION__) << "Add value[" << it->first << ":" << it->second << "]";
							t.insert(std::make_pair(it->first,it->second));
						} else {
							if( t[it->first] != it->second )
							{
								//std::cerr << "Replace value[" << it->first << ":" << it->second << "]";
								BOOSTER_LOG(debug,__FUNCTION__) << "Replace value[" << it->first << ":" << it->second << "]";
								t[it->first] = it->second;
							}
						}
					}
					break;
				}
				// Object -> Array
				if(vv.type() == cppcms::json::is_array)
				{
					cppcms::json::array& t = v.at(key).array();
					//std::cerr << "Value at key is array[" << tools::json_to_string(v.at(key)) << "]";
					BOOSTER_LOG(debug,__FUNCTION__) << "Value at key is array[" << tools::json_to_string(v.at(key)) << "]";
					cppcms::json::value tmp;
					
					for(cppcms::json::object::const_iterator it=value.object().begin();it!=value.object().end();it++)
					{
						//cppcms::json::array::iterator itt = std::find(t.begin(), t.end(), tmp);
						int replaced = 0;
						for(unsigned int i=0; i<t.size();i++)
						{
							
							if(t.at(i).type() == cppcms::json::is_object)
							{
								//std::cerr << "object[" << json_to_string(t.at(i)) << "]";
								cppcms::json::object::const_iterator oit = t.at(i).object().find(it->first);
								if(oit != t.at(i).object().end() && oit->second != it->second)
								{
									//std::cerr << "Replace value[" << oit->first << ":" << oit->second << "]";
									BOOSTER_LOG(debug,__FUNCTION__) << "Replace value[" << it->first << ":" << it->second << "]";
									t.at(i).object()[it->first] = it->second;
									replaced = 2;
									break;
								} else {
									replaced = 1;
									break;
								}
							}
						}
						//std::cerr << "replaced[" << replaced << "]";
						if (replaced == 0)
							tmp[it->first] = it->second;
					}
					if(!tmp.is_undefined())
						t.push_back(tmp);
					//std::cerr << "Result value[" << json_to_string(t) << "]";
					BOOSTER_LOG(debug,__FUNCTION__) << "Result value[" << tools::json_to_string(t) << "]";
				} else {
					// Value -> Array
					std::cerr << "Value at key[" << tools::json_to_string(v.at(key)) << "]";
					BOOSTER_LOG(debug,__FUNCTION__) << "Value at key[" << tools::json_to_string(v.at(key)) << "]";
					cppcms::json::array& t = v.at(key).array();
					cppcms::json::array::const_iterator it = std::find(t.begin(), t.end(), value);
					if(it == t.end())
						t.push_back(*it);
				}
				break;
			}
			default:{
				if(vv.type() == cppcms::json::is_array)
				{
					cppcms::json::array& t = v.at(key).array();
					//std::cerr << "Value[" << tools::json_to_string(v.at(key)) << "]";
					BOOSTER_LOG(debug,__FUNCTION__) << "Value[" << tools::json_to_string(v.at(key)) << "]";
					if( std::find(t.begin(), t.end(), value) == t.end() )
					{
						//std::cerr << "Add value[" << value << "]";
						BOOSTER_LOG(debug,__FUNCTION__) << "Add value[" << value << "]";
						t.push_back(value);
					}
				} else {
					v.at(key,value);
				}
			}
		}
	}
	else
		v.at(key,value); //can't load value or it's undefined

	BOOSTER_LOG(debug,__FUNCTION__) << "new value[" << tools::json_to_string(v.at(key)) << "]";
}

void json_erase(const std::string& key, cppcms::json::value& v)
{
	std::string p1 = (key.empty()) ? "" : tools::path_prev(key);
	std::string p2 = (key.empty()) ? "" : tools::key_prev(key);
	BOOSTER_LOG(debug,__FUNCTION__) << "key[" << key << "]=>[" << p1 << "][" << p2 << "]";
	
	if(!v.find(key).is_null() && !v.find(key).is_undefined())
	{
		if(!p1.empty())
		{
			BOOSTER_LOG(debug,__FUNCTION__) << "value before: [" << tools::json_to_string(v.at(p1)) << "]";
			cppcms::json::object::iterator it = v.at(p1).object().find(p2);
			v.at(p1).object().erase(it);
			BOOSTER_LOG(debug,__FUNCTION__) << "value after: [" << tools::json_to_string(v.at(p1)) << "]";
		} else {
			BOOSTER_LOG(debug,__FUNCTION__) << "value before: [" << tools::json_to_string(v) << "]";
			cppcms::json::object::iterator it = (p2.empty()) ? v.object().begin() : v.object().find(p2);
			v.object().erase(it);
			BOOSTER_LOG(debug,__FUNCTION__) << "value after: [" << tools::json_to_string(v) << "]";
		}
	}
	else
		BOOSTER_LOG(debug,__FUNCTION__) << "Can't find value at key[" << key << "]";
}

namespace data {

// load cppcms::json::value with data of file at path or tmpl (able to use repository)

bool load(const std::string& file, const std::string& tmpl, cppcms::json::value& v)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "file: " << file << ", template: " << tmpl;

	booster::nowide::ifstream fin(file.c_str(), booster::nowide::ifstream::in);

	if (!fin)
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "Can`t open file: " << file << ", try use template file: " << tmpl;

		if (!tmpl.empty())
			fin.open(tmpl.c_str(), booster::nowide::ifstream::in);

		if (!fin) {
			BOOSTER_LOG(debug,__FUNCTION__) << "Can`t open template file " << tmpl << ", exit";
			fin.close();
			return false;
		}
	}
	//at this point - fin is path || fin
	int lineno = 0;

	boost::filesystem::path p(file);
	if(boost::filesystem::file_size(p)) {
		if (!v.load(fin, true, &lineno)) {
			BOOSTER_LOG(error,__FUNCTION__) << "Error in reading data at line: " << lineno;
			fin.close();
			return false;
		}
		if (v.is_undefined()) {
			BOOSTER_LOG(debug,__FUNCTION__) << "Data in datafile is undefined";
			fin.close();
			return false; //this is not error actually, just signal for calling func
		}
	}
	else
		BOOSTER_LOG(debug,__FUNCTION__) << "File is empty";

	fin.close();
	return true;
}

bool load(const std::string& file, cppcms::json::value& v)
{
	return tools::data::load(file,"",v);
}

// save cppcms::json::value with data to file at path or tmpl (able to use scm repository)
bool save(const std::string& file, const std::string& tmpl, cppcms::json::value& v, bool backup)
{
	boost::filesystem::path file_path(file);
	
	if( !boost::filesystem::exists(file_path) )
	{
		BOOSTER_LOG(debug,__FUNCTION__) << "file: " << file << " doesn't exists, create new one";

		//check directory for new user (one recursion)
		//if directory is not exists - create new one
		if(file_path.is_absolute())
		{
			boost::filesystem::recursive_directory_iterator pit(file_path), pend;
			pit++;

			if( !boost::filesystem::exists(pit->path()) )
				boost::filesystem::create_directory(pit->path());
		}

		if( !tmpl.empty() )
			boost::filesystem::copy_file(boost::filesystem::path(tmpl), file_path);
	} else {
		BOOSTER_LOG(debug,__FUNCTION__) << "file exists, backup " << ((backup)?"on":"off");

		if(backup)
			boost::filesystem::rename(file_path,boost::filesystem::path(file+"~") );
	}

	booster::nowide::ofstream fout(file.c_str(), booster::nowide::ofstream::out);

	if(!v.is_undefined()) {
		try {
			v.save(fout,true); //WARN: just override whole file
		}
		catch (...) { //WARN
			throw tools::parsing_error("Error in saving value");
			return false;
		}
	} else {
		BOOSTER_LOG(debug,__FUNCTION__) << "value is undefined - save empty string";
	}
	fout.close();

	return true;
}

// save cppcms::json::value with data to file at path or tmpl (able to use repository)
bool save(const std::string& file, cppcms::json::value& v, bool backup)
{
	return tools::data::save(file,"",v, backup);
}

//for ffind see so/q/612097/list files
int list(const std::string& dir, std::vector<std::string>& files)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "directory: " << dir;
	int cnt = 0;
#ifdef OPNP_WIN_NATIVE
	WIN32_FIND_DATA fd;
	HANDLE hf;
	bool cnt_hidden = false;
	DWORD attr = FILE_ATTRIBUTE_DIRECTORY;
	if (!cnt_hidden)
		attr |= FILE_ATTRIBUTE_HIDDEN;

	hf = FindFirstFile(dir, &fd);
	if (hf == INVALID_HANDLE_VALUE) {
		BOOSTER_LOG(error,__FUNCTION__) << "Error(" << errno << ") opening " << dir << ": " << GetLastError();
		return errno;
	}

	do {
		if (!(fd.dwFileAttributes& attr))
			files.push_back(fd.cFileName);
	}
	while( FindNextFile(hf,&fd) );

	//BOOSTER_LOG(debug,__FUNCTION__) << "file=" << fd.cFileName;
	FindClose(hf);
#else
	DIR *dp; //http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		BOOSTER_LOG(error,__FUNCTION__) << "Error(" << errno << ") opening dir " << dir;
		return 0;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		try
		{
			if ( !( strncmp(dirp->d_name,".",1) && strncmp(dirp->d_name,"..",2) ) )
				continue;

			files.push_back(std::string(dirp->d_name));
			cnt++;
		}
		catch (std::exception& e)
		{
			closedir(dp);
			BOOSTER_LOG(error,__FUNCTION__) << e.what();
		}
	}
	closedir(dp);
#endif
	return cnt;
}

} //config

/* string & hash function */

std::string to_str(unsigned char *ptr,size_t n)
{
	std::string res;
	for(unsigned i=0;i<n;i++) {
		char buf[3];
		sprintf(buf,"%02x",ptr[i]);
		res+=buf;
	}
	return res;
}

time_t get_mtime(const std::string& filename)
{
	//stat/_wstat checks the filename is empty (ENOENT)
#ifdef OPNCMS_WIN_NATIVE
	struct _stat st;
	if( _wstat(booster::nowide::convert(filename).c_str(), &st) < 0 )
		return 0;
#else
	struct stat st;
	if( stat(filename.c_str(), &st ) < 0 )
		return 0;
#endif
	return st.st_mtime;
}

std::string string_hash(const std::string& s, const std::string& htype)
{
	std::auto_ptr<cppcms::crypto::message_digest> d(cppcms::crypto::message_digest::create_by_name(htype));
	unsigned char buf[MAX_HASH_SIZE];
	d->append(s.c_str(),s.size());
	unsigned n=d->digest_size();
	d->readout(buf);
	return to_str(buf,n);
}

//FIXME: aware use writable streams
//Using: is - pointer to istream,
//htype - type of hash ("sha1", "md5", etc..),
std::string stream_hash(std::istream& is, const std::string& htype)
{
	std::auto_ptr<cppcms::crypto::message_digest> d(cppcms::crypto::message_digest::create_by_name(htype));
	
	std::string s( (std::istreambuf_iterator<char>( is )), (std::istreambuf_iterator<char>()) );
	return string_hash(s,htype);
}

/*
//TODO: get really randomized value
std::string get_seed(const std::string& htype)
{
	return "";
}
*/
//FIXME: read whole all file, not partial
std::string file_hash(const std::string& file, const std::string& htype)
{
	boost::filesystem::path file_path(file);
	
	if( !boost::filesystem::exists(file_path) )
		return tools::empty_string;

	booster::nowide::ifstream ifs;
	ifs.open(file.c_str(), std::ios::binary);

	if (!ifs) {
		BOOSTER_LOG(debug,__FUNCTION__) << "Can`t open file: " << file;
		return tools::empty_string;
	}
	return stream_hash(ifs, htype);
}

//Using: str - string with data for magnet
std::string magnet(const std::string& str, const std::string& tags)
{
	std::string m = "magnet:?xt=";
	m = m + "urn:sha1:" + string_hash(str,"sha1"); //sha1 hash

	if (!tags.empty())
		m = m + "&kt=" + tools::replace(tags,", 	", "+"); //using \x09 simbol for tab
	//m += "&xs=http://"; //N2R url
	//m += "&xs=dchub://"; //dc++
	//m += "&kt=t1+t2+mp3"; //tags
	//m += "&as=http://"; //source file (url encoded)
	return m;
}

void init_random()
{
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "clock_gettime failed";
		srandom(time(NULL));
	}
	else
		srandom(ts.tv_nsec ^ ts.tv_sec);
}

//TODO: using rtdsc?
unsigned long get_random()
{
#ifdef OPNP_WIN_NATIVE

	HCRYPTPROV hCryptProv;
	unsigned long rand_buf;
	if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "CryptAcquireContext failed";
		return 0;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "CryptAcquireContext succeeded";
 
	if (!CryptGenRandom(hCryptProv, sizeof(rand_buf), (BYTE *)&rand_buf))
	{
		BOOSTER_LOG(error,__FUNCTION__) << "CryptGenRandom failed";
		return 0;
	}
	
	return rand_buf;

#else

	init_random();
	return random();

#endif
}

//TODO: using rtdsc?
std::string get_random(size_t size)
{
	//TODO: it's better to randomly shift the sequence
	char a[37] = "9qwe2rty4uio6plk0jhg7fds3azx8cvb1nm5";
//	unsigned long h[36] = {0};
	BOOSTER_LOG(debug,__FUNCTION__);
	std::string rnd;
	size_t n = size;
	unsigned long b = 0;
	unsigned long c = 0;

#ifndef OPNP_WIN_NATIVE
	init_random();
#endif

	while(n)
	{
#ifdef OPNP_WIN_NATIVE
		c = get_random() % 36;
#else
		c = random() % 36;
#endif
		if(b != c)
		{
			rnd += a[c];
//			h[c]++;
			b = c;
			n--;
		}
	}
	/*
	std::cerr << "histogram: ";
	for(int i=0; i<36; i++)
		std::cerr << h[i] << " ";
	std::cerr << std::endl;
	*/
	return rnd;
}

/* IP functions */

std::string get_hostname()
{
	BOOSTER_LOG(debug,__FUNCTION__);
	char hostname[MAX_BUFFER] = {0};

	if (gethostname(hostname, MAX_BUFFER-1) < 0) {
		BOOSTER_LOG(error,__FUNCTION__) << "can`t get hostname";
		return "";
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "hostname=" << hostname;
	return hostname;
}

bool getfullbyname(tools::vec_str& hlist)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	struct addrinfo hints, *info, *p;
	int ai_res;

	std::string hostname = get_hostname();

	if(hostname.empty())
		return false;

	memset(&hints, 0, sizeof(hints));
#ifdef IPV6_ENABLE
	hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
#else
	hints.ai_family = AF_INET; /*IPV4*/
#endif
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME; /*AI_PASSIVE?*/

	if ((ai_res = getaddrinfo(hostname.c_str(), "http", &hints, &info)) != 0) {
		BOOSTER_LOG(error,__FUNCTION__) << "getaddrinfo: " << gai_strerror(ai_res);
		freeaddrinfo(info);
		return false;
	}

	try
	{
		for(p = info; p != NULL; p = p->ai_next)
			hlist.push_back(p->ai_canonname);
	}
	catch (std::exception& e)
	{
		freeaddrinfo(info);
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
	}	
	freeaddrinfo(info);
	return true;
}

//FIXME: working just for IPv4 cases
bool is_ip(const std::string& ip)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
	if(!result)
		result = inet_pton(AF_INET6, ip.c_str(), &(sa.sin_addr));
	BOOSTER_LOG(debug,__FUNCTION__) << "ip " << ip << " " << std::string(result?"validated":"not validated");
	return result != 0;
}

std::string get_ip(cppcms::http::request &req)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "HTTP_CLIENT_IP=" << req.getenv("HTTP_CLIENT_IP") << ", HTTP_X_FORWARDED_FOR=" << req.getenv("HTTP_X_FORWARDED_FOR") << ", remote_addr=" << req.remote_addr();
	//ip is pass from internet
	if (!req.get("HTTP_CLIENT_IP").empty())
		return req.getenv("HTTP_CLIENT_IP");

	//ip is pass from proxy
	if (!req.get("HTTP_X_FORWARDED_FOR").empty())
		return req.getenv("HTTP_X_FORWARDED_FOR");

	return req.remote_addr();
}

if_ip_list::if_ip_list()
: ifaddrs_(NULL)
{
	init();
}
	
void if_ip_list::init()
{
	if (getifaddrs(&ifaddrs_) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	if(ip_.size())
		ip_.clear();
	
	struct ifaddrs *ifa=NULL;
	std::string ip;
	for (ifa = ifaddrs_; ifa != NULL; ifa = ifa->ifa_next)
	{
		if(ifa->ifa_addr == NULL)
			continue;
		
		switch(ifa->ifa_addr->sa_family)
		{
			case AF_INET:
				ip = get_ip( ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr );
				break;
			case AF_INET6:
				ip = get_ip( ((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr );
				break;
			case AF_PACKET:
				break;
			default:
				BOOSTER_LOG(debug,__FUNCTION__) << "unknown sa_family(" << ifa->ifa_addr->sa_family << ")";
		}
		if(!ip.empty())
			ip_.push_back(ip);
	}
}

bool if_ip_list::find(const std::string& s)
{
	if(s.empty() || !ip_.size())
		return false;
	return (std::find(ip_.begin(),ip_.end(),s) != ip_.end());
}

bool if_ip_list::empty()
{
	return ip_.empty();
}

if_ip_list::~if_ip_list()
{
	if (ifaddrs_ != NULL)
		freeifaddrs(ifaddrs_);
}

std::string if_ip_list::get_ip(struct in_addr address)
{
	char str[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &address, str, INET_ADDRSTRLEN) == NULL)
	{
		BOOSTER_LOG(error,__FUNCTION__) << strerror(errno);
		return "";
	}
	return std::string(str);
}

std::string if_ip_list::get_ip(struct in6_addr address)
{
	char str[INET6_ADDRSTRLEN];
	if (inet_ntop(AF_INET6, &address, str, INET6_ADDRSTRLEN) == NULL)
	{
		BOOSTER_LOG(error,__FUNCTION__) << strerror(errno);
		return "";
	}
	return std::string(str);
}

//Checks if ip is local
//TODO: IP should be cached
bool is_local(if_ip_list& ip_list, const std::string& ip)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "ip(" << ip << ")";

	//std::string ip = get_ip(req);
	if ( ip.empty() || !is_ip(ip) || ip_list.empty() )
		return false;

	return ip_list.find(ip);
}

// Checks if request is local by extracted ip
bool is_local(if_ip_list& ip_list, cppcms::http::request &req)
{
	std::string ip = tools::get_ip(req);//TEST: =  request_.remote_addr();

	if (tools::is_local(ip_list, ip))
		//if IP is local - just get username from config
		//this->username_ = username();
		return true;

	return false;
}

std::string get_directmail(const std::string& mail)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "mail(" << mail <<")";
	if(mail.empty())
		return "";

	std::vector<std::string> v;
	tools::split(mail,"@",v);

	if(!v.size() || v[1].empty())
		return "";

	for(int i = 0; i < TOOLS_DIRECT_NUM; i++)
	{
		if(tools::direct_smtp[i][0] == v[1])
			return tools::direct_smtp[i][1];
	}
	return std::string("smtp.")+v[1]+":25";
}

bool send_email(const std::string& user, const std::string& password, const std::string& mail, const std::string& subj, const std::string& msg, bool direct)
{
	BOOSTER_LOG(debug,__FUNCTION__) << "user(" << user << "), mail(" << mail << "), subj(" << subj << "), msg(" << msg << ")";

	std::string host = get_hostname();
	std::string mailhost = "localhost:25";

	if(host.empty()) {
		host = "localhost";
	}
	
	if(direct) {
		std::string m = get_directmail(mail);
		if(!m.empty())
			mailhost = m;
	}
	BOOSTER_LOG(debug,__FUNCTION__) << "host(" << host << "), mailhost(" << mailhost << ")";
	if(!ESMTP::open(mailhost, user, password)) {
		ESMTP::close();
		return false;
	}
	
	try
	{
		std::vector<std::string> r;
		r.push_back(mail);

		if(!ESMTP::send(std::string("opncms@")+host, mail, r, subj, msg)) {
			ESMTP::close();
			return false;
		}
	}
	catch(std::exception& e)
	{
		BOOSTER_LOG(error,__FUNCTION__) << e.what();
	}
	ESMTP::close();
	return true;
}

// ---------------------- EXEC functions -------------------------
std::string vexec(const std::string& cmd, unsigned int timeout)
{
	char buf[MAX_BUFFER];
	std::string out;

	if (!timeout)
		timeout = 10;

	FILE* pipe = popen(cmd.c_str(), "r" );
	if (pipe == NULL ) {
		BOOSTER_LOG(debug,__FUNCTION__) << "invoking " << cmd << " is failed";
		return "";
	}
	//waitfor(timeout);
	while(fgets(buf, MAX_BUFFER, pipe) != NULL ) {
		out.append(buf);
	}
	pclose(pipe);
	return out;
}

} // namespace tools

///
/// \endcond
///
