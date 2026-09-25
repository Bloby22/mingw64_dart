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

    std::wstring parameters;
    for (int index = 1; index < argc; ++index) {
        if (!parameters.empty()) {
            parameters += L' ';
        }
        parameters += QuoteArgument(argv[index]);
    }

    SHELLEXECUTEINFOW execute_info{};
    execute_info.cbSize = sizeof(execute_info);
    execute_info.fMask = SEE_MASK_NOCLOSEPROCESS;
    execute_info.lpFile = flutter_bat.c_str();
    execute_info.lpParameters = parameters.c_str();
    execute_info.nShow = SW_SHOW;

    if (!ShellExecuteExW(&execute_info)) {
        return 1;
    }

    WaitForSingleObject(execute_info.hProcess, INFINITE);
    DWORD exit_code = 1;
    GetExitCodeProcess(execute_info.hProcess, &exit_code);
    CloseHandle(execute_info.hProcess);
    return static_cast<int>(exit_code);
}
