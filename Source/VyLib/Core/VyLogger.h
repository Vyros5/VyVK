#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/STL/Pointers.h>
#include <VyLib/STL/String.h>
#include <VyLib/STL/Containers.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#define VY_ASSERT_MESSAGE_BOX (!VY_DIST && VY_PLATFORM_WINDOWS)

#ifdef VY_ASSERT_MESSAGE_BOX
#   include <Windows.h>
#endif

namespace Vy
{
	enum class LogType : U8
	{
		Core   = 0, 
		Client = 1
	};

	enum class LogLevel : U8
	{
		Trace  = 0, 
		Info, 
		Debug,
		Warn, 
		Error, 
		Fatal
	};

	class VyLogger
	{
	public:
		struct TagDetails
		{
			bool     enabled     = true;
			LogLevel levelFilter = LogLevel::Trace;
		};

	public:
		static void init();
		static void shutdown();

		inline static Shared<spdlog::logger>& getCoreLogger()          { return s_CoreLogger; }
		inline static Shared<spdlog::logger>& getClientLogger()        { return s_ClientLogger; }
		inline static Shared<spdlog::logger>& getEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool 
        hasTag(const String& tag) 
        { 
            return s_EnabledTags.contains(tag); 
        }
		
        
        static TMap<String, TagDetails>& 
        enabledTags() 
        { 
            return s_EnabledTags; 
        }

		template<typename FormatStr, typename... Args>
		constexpr static void 
        printMessage(
            LogType     type, 
            LogLevel    level, 
            StringView  tag, 
            FormatStr&& format, 
            Args&&...   args
        );

		template<typename FormatStr, typename... Args>
		static void 
        printAssertMessage(
            LogType     type, 
            StringView  prefix, 
            FormatStr&& format, 
            Args&&...   args
        );

		template<typename... Args>
		static void 
        printAssertMessage(
            LogType    type, 
            StringView prefix
        );

	public:
		// Enum utils
		static CString levelToString(LogLevel level)
		{
			switch (level)
			{
				case LogLevel::Trace: return "Trace";
				case LogLevel::Info:  return "Info";
				case LogLevel::Debug: return "Debug";
				case LogLevel::Warn:  return "Warn";
				case LogLevel::Error: return "Error";
				case LogLevel::Fatal: return "Fatal";
			}
			return "";
		}
		
		static LogLevel levelFromString(StringView string)
		{
			if (string == "Trace") return LogLevel::Trace;
			if (string == "Info" ) return LogLevel::Info;
			if (string == "Debug") return LogLevel::Debug;
			if (string == "Warn" ) return LogLevel::Warn;
			if (string == "Error") return LogLevel::Error;
			if (string == "Fatal") return LogLevel::Fatal;

			return LogLevel::Trace;
		}

	private:
		static Shared<spdlog::logger> s_CoreLogger;
		static Shared<spdlog::logger> s_ClientLogger;
		static Shared<spdlog::logger> s_EditorConsoleLogger;

