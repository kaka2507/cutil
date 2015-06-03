// -*- C++ -*-
/* 
 * File:   DBInterface.cpp
 * Author: baodn
 *
 * Created on June 26, 2014, 4:29 AM
 */
#include "DBInterface.h"

namespace connector {

	DBInterface::~DBInterface() {
		if(conn_) {
			mysql_close(conn_);
			conn_ = NULL;
		}
	}

	void DBInterface::Init(string ip, int port, string database, string user, string password) {
		ip_ = ip;
		port_ = port;
		database_ = database;
		user_ = user;
		password_ = password;
		Connect();
	}

	void DBInterface::Connect() {
		DBG_LOG(DEBUG,"Connect DB host: %s port: %d user: %s pass: %s database: %s", ip_.c_str(), port_, user_.c_str(), password_.c_str(), database_.c_str());
		if(conn_) {
			mysql_close(conn_);
			conn_ = NULL;
		}
		conn_ = mysql_init(NULL);
		my_bool bReconnect = true;
		mysql_options(conn_, MYSQL_OPT_RECONNECT, &bReconnect);	
		mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8");
		if(mysql_real_connect(conn_, ip_.c_str(), user_.c_str(), password_.c_str(), database_.c_str(), port_, NULL, CLIENT_IGNORE_SIGPIPE) == NULL) {
			DBG_LOG(ERROR,"Can not make connection to DB host: %s port: %d user: %s pass: %s database: %s", ip_.c_str(), port_, user_.c_str(), password_.c_str(), database_.c_str());
			return;
		}
		if (mysql_set_character_set(conn_, "utf8")) {
		    DBG_LOG(ERROR,"Can not set character set for connection to DB");
		}
	}
}

