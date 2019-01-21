#include "Logger.h"


std::ostringstream &Logger::Log() {
	if (!Logger::logg) {
		Logger::logg = new std::ostringstream();

	}
	return *logg;
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