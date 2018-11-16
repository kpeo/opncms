////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#include <opncms/base64.h>
#include <opncms/module/media.h>

Media::Media(cppcms::application &app):
app_(app)
{
	BOOSTER_LOG(debug,__FUNCTION__);
	path_ = app_.settings().get<std::string>("opncms.media");
	upload_ = app_.settings().get<std::string>("opncms.file.root");

	if(upload_.empty())
		upload_ = path_ + SLH + "upload";
}

Media::~Media()
{
	BOOSTER_LOG(debug,__FUNCTION__);
}

const std::string& Media::root()
{
	return path_;
}

bool Media::file_save(const std::string& name, const std::string& data)
{
	BOOSTER_LOG(error,__FUNCTION__);
	bool local = ioc::get<Auth>().local();
	std::string username = ioc::get<Auth>().id();
	std::string tmpl = app_.settings().get<std::string>("opncms.templates.users");
	std::string path;

	//1. store data
	if (local)
		path = path_+SLH+"user"+SLH+"image";
	else
		path = path_+SLH+"users"+SLH+username+SLH+"image";

	std::ofstream ofs((path+SLH+name).c_str(), std::ios::binary);
	std::string base64_data = base64_decode(data);
	ofs.write(base64_data.data(),base64_data.size());

	ofs.close();
	if(!ofs)
	{
		BOOSTER_LOG(error,__FUNCTION__) << "error has occured while saving";
		return false;
	}
	//2. store config
	//ioc::get<Data>().ref().file_save(username, tmpl, local);
	return true;
}

void Media::file_save(const std::string& name, booster::shared_ptr<cppcms::http::file> data)
{
	BOOSTER_LOG(error,__FUNCTION__);
	bool local = ioc::get<Auth>().local();
	std::string username = ioc::get<Auth>().id();
	std::string tmpl = app_.settings().get<std::string>("opnproject.templates.users");
	std::string path;

	//1. store data
	if (local)
		path = path_+SLH+"user"+SLH+"image";
	else
		path = path_+SLH+"users"+SLH+username+SLH+"image";

	data->save_to(path+SLH+name);
	//2. store config
	//ioc::get<Data>().ref().file_save(username, tmpl, local);
	//TODO: 3. store to Net
}
