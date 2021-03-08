#include <library/log.hpp>

#include <ctime>
#include <iostream>

using namespace std;

namespace library
{
	Log logger;

	Log::Log()
	{
		// console output enabled by default
		this->outputConsole = true;
	}

	Log::Log(std::string filename) : Log()
	{
		this->open(filename);
	}

	Log::~Log()
	{
		// close file, if open
		this->file.close();
	}

	void Log::open(std::string filename)
	{
		if (file.good()) file.close();
		// output logfile
		this->file.open(filename.c_str(), ios::app | ios::out);
	}

	void Log::setOutputConsole(bool conout)
	{
		this->outputConsole = conout;
	}

	Log& operator<<(Log& out, const Log::LogLevel level)
	{
		if (level) // not ENDL
		{
//			out.synch.lock();

			stringstream ss;

			// add timestamp to log
			time_t rawtime;
			time(&rawtime);
			tm* t = gmtime(&rawtime);

			if (t)
			{
				ss << "[" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << "] ";
			}
			out.log += ss.str();

			// add loglevel: symbol --> text
			switch (level)
			{
			case Log::INFO:
				out.log += "INFO: "; break;
			case Log::WARN:
				out.log += "WARNING: "; break;
			case Log::ERR:
				out.log += "ERROR: "; break;
			case Log::ENDL:
				break;
			}

		}
		else // Log::log_level_t.ENDL
		{
			if (out.outputConsole)
			{
				// output to console, if enabled
				cout << out.log << endl;
			}
			// output to file
			if (out.file.good())
				out.file << out.log << endl;
			// clear log string
			out.log = "";

//			out.synch.unlock();
		}
		// outgoing = incoming Log, for chaining
		return out;
	}

	bool Log::write(LogLevel level, std::string text)
	{
		if (this->file.is_open() == false) return false;

//		synch.lock();
		stringstream ss;

		time_t rawtime;
		time(&rawtime);
		tm* t = gmtime(&rawtime);

		if (t)
		{
			ss << "[" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << "] ";
		}

		// add loglevel: symbol --> text
		switch (level)
		{
		case Log::INFO:
			ss << "INFO"; break;
		case Log::WARN:
			ss << "WARNING"; break;
		case Log::ERR:
			ss << "ERROR"; break;
		case Log::ENDL:
			break;
		}

		ss << ": " << text << endl;
		// to file
		if (file.good())
			file << ss.str();
		// output to console
		if (this->outputConsole)
		{
			cout << ss.str() << endl;
		}

//		synch.unlock();
		return true;
	}

}
