////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_BCRYPT_H
#define OPNCMS_BCRYPT_H

//#define __SKIP_GNU	// against the conflict with unistd.h crypt declaration

#include <ow-crypt.h>
#include <cstdlib>
#include <fstream>
#include <booster/log.h>

#define RANDOM_DEVICE			"/dev/urandom"
#define CRYPT_PREFIX			"$2a$"
#define CRYPT_ROUNDS			15 // 4 < cost < 32

std::string bcrypt_salt(unsigned long count = CRYPT_ROUNDS);
std::string bcrypt_hash(const std::string& key, const std::string& salt);

#endif
