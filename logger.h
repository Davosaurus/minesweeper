#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

class Logger {
private:
	const char* logFilePath = "log.txt";
	std::ofstream logFile_;
	std::mutex mutex_;
	
	// Constructor
	Logger() {
		std::lock_guard<std::mutex> lock(mutex_);
		logFile_.open(logFilePath, std::ios::app);
		if(logFile_.is_open()) {
			logFile_ << std::unitbuf;
		}
	}

	// Destructor automatically restores the original buffer
	~Logger() {
		if(logFile_.is_open()) {
			logFile_.close();
		}
	}
	
public:
	// Gets the single instance of the Logger
	static Logger& getInstance() {
		static Logger instance; 
		return instance;
	}
	
	// Overload the << operator
	// We template this so it handles strings, ints, endl, etc.
	template <typename T>
	Logger& operator<<(const T& message) {
		if(logFile_.is_open()) {
			logFile_ << message;
		}
		return *this; // Return reference to allow chaining
	}

	// Special overload for manipulators like std::endl
	Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
		if(logFile_.is_open()) {
			manip(logFile_);
		}
		return *this;
	}
	
	// Removes the log file from the file system
	void clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		logFile_.close();
		std::filesystem::remove(logFilePath);
	}

	// Prevent copying or assignment
	Logger(const Logger&) = delete;
	void operator=(const Logger&) = delete;
};

#define LOGGER Logger::getInstance()