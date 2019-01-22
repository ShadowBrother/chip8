#include "Logger.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

Logger *Logr = new Logger("D:\\My Documents\\GitHub\\chip8\\Debug\\g_chip8Log");

Logger::Logger(std::string filename) {
	Logger::filename = filename;
	Logger::logg = new std::ostringstream();
}

std::ostringstream &Logger::Log() {
	
	return *Logger::logg;
}

void Logger::writeLog() {
	if (Log()) {
		if (Log().good()) {
			Log() << std::endl; //add endline to logg
								//open output file stream
			std::ofstream logFile;
			logFile.open(filename, std::ios::out);

			logFile << Log().str();//output logg string to file
			std::cerr << logFile.fail();

			logFile.close();
		}
	}
}