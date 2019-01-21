//Logger.h header file for chip8/RCA logging
//Jesse Hoyt - jesselhoyt@gmail.com

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

std::string filename = "log";

static class Logger {


private:
	static std::ostringstream *logg;
	
public:
	static std::ostringstream &Log();
	static void writeLog();
};

#endif
