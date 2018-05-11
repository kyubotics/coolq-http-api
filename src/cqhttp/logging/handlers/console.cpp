#include "./console.h"

#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <iostream>

using namespace std;

namespace cqhttp::logging {
    static const auto LOGGER_NAME = "console";

    static void redirect_stdio_to_console() {
        // see https://stackoverflow.com/a/46050762

        // get STDOUT handle
        const auto console_output = GetStdHandle(STD_OUTPUT_HANDLE);
        const auto system_output = _open_osfhandle(intptr_t(console_output), _O_TEXT);
        auto c_output_handle = _fdopen(system_output, "w");

        // get STDERR handle
        const auto console_error = GetStdHandle(STD_ERROR_HANDLE);
        const auto system_error = _open_osfhandle(intptr_t(console_error), _O_TEXT);
        auto c_error_handle = _fdopen(system_error, "w");

        // get STDIN handle
        const auto console_input = GetStdHandle(STD_INPUT_HANDLE);
        const auto system_input = _open_osfhandle(intptr_t(console_input), _O_TEXT);
        auto c_input_handle = _fdopen(system_input, "r");

        // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
        ios::sync_with_stdio(true);

        // redirect the CRT standard input, output, and error handles to the console
        freopen_s(&c_input_handle, "CONIN$", "r", stdin);
        freopen_s(&c_output_handle, "CONOUT$", "w", stdout);
        freopen_s(&c_error_handle, "CONOUT$", "w", stderr);

        // clear the error state for each of the C++ standard stream objects. We need to do this, as
        // attempts to access the standard streams before they refer to a valid target will cause the
        // iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
        // to always occur during startup regardless of whether anything has been read from or written to
        // the console or not.
        std::wcout.clear();
        std::cout.clear();
        std::wcerr.clear();
        std::cerr.clear();
        std::wcin.clear();
        std::cin.clear();
    }

    void ConsoleHandler::init() {
        // create a console for logging
        AllocConsole();
        redirect_stdio_to_console();

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitleW(L"CoolQ HTTP API 插件 - 日志 (请不要关闭此控制台窗口)");
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_EXTENDED_FLAGS); // disable input

        logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%L] %v");
        logger_->flush_on(spdlog::level::trace);
        logger_->set_level(spdlog::level::debug);
    }

    void ConsoleHandler::destroy() {
        spdlog::drop(LOGGER_NAME);
        SetConsoleTitleW(L"CoolQ HTTP API 插件 - 日志 (如果此控制台没有自动关闭，你可以手动关闭)");
        FreeConsole();
    }
} // namespace cqhttp::logging
