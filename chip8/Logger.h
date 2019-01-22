//Logger.h header file for chip8/RCA logging
//Jesse Hoyt - jesselhoyt@gmail.com

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

class Logger {


	private:
		std::string filename;
		std::ostringstream *logg;
	public:
		Logger(std::string filename);
		
		std::ostringstream &Log();
		void writeLog();
};

extern Logger *Logr;

#endif
