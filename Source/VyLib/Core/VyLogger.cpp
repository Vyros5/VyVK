#include <VyLib/Core/VyLogger.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <VyLib/STL/Path.h>

#define VY_HAS_CONSOLE !VY_DIST

namespace Vy
{
	Shared<spdlog::logger> VyLogger::s_CoreLogger;
	Shared<spdlog::logger> VyLogger::s_ClientLogger;
	Shared<spdlog::logger> VyLogger::s_EditorConsoleLogger;

	void VyLogger::init()
	{
		// Create "logs" directory if doesn't exist
		// String logsDirectory = "logs";
		// if (!std::filesystem::exists(logsDirectory))
        // {
        //     std::filesystem::create_directories(logsDirectory);
        // }

		TVector<spdlog::sink_ptr> vyEngineSinks = {
			// MakeShared<spdlog::sinks::basic_file_sink_mt>("Logs/VyEngine.log", true),
#if VY_HAS_CONSOLE
			MakeShared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		TVector<spdlog::sink_ptr> appSinks = {
			// MakeShared<spdlog::sinks::basic_file_sink_mt>("Logs/App.log", true),
#if VY_HAS_CONSOLE
			MakeShared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		TVector<spdlog::sink_ptr> editorConsoleSinks = {
			// MakeShared<spdlog::sinks::basic_file_sink_mt>("Logs/App.log", true),
#if VY_HAS_CONSOLE
			// MakeShared<EditorConsoleSink>(1)
#endif
		};

		// vyEngineSinks[0]->set_pattern("[%T] [%l] %n: %v");
		// appSinks[0]->set_pattern("[%T] [%l] %n: %v");

#if VY_HAS_CONSOLE
		// vyEngineSinks[1]->set_pattern("%^[%T] %n: %v%$");
		// appSinks[1]->set_pattern("%^[%T] %n: %v%$");
		vyEngineSinks[0]->set_pattern("%^[%T] %n: %v%$");
		appSinks[0]->set_pattern("%^[%T] %n: %v%$");
        
		for (auto sink : editorConsoleSinks)
        {
            sink->set_pattern("%^%v%$");
        }
#endif

		s_CoreLogger = MakeShared<spdlog::logger>("VYENGINE", vyEngineSinks.begin(), vyEngineSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = MakeShared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);

		s_EditorConsoleLogger = MakeShared<spdlog::logger>("CONSOLE", editorConsoleSinks.begin(), editorConsoleSinks.end());
		s_EditorConsoleLogger->set_level(spdlog::level::trace);

		VY_INFO("VyLogger Initialized!");
	}


	void VyLogger::shutdown()
	{
		s_EditorConsoleLogger.reset();
		s_ClientLogger.reset();
		s_CoreLogger.reset();
		spdlog::drop_all();
	}
}