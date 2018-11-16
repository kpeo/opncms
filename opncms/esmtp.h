////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-2018 Vladimir Yakunin (kpeo) <opncms@gmail.com>
//
//  The redistribution terms are provided in the COPYRIGHT.txt file
//  that must be distributed with this source code.
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef OPNCMS_ESMTP_H
#define OPNCMS_ESMTP_H

#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
#include <string.h>

#include <openssl/ssl.h>
#include <auth-client.h>
#include <libesmtp.h>
#include <booster/log.h>

//#define _XOPEN_SOURCE 500
#define arg_unused __attribute__((unused))
#define ESMTP_MAX_STR 512
#define ESMTP_HOST_DEFAULT "localhost:25"

namespace ESMTP
{
	bool open(const std::string& /*host*/, const std::string& /*user*/, const std::string& /*password*/, const std::string& tls_password="");
	void close();
	bool send(const std::string& /*from*/, const std::string& /*to*/, std::vector<std::string>& /*recipients*/, const std::string& /*subject*/, const std::string& /*data*/);

	int authinteract(auth_client_request_t /*request*/, char** /*result*/, int /*fields*/, void* /*arg*/);
	int tlsinteract(char* /*buf*/, int /*buflen*/, int /*rwflag*/, void* /*arg*/);
	void print_recipient_status(smtp_recipient_t /*recipient*/, const char* /*mailbox*/, void* /*arg*/);
	void monitor_cb(const char* /*buf*/, int /*buflen*/, int /*writing*/, void* /*arg*/);
	void event_cb(smtp_session_t /*session*/, int /*event_no*/, void* arg,...);
	int handle_invalid_peer_certificate(long /*result*/);	
}

#endif
