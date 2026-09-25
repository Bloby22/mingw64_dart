#include <windows.h>

#include <filesystem>
#include <string>

namespace {
std::wstring QuoteArgument(const std::wstring& argument) {
    std::wstring quoted = L"\"";
    for (wchar_t character : argument) {
        if (character == L'\"') {
            quoted += L'\\';
        }
        quoted += character;
    }
    quoted += L"\"";
    return quoted;
}
}

int wmain(int argc, wchar_t* argv[]) {
    wchar_t module_path[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, module_path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return 1;
    }

    const std::filesystem::path flutter_bat =
        std::filesystem::path(module_path).parent_path() /
        L".." / L"lib" / L"flutter" / L"bin" / L"flutter.bat";

    std::wstring command_line = L"cmd.exe /d /c \"\"" + flutter_bat.wstring() + L"\"";
    for (int index = 1; index < argc; ++index) {
        command_line += L' ';
        command_line += QuoteArgument(argv[index]);
    }
    command_line += L'\"';

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};

    if (!CreateProcessW(
            nullptr,
            command_line.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_UNICODE_ENVIRONMENT,
            nullptr,
            nullptr,
            &startup_info,
            &process_info)) {
        return 1;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(process_info.hProcess, &exit_code);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    return static_cast<int>(exit_code);
}