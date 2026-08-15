#pragma once

// Minimal declarations required to build this small x64 proxy without the
// full Windows SDK. The layouts below match the public Win32/XInput ABI.

#define WINAPI __stdcall
#define MAX_PATH 260
#define DLL_PROCESS_ATTACH 1
#define LOAD_WITH_ALTERED_SEARCH_PATH 0x00000008
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#define ERROR_DEVICE_NOT_CONNECTED 1167
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define ERROR_BAD_ENVIRONMENT 10
#define ERROR_MOD_NOT_FOUND 126
#define MB_OK 0x00000000
#define MB_ICONERROR 0x00000010
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_BEGIN 0
#define PAGE_READWRITE 0x04
#define HEAP_ZERO_MEMORY 0x00000008
#define FALSE 0
#define TRUE 1

using BOOL = int;
using BYTE = unsigned char;
using WORD = unsigned short;
using SHORT = short;
using LONG = long;
using DWORD = unsigned long;
using UINT = unsigned int;
using SIZE_T = unsigned __int64;
using ULONG_PTR = unsigned __int64;
using LPVOID = void*;
using LPCVOID = const void*;
using HANDLE = void*;
using HMODULE = void*;
using LPCSTR = const char*;
using LPCWSTR = const wchar_t*;
using LPWSTR = wchar_t*;
using FARPROC = __int64(WINAPI*)();

#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(~static_cast<ULONG_PTR>(0))

struct GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
};

struct XINPUT_GAMEPAD {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
};

struct XINPUT_STATE {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
};

struct XINPUT_VIBRATION {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
};

struct XINPUT_CAPABILITIES {
    BYTE Type;
    BYTE SubType;
    WORD Flags;
    XINPUT_GAMEPAD Gamepad;
    XINPUT_VIBRATION Vibration;
};

struct XINPUT_BATTERY_INFORMATION {
    BYTE BatteryType;
    BYTE BatteryLevel;
};

struct XINPUT_KEYSTROKE {
    WORD VirtualKey;
    wchar_t Unicode;
    WORD Flags;
    BYTE UserIndex;
    BYTE HidCode;
};

using PXINPUT_KEYSTROKE = XINPUT_KEYSTROKE*;

struct STARTUPINFOW {
    DWORD cb;
    LPWSTR lpReserved;
    LPWSTR lpDesktop;
    LPWSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    BYTE* lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
};

struct PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
};

extern "C" {
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE object);
__declspec(dllimport) HANDLE WINAPI CreateFileW(
    LPCWSTR file_name,
    DWORD desired_access,
    DWORD share_mode,
    LPVOID security_attributes,
    DWORD creation_disposition,
    DWORD flags_and_attributes,
    HANDLE template_file);
__declspec(dllimport) BOOL WINAPI CreateProcessW(
    LPCWSTR application_name,
    LPWSTR command_line,
    LPVOID process_attributes,
    LPVOID thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    STARTUPINFOW* startup_info,
    PROCESS_INFORMATION* process_information);
__declspec(dllimport) BOOL WINAPI DisableThreadLibraryCalls(HMODULE module);
__declspec(dllimport) void WINAPI ExitProcess(UINT exit_code);
__declspec(dllimport) BOOL WINAPI FlushInstructionCache(
    HANDLE process, LPCVOID base_address, SIZE_T size);
__declspec(dllimport) HANDLE WINAPI GetCurrentProcess();
__declspec(dllimport) DWORD WINAPI GetEnvironmentVariableW(
    LPCWSTR name, LPWSTR buffer, DWORD size);
__declspec(dllimport) DWORD WINAPI GetFileAttributesW(LPCWSTR file_name);
__declspec(dllimport) DWORD WINAPI GetFileSize(HANDLE file, DWORD* high_size);
__declspec(dllimport) DWORD WINAPI GetModuleFileNameW(HMODULE module, wchar_t* file_name, DWORD size);
__declspec(dllimport) HMODULE WINAPI GetModuleHandleW(LPCWSTR module_name);
__declspec(dllimport) UINT WINAPI GetPrivateProfileIntW(
    LPCWSTR application_name,
    LPCWSTR key_name,
    int default_value,
    LPCWSTR file_name);
__declspec(dllimport) FARPROC WINAPI GetProcAddress(HMODULE module, LPCSTR name);
__declspec(dllimport) HANDLE WINAPI GetProcessHeap();
__declspec(dllimport) UINT WINAPI GetSystemDirectoryW(wchar_t* buffer, UINT size);
__declspec(dllimport) HMODULE WINAPI LoadLibraryExW(LPCWSTR file_name, HANDLE file, DWORD flags);
__declspec(dllimport) HMODULE WINAPI LoadLibraryW(LPCWSTR file_name);
__declspec(dllimport) LPVOID WINAPI HeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes);
__declspec(dllimport) BOOL WINAPI HeapFree(HANDLE heap, DWORD flags, LPVOID memory);
__declspec(dllimport) BOOL WINAPI ReadFile(
    HANDLE file,
    LPVOID buffer,
    DWORD bytes_to_read,
    DWORD* bytes_read,
    LPVOID overlapped);
__declspec(dllimport) DWORD WINAPI SetFilePointer(
    HANDLE file, LONG distance, LONG* distance_high, DWORD move_method);
__declspec(dllimport) BOOL WINAPI SetEndOfFile(HANDLE file);
__declspec(dllimport) BOOL WINAPI SetFileAttributesW(LPCWSTR file_name, DWORD attributes);
__declspec(dllimport) BOOL WINAPI SetEnvironmentVariableW(LPCWSTR name, LPCWSTR value);
__declspec(dllimport) void WINAPI Sleep(DWORD milliseconds);
__declspec(dllimport) BOOL WINAPI VirtualProtect(
    LPVOID address, SIZE_T size, DWORD new_protection, DWORD* old_protection);
__declspec(dllimport) BOOL WINAPI WriteFile(
    HANDLE file,
    LPCVOID buffer,
    DWORD bytes_to_write,
    DWORD* bytes_written,
    LPVOID overlapped);
__declspec(dllimport) int WINAPI MessageBoxW(
    HANDLE window, LPCWSTR text, LPCWSTR caption, UINT type);
}
