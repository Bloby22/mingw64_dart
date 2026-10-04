#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define BUF (MAX_PATH * 4)

static int file_exists(const wchar_t *p) {
    DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

/* Command line after argv[0], original quoting preserved. */
static const wchar_t *skip_argv0(const wchar_t *c) {
    if (*c == L'"') {
        c++;
        while (*c && *c != L'"') c++;
        if (*c) c++;
    } else {
        while (*c && *c != L' ' && *c != L'\t') c++;
    }
    while (*c == L' ' || *c == L'\t') c++;
    return c;
}

static void strip_last(wchar_t *p) {
    wchar_t *s = wcsrchr(p, L'\\');
    if (s) *s = 0;
}

static int run(wchar_t *cmd) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD code = 1;

    /* Ctrl+C is handled by the child process */
    SetConsoleCtrlHandler(NULL, TRUE);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        fwprintf(stderr, L"launcher: failed to start process (error %lu)\n",
                 GetLastError());
        return 1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)code;
}

int wmain(void) {
    wchar_t prefix[BUF];
    DWORD n = GetModuleFileNameW(NULL, prefix, BUF);
    if (n == 0 || n >= BUF) {
        fwprintf(stderr, L"launcher: cannot determine executable path\n");
        return 1;
    }
    strip_last(prefix);   /* <prefix>\bin */
    strip_last(prefix);   /* <prefix> */

    const wchar_t *rest = skip_argv0(GetCommandLineW());
    size_t len = wcslen(rest) + 2 * BUF;
    wchar_t *cmd = (wchar_t *)malloc(len * sizeof(wchar_t));
    if (!cmd) return 1;

    wchar_t dart[BUF];
    _snwprintf(dart, BUF, L"%ls\\lib\\dart-sdk\\bin\\dart.exe", prefix);
    if (!file_exists(dart)) {
        fwprintf(stderr, L"dart: not found: %ls\n", dart);
        free(cmd);
        return 1;
    }
    _snwprintf(cmd, len, L"\"%ls\" %ls", dart, rest);
    return run(cmd);
}
