#include "Log.h"
#include <chrono>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <mutex>

#include "ucp3.h"


Log::ELevel Log::s_setting = Log::ELevel::Info;

void Log::DoWrite(ELevel level, ESource source, char const* fmt, ...)
{
	static constexpr size_t s_bufferSize = 1024;
	static char             s_szBuffer[s_bufferSize];
	static std::stringstream    s_stream;
	static std::mutex       s_mutex;

	std::lock_guard<std::mutex> const lock(s_mutex);

	// sprintf input

	va_list args;
	va_start(args, fmt);
	int const count = vsprintf_s(s_szBuffer, s_bufferSize, fmt, args);
	va_end(args);

	if (count <= 0)
	{
		return;
	}

	// write level
	ucp_NamedVerbosity logLevel = ucp_NamedVerbosity::Verbosity_0;

	char const* szLevel;
	switch (level)
	{
		case ELevel::Error:
			szLevel = "E ";
			logLevel = ucp_NamedVerbosity::Verbosity_ERROR;
			break;
		case ELevel::Warning:
			szLevel = "W ";
			logLevel = ucp_NamedVerbosity::Verbosity_WARNING;
			break;
// #ifndef NDEBUG
		case ELevel::Debug:
			szLevel = "D ";
			logLevel = ucp_NamedVerbosity::Verbosity_1;
			break;
// #endif
		case ELevel::Info:
		default:
			szLevel = "I ";
			logLevel = ucp_NamedVerbosity::Verbosity_0;
			break;
	}
	s_stream << szLevel;

	// write time

	time_t in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	s_stream << std::put_time(std::localtime(&in_time_t), "%X ");
	
	// write source

	char const* szSource;
	switch (source)
	{
	case ESource::Client:
		szSource = "Client";
		break;
	case ESource::Server:
		szSource = "Server";
		break;
	case ESource::System:
	default:
		szSource = "System";
		break;
	}

	s_stream << szSource << ": ";

	// write input

	s_stream << s_szBuffer;

	// ucp_log is thread safe
	ucp_log(logLevel, s_stream.str().c_str());

	s_stream.str(std::string());
}


