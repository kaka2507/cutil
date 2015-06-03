// -*- C++ -*-
/* 
 * File:   DBInterface.h
 * Author: baodn
 *
 * Created on June 26, 2014, 4:29 AM
 */

#ifndef DB_INTERFACE_H
#define	DB_INTERFACE_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include "mysql.h"
#include "klogger.h"
using namespace std;

namespace connector {
	class DBInterface {
		public:
			DBInterface(){};
			~DBInterface();
			void Init(string ip, int port, string database, string user, string password);
		protected:
			void Connect();
			MYSQL *conn_;
			string ip_;
			int port_;
			string database_;
			string user_;
			string password_;
	};
}

#endif //DB_INTERFACE_H

