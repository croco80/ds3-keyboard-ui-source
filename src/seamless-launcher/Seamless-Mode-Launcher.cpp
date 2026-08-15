#include "Win32-XInput-MINIMAL.h"

extern "C" void* __cdecl memset(void* destination, int value, SIZE_T count);
#pragma function(memset)
extern "C" void* __cdecl memset(void* destination, int value, SIZE_T count)
{
    volatile unsigned char* output =
        static_cast<volatile unsigned char*>(destination);
    for (SIZE_T i = 0; i < count; ++i) {
        output[i] = static_cast<unsigned char>(value);
    }
    return destination;
}

namespace {

bool append_path(wchar_t* buffer, DWORD capacity, const wchar_t* suffix)
{
    DWORD current = 0;
    while (buffer[current]) {
        ++current;
    }
    DWORD additional = 0;
    while (suffix[additional]) {
        ++additional;
    }
    if (current + additional + 1 > capacity) {
        return false;
    }
    for (DWORD i = 0; i <= additional; ++i) {
        buffer[current + i] = suffix[i];
    }
    return true;
}

bool copy_text(wchar_t* destination, DWORD capacity, const wchar_t* source)
{
    destination[0] = L'\0';
    return append_path(destination, capacity, source);
}

bool parent_directory(wchar_t* path)
{
    wchar_t* slash = nullptr;
    for (wchar_t* cursor = path; *cursor; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') {
            slash = cursor;
        }
    }
    if (!slash) {
        return false;
    }
    *slash = L'\0';
    return true;
}

bool file_exists(const wchar_t* path)
{
    return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

void show_error(const wchar_t* message)
{
    MessageBoxW(
        nullptr,
        message,
        L"DS3 Seamless + Keyboard UI",
        MB_OK | MB_ICONERROR);
}

UINT launch_game()
{
    wchar_t root[MAX_PATH]{};
    const DWORD root_length = GetModuleFileNameW(nullptr, root, MAX_PATH);
    if (root_length == 0 || root_length >= MAX_PATH || !parent_directory(root)) {
        show_error(L"Could not determine the launcher's folder.");
        return ERROR_PATH_NOT_FOUND;
    }

    wchar_t game[MAX_PATH]{};
    if (!copy_text(game, MAX_PATH, root) ||
        !append_path(game, MAX_PATH, L"\\DarkSoulsIII.exe") ||
        !file_exists(game)) {
        show_error(
            L"DarkSoulsIII.exe was not found beside this launcher.\n\n"
            L"Extract every package item into the DARK SOULS III\\Game folder.");
        return ERROR_FILE_NOT_FOUND;
    }

    wchar_t seamless[MAX_PATH]{};
    if (!copy_text(seamless, MAX_PATH, root) ||
        !append_path(seamless, MAX_PATH, L"\\SeamlessCoop\\ds3sc.dll") ||
        !file_exists(seamless)) {
        show_error(
            L"SeamlessCoop\\ds3sc.dll was not found.\n\n"
            L"Install the official Dark Souls III Seamless Co-op package first.");
        return ERROR_MOD_NOT_FOUND;
    }

    wchar_t proxy[MAX_PATH]{};
    if (!copy_text(proxy, MAX_PATH, root) ||
        !append_path(proxy, MAX_PATH, L"\\xinput1_3.dll") ||
        !file_exists(proxy)) {
        show_error(
            L"xinput1_3.dll was not found beside this launcher.\n\n"
            L"Extract every package item again.");
        return ERROR_MOD_NOT_FOUND;
    }

    if (!SetEnvironmentVariableW(L"DS3_KEYBOARD_UI_SEAMLESS", L"1")) {
        show_error(L"Could not select Seamless mode for the game process.");
        return ERROR_BAD_ENVIRONMENT;
    }

    wchar_t command[MAX_PATH + 3]{};
    command[0] = L'"';
    command[1] = L'\0';
    if (!append_path(command, MAX_PATH + 3, game) ||
        !append_path(command, MAX_PATH + 3, L"\"")) {
        show_error(L"The Dark Souls III path is too long.");
        return ERROR_PATH_NOT_FOUND;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(
            game,
            command,
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            root,
            &startup,
            &process)) {
        show_error(
            L"Dark Souls III could not be started.\n\n"
            L"Make sure Steam is running and the game is not already open.");
        return ERROR_MOD_NOT_FOUND;
    }

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return 0;
}

} // namespace

extern "C" void WINAPI LauncherEntry()
{
    ExitProcess(launch_game());
}