		inline static TMap<String, TagDetails> s_EnabledTags;
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core logging
#define VY_CORE_TRACE_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Trace, tag, __VA_ARGS__)
#define VY_CORE_INFO_TAG(tag, ...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Info,  tag, __VA_ARGS__)
#define VY_CORE_DEBUG_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Debug, tag, __VA_ARGS__)
#define VY_CORE_WARN_TAG(tag, ...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Warn,  tag, __VA_ARGS__)
#define VY_CORE_ERROR_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Error, tag, __VA_ARGS__)
#define VY_CORE_FATAL_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Fatal, tag, __VA_ARGS__)

// Client logging
#define VY_TRACE_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Trace, tag, __VA_ARGS__)
#define VY_INFO_TAG(tag, ...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Info,  tag, __VA_ARGS__)
#define VY_DEBUG_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Debug, tag, __VA_ARGS__)
#define VY_WARN_TAG(tag, ...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Warn,  tag, __VA_ARGS__)
#define VY_ERROR_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Error, tag, __VA_ARGS__)
#define VY_FATAL_TAG(tag, ...) ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define VY_CORE_TRACE(...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Trace, "", __VA_ARGS__)
#define VY_CORE_INFO(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Info,  "", __VA_ARGS__)
#define VY_CORE_DEBUG(...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Debug, "", __VA_ARGS__)
#define VY_CORE_WARN(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Warn,  "", __VA_ARGS__)
#define VY_CORE_ERROR(...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Error, "", __VA_ARGS__)
#define VY_CORE_FATAL(...)  ::Vy::VyLogger::printMessage(::Vy::LogType::Core, ::Vy::LogLevel::Fatal, "", __VA_ARGS__)

// Client Logging
#define VY_TRACE(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Trace, "", __VA_ARGS__)
#define VY_INFO(...)    ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Info,  "", __VA_ARGS__)
#define VY_DEBUG(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Debug, "", __VA_ARGS__)
#define VY_WARN(...)    ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Warn,  "", __VA_ARGS__)
#define VY_ERROR(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Error, "", __VA_ARGS__)
#define VY_FATAL(...)   ::Vy::VyLogger::printMessage(::Vy::LogType::Client, ::Vy::LogLevel::Fatal, "", __VA_ARGS__)

// Editor Console Logging Macros
#define VY_CONSOLE_LOG_TRACE(...)   ::Vy::VyLogger::getEditorConsoleLogger()->trace(__VA_ARGS__)
#define VY_CONSOLE_LOG_INFO(...)    ::Vy::VyLogger::getEditorConsoleLogger()->info(__VA_ARGS__)
#define VY_CONSOLE_LOG_WARN(...)    ::Vy::VyLogger::getEditorConsoleLogger()->warn(__VA_ARGS__)
#define VY_CONSOLE_LOG_ERROR(...)   ::Vy::VyLogger::getEditorConsoleLogger()->error(__VA_ARGS__)
#define VY_CONSOLE_LOG_FATAL(...)   ::Vy::VyLogger::getEditorConsoleLogger()->critical(__VA_ARGS__)


namespace Vy 
{
	template<typename FormatStr, typename... Args>
	constexpr void 
    VyLogger::printMessage(
        LogType     type, 
        LogLevel    level, 
        StringView  tag, 
        FormatStr&& format, 
        Args&&...   args)
	{
		String formattedMessage;
		if constexpr (sizeof...(args) == 0)
		{
			if constexpr (std::is_convertible_v<FormatStr, CString> || std::is_convertible_v<StringView, FormatStr>)
            {
                formattedMessage = String(format);
            }
			else if constexpr (std::is_convertible_v<FormatStr, std::string_view> || std::is_convertible_v<FormatStr, std::string>)
            {
                formattedMessage = String(format.c_str(), format.size());
            }
			else
            {
                formattedMessage = fmt::to_string(format);
            }

		}

		if constexpr (std::is_convertible_v<FormatStr, CString> || std::is_convertible_v<StringView, FormatStr>)
		{
			formattedMessage = fmt::vformat(format, fmt::make_format_args(args...));
		}
		else if constexpr (std::is_same_v<FormatStr, std::string_view> || std::is_same_v<FormatStr, std::string>)
		{
			formattedMessage = fmt::vformat(fmt::string_view(format.c_str(), format.size()), fmt::make_format_args(args...));
		}


		auto detail = s_EnabledTags[String(tag)];

		if (detail.enabled && detail.levelFilter <= level)
		{
			auto logger = (type == LogType::Core) 
				? getCoreLogger() 
				: getClientLogger();

			String logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";

			switch (level)
			{
				case LogLevel::Trace:
					logger->trace(fmt::runtime(logString), tag, formattedMessage);
					break;

				case LogLevel::Info:
					logger->info(fmt::runtime(logString), tag, formattedMessage);
					break;

				case LogLevel::Warn:
					logger->warn(fmt::runtime(logString), tag, formattedMessage);
					break;

				case LogLevel::Error:
					logger->error(fmt::runtime(logString), tag, formattedMessage);
					break;

				case LogLevel::Fatal:
					logger->critical(fmt::runtime(logString), tag, formattedMessage);
					break;
			}

		}
	}


	template<typename FormatStr, typename... Args>
	void VyLogger::printAssertMessage(
        LogType     type, 
        StringView  prefix, 
        FormatStr&& format, 
        Args&&...   args)
	{
		fmt::string_view fmtString;

		if constexpr (std::is_same_v<FormatStr, std::string>)
		{
			fmtString = fmt::string_view(format.c_str(), format.size());
		}
		else
		{
			fmtString = fmt::string_view(format);
		}

		auto formattedMessage = fmt::vformat(fmtString, fmt::make_format_args(args...));

		auto logger = (type == LogType::Core) ? getCoreLogger() : getClientLogger();
		logger->error("{0}: {1}", prefix, formattedMessage);

#ifdef VY_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, formattedMessage.data(), "VyAssert", MB_OK | MB_ICONERROR);
#endif
	}



	template<typename... Args>
	inline void VyLogger::printAssertMessage(
        LogType    type, 
        StringView prefix
    )
	{
		auto logger = (type == LogType::Core) ? getCoreLogger() : getClientLogger();
		logger->error("{0}", prefix);
#ifdef VY_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, "No message :(", "VyAssert", MB_OK | MB_ICONERROR);
#endif
	}
}