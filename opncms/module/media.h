////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_M_MEDIA_H
#define OPNCMS_M_MEDIA_H

#include <cppcms/application.h>
#include <cppcms/http_file.h>

#include <opncms/module/data.h>
#include <opncms/module/auth.h>

///
/// \brief Media Manager - class to operate on media objects (files, etc.).
/// Unlike Data - it don't use the content data
/// (that not viewable from user's browser directly),
/// its main goal - actions & presentable content
/// depends on Data (for storing configuration data)
///
class Media
{

public:
	///
	/// \brief Constructor
	///
        Media(cppcms::application& /*app*/);
	///
	/// \brief Destructor
	///
        ~Media();

	///
	/// \brief Returns root path for media files
	///
	const std::string& root();
	///
	/// \brief Performs saving of avatar file with BASE64 data
	/// \param name name of avatar file
	/// \param photo BASE64 of avatar file
	///
	bool file_save(const std::string& /*name*/, const std::string& /*data*/);
	///
	/// \brief Performs saving of avatar file with streams
	/// \param name name of avatar file
	/// \param photo pointer to cppcms::http::file structure
	///
	void file_save(const std::string& /*name*/, booster::shared_ptr<cppcms::http::file> /*data*/);
private:
	cppcms::application& app_;
	std::string path_;
	std::string upload_;
};

#endif
