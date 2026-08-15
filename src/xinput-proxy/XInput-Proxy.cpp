#include "Win32-XInput-MINIMAL.h"

extern "C" void* __cdecl memset(void* destination, int value, SIZE_T count);
extern "C" void* __cdecl memcpy(
    void* destination, const void* source, SIZE_T count);
#pragma function(memset)
#pragma function(memcpy)
extern "C" void* __cdecl memset(void* destination, int value, SIZE_T count)
{
    volatile unsigned char* output =
        static_cast<volatile unsigned char*>(destination);
    for (SIZE_T i = 0; i < count; ++i) {
        output[i] = static_cast<unsigned char>(value);
    }
    return destination;
}

extern "C" void* __cdecl memcpy(
    void* destination, const void* source, SIZE_T count)
{
    volatile unsigned char* output =
        static_cast<volatile unsigned char*>(destination);
    const volatile unsigned char* input =
        static_cast<const volatile unsigned char*>(source);
    for (SIZE_T i = 0; i < count; ++i) {
        output[i] = input[i];
    }
    return destination;
}

namespace {

HMODULE g_real_xinput = nullptr;
HMODULE g_modengine = nullptr;

using XInputGetStateFn = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);
using XInputSetStateFn = DWORD(WINAPI*)(DWORD, XINPUT_VIBRATION*);
using XInputGetCapabilitiesFn = DWORD(WINAPI*)(DWORD, DWORD, XINPUT_CAPABILITIES*);
using XInputEnableFn = void(WINAPI*)(BOOL);
using XInputGetDSoundAudioDeviceGuidsFn = DWORD(WINAPI*)(DWORD, GUID*, GUID*);
using XInputGetBatteryInformationFn =
    DWORD(WINAPI*)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION*);
using XInputGetKeystrokeFn = DWORD(WINAPI*)(DWORD, DWORD, PXINPUT_KEYSTROKE);
using XInputWaitForGuideButtonFn = DWORD(WINAPI*)(DWORD, DWORD, void*);
using XInputCancelGuideButtonWaitFn = DWORD(WINAPI*)(DWORD);
using XInputPowerOffControllerFn = DWORD(WINAPI*)(DWORD);

XInputGetStateFn g_get_state = nullptr;
XInputSetStateFn g_set_state = nullptr;
XInputGetCapabilitiesFn g_get_capabilities = nullptr;
XInputEnableFn g_enable = nullptr;
XInputGetDSoundAudioDeviceGuidsFn g_get_audio_guids = nullptr;
XInputGetBatteryInformationFn g_get_battery = nullptr;
XInputGetKeystrokeFn g_get_keystroke = nullptr;
XInputGetStateFn g_get_state_ex = nullptr;
XInputWaitForGuideButtonFn g_wait_for_guide = nullptr;
XInputCancelGuideButtonWaitFn g_cancel_guide_wait = nullptr;
XInputPowerOffControllerFn g_power_off = nullptr;

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

bool executable_name_equals(const wchar_t* left, const wchar_t* right)
{
    for (;;) {
        wchar_t a = *left++;
        wchar_t b = *right++;
        if (a >= L'A' && a <= L'Z') {
            a += L'a' - L'A';
        }
        if (b >= L'A' && b <= L'Z') {
            b += L'a' - L'A';
        }
        if (a != b) {
            return false;
        }
        if (!a) {
            return true;
        }
    }
}

FARPROC ordinal(unsigned short value)
{
    return GetProcAddress(
        g_real_xinput,
        reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(value)));
}

bool current_process_is_game()
{
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }

    const wchar_t* name = path;
    for (const wchar_t* cursor = path; *cursor; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') {
            name = cursor + 1;
        }
    }
    return executable_name_equals(name, L"DarkSoulsIII.exe");
}

bool load_real_xinput()
{
    wchar_t path[MAX_PATH]{};
    const UINT length = GetSystemDirectoryW(path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH ||
        !append_path(path, MAX_PATH, L"\\xinput1_4.dll")) {
        return false;
    }

    g_real_xinput = LoadLibraryW(path);
    if (!g_real_xinput) {
        return false;
    }

    g_get_state = reinterpret_cast<XInputGetStateFn>(
        GetProcAddress(g_real_xinput, "XInputGetState"));
    g_set_state = reinterpret_cast<XInputSetStateFn>(
        GetProcAddress(g_real_xinput, "XInputSetState"));
    g_get_capabilities = reinterpret_cast<XInputGetCapabilitiesFn>(
        GetProcAddress(g_real_xinput, "XInputGetCapabilities"));
    g_enable = reinterpret_cast<XInputEnableFn>(
        GetProcAddress(g_real_xinput, "XInputEnable"));
    // XInput 1.4 no longer exposes XInputGetDSoundAudioDeviceGuids.
    // Dark Souls III 1.15.2 imports only GetState and SetState.
    g_get_audio_guids = nullptr;
    g_get_battery = reinterpret_cast<XInputGetBatteryInformationFn>(
        GetProcAddress(g_real_xinput, "XInputGetBatteryInformation"));
    g_get_keystroke = reinterpret_cast<XInputGetKeystrokeFn>(
        GetProcAddress(g_real_xinput, "XInputGetKeystroke"));
    g_get_state_ex = reinterpret_cast<XInputGetStateFn>(ordinal(100));
    g_wait_for_guide = reinterpret_cast<XInputWaitForGuideButtonFn>(ordinal(101));
    g_cancel_guide_wait = reinterpret_cast<XInputCancelGuideButtonWaitFn>(ordinal(102));
    g_power_off = reinterpret_cast<XInputPowerOffControllerFn>(ordinal(103));
    return g_get_state && g_set_state;
}

bool seamless_mode_requested()
{
    wchar_t value[2]{};
    const DWORD length = GetEnvironmentVariableW(
        L"DS3_KEYBOARD_UI_SEAMLESS", value, 2);
    return length == 1 && value[0] == L'1';
}

bool build_settings_path(
    const wchar_t* game_root, wchar_t* settings, DWORD capacity)
{
    return copy_text(settings, capacity, game_root) &&
        append_path(
            settings,
            capacity,
            L"\\DS3 Keyboard Icons\\settings.ini");
}

bool setting_enabled(
    const wchar_t* game_root,
    const wchar_t* section,
    const wchar_t* key)
{
    wchar_t settings[MAX_PATH]{};
    if (!build_settings_path(game_root, settings, MAX_PATH)) {
        return false;
    }

    return GetPrivateProfileIntW(section, key, 0, settings) != 0;
}

DWORD text_length(const wchar_t* text)
{
    DWORD length = 0;
    while (text[length]) {
        ++length;
    }
    return length;
}

wchar_t* find_text(wchar_t* text, const wchar_t* search)
{
    const DWORD search_length = text_length(search);
    if (!search_length) {
        return text;
    }

    for (wchar_t* cursor = text; *cursor; ++cursor) {
        DWORD matched = 0;
        while (matched < search_length &&
               cursor[matched] == search[matched]) {
            ++matched;
        }
        if (matched == search_length) {
            return cursor;
        }
    }
    return nullptr;
}

bool replace_xml_value(
    wchar_t* xml,
    DWORD capacity,
    const wchar_t* opening_tag,
    const wchar_t* closing_tag,
    const wchar_t* replacement)
{
    wchar_t* opening = find_text(xml, opening_tag);
    if (!opening) {
        return false;
    }
    wchar_t* value = opening + text_length(opening_tag);
    wchar_t* closing = find_text(value, closing_tag);
    if (!closing) {
        return false;
    }

    const DWORD total_length = text_length(xml);
    const DWORD old_length = static_cast<DWORD>(closing - value);
    const DWORD new_length = text_length(replacement);
    const DWORD value_offset = static_cast<DWORD>(value - xml);
    const DWORD closing_offset = value_offset + old_length;

    if (new_length > old_length) {
        const DWORD growth = new_length - old_length;
        if (total_length + growth + 1 > capacity) {
            return false;
        }
        for (DWORD index = total_length + 1; index > closing_offset; --index) {
            xml[index + growth - 1] = xml[index - 1];
        }
    } else if (old_length > new_length) {
        const DWORD shrink = old_length - new_length;
        for (DWORD index = closing_offset; index <= total_length; ++index) {
            xml[index - shrink] = xml[index];
        }
    }

    for (DWORD index = 0; index < new_length; ++index) {
        value[index] = replacement[index];
    }
    return true;
}

const wchar_t* default_graphics_config()
{
    return
        L"<?xml version=\"1.0\" encoding=\"UTF-16\" ?>\r\n"
        L"<config><ScreenMode>FULLSCREEN</ScreenMode>\r\n"
        L"<Resolution-WindowScreenWidth>2560</Resolution-WindowScreenWidth>\r\n"
        L"<Resolution-WindowScreenHeight>1600</Resolution-WindowScreenHeight>\r\n"
        L"<Resolution-FullScreenWidth>2560</Resolution-FullScreenWidth>\r\n"
        L"<Resolution-FullScreenHeight>1600</Resolution-FullScreenHeight>\r\n"
        L"<Auto-detectBestRenderingSettings>OFF</Auto-detectBestRenderingSettings>\r\n"
        L"<QualitySetting>CUSTOM</QualitySetting>\r\n"
        L"<TextureQuality>MAX</TextureQuality>\r\n"
        L"<Antialiasing>ON</Antialiasing>\r\n"
        L"<SSAO>HIGH</SSAO>\r\n"
        L"<DepthOfField>MAX</DepthOfField>\r\n"
        L"<MotionBlur>HIGH</MotionBlur>\r\n"
        L"<ShadowQuality>MAX</ShadowQuality>\r\n"
        L"<LightingQuality>MAX</LightingQuality>\r\n"
        L"<EffectsQuality>MAX</EffectsQuality>\r\n"
        L"<ReflectionQuality>MAX</ReflectionQuality>\r\n"
        L"<WaterSurfaceQuality>HIGH</WaterSurfaceQuality>\r\n"
        L"<ShadeQuality>MAX</ShadeQuality>\r\n"
        L"</config>\r\n";
}

bool format_decimal(DWORD value, wchar_t* output, DWORD capacity)
{
    wchar_t reversed[16]{};
    DWORD digits = 0;
    do {
        if (digits >= 15) {
            return false;
        }
        reversed[digits++] = static_cast<wchar_t>(L'0' + (value % 10));
        value /= 10;
    } while (value);

    if (digits + 1 > capacity) {
        return false;
    }
    for (DWORD index = 0; index < digits; ++index) {
        output[index] = reversed[digits - index - 1];
    }
    output[digits] = L'\0';
    return true;
}

bool write_utf16_file(const wchar_t* path, const wchar_t* text)
{
    HANDLE file = CreateFileW(
        path,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    const BYTE bom[2] = { 0xff, 0xfe };
    DWORD written = 0;
    const DWORD text_bytes = text_length(text) * sizeof(wchar_t);
    const bool ok =
        WriteFile(file, bom, sizeof(bom), &written, nullptr) &&
        written == sizeof(bom) &&
        WriteFile(file, text, text_bytes, &written, nullptr) &&
        written == text_bytes;
    CloseHandle(file);
    return ok;
}

bool write_default_graphics_config(
    const wchar_t* path, DWORD width, DWORD height)
{
    wchar_t width_text[16]{};
    wchar_t height_text[16]{};
    HANDLE heap = GetProcessHeap();
    wchar_t* xml = static_cast<wchar_t*>(
        HeapAlloc(heap, HEAP_ZERO_MEMORY, 2048 * sizeof(wchar_t)));
    if (!format_decimal(width, width_text, 16) ||
        !format_decimal(height, height_text, 16) ||
        !xml ||
        !copy_text(xml, 2048, default_graphics_config())) {
        if (xml) {
            HeapFree(heap, 0, xml);
        }
        return false;
    }

    const bool updated =
        replace_xml_value(
            xml,
            2048,
            L"<Resolution-WindowScreenWidth>",
            L"</Resolution-WindowScreenWidth>",
            width_text) &&
        replace_xml_value(
            xml,
            2048,
            L"<Resolution-WindowScreenHeight>",
            L"</Resolution-WindowScreenHeight>",
            height_text) &&
        replace_xml_value(
            xml,
            2048,
            L"<Resolution-FullScreenWidth>",
            L"</Resolution-FullScreenWidth>",
            width_text) &&
        replace_xml_value(
            xml,
            2048,
            L"<Resolution-FullScreenHeight>",
            L"</Resolution-FullScreenHeight>",
            height_text);
    const bool result = updated && write_utf16_file(path, xml);
    HeapFree(heap, 0, xml);
    return result;
}

bool refresh_graphics_config(DWORD width, DWORD height)
{
    wchar_t path[MAX_PATH]{};
    const DWORD appdata_length = GetEnvironmentVariableW(
        L"APPDATA", path, MAX_PATH);
    if (appdata_length == 0 || appdata_length >= MAX_PATH ||
        !append_path(path, MAX_PATH, L"\\DarkSoulsIII\\GraphicsConfig.xml")) {
        return false;
    }

    const DWORD original_attributes = GetFileAttributesW(path);
    const bool existed = original_attributes != INVALID_FILE_ATTRIBUTES;
    if (existed && (original_attributes & FILE_ATTRIBUTE_READONLY)) {
        SetFileAttributesW(
            path, original_attributes & ~FILE_ATTRIBUTE_READONLY);
    }

    HANDLE file = CreateFileW(
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    bool result = false;
    wchar_t width_text[16]{};
    wchar_t height_text[16]{};
    if (!format_decimal(width, width_text, 16) ||
        !format_decimal(height, height_text, 16)) {
        return false;
    }
    if (file != INVALID_HANDLE_VALUE) {
        DWORD high_size = 0;
        const DWORD file_size = GetFileSize(file, &high_size);
        if (high_size == 0 && file_size >= 2 && file_size < 1024 * 1024) {
            HANDLE heap = GetProcessHeap();
            BYTE* raw = static_cast<BYTE*>(
                HeapAlloc(heap, HEAP_ZERO_MEMORY, file_size + 2));
            DWORD bytes_read = 0;
            if (raw && ReadFile(file, raw, file_size, &bytes_read, nullptr) &&
                bytes_read == file_size && raw[0] == 0xff && raw[1] == 0xfe &&
                ((file_size - 2) % sizeof(wchar_t)) == 0) {
                const DWORD character_count =
                    (file_size - 2) / sizeof(wchar_t);
                const DWORD capacity = character_count + 256;
                wchar_t* xml = static_cast<wchar_t*>(
                    HeapAlloc(
                        heap,
                        HEAP_ZERO_MEMORY,
                        capacity * sizeof(wchar_t)));
                if (xml) {
                    const wchar_t* source =
                        reinterpret_cast<const wchar_t*>(raw + 2);
                    for (DWORD index = 0; index < character_count; ++index) {
                        xml[index] = source[index];
                    }
                    xml[character_count] = L'\0';

                    const bool updated =
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<ScreenMode>",
                            L"</ScreenMode>",
                            L"FULLSCREEN") &&
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<Resolution-WindowScreenWidth>",
                            L"</Resolution-WindowScreenWidth>",
                            width_text) &&
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<Resolution-WindowScreenHeight>",
                            L"</Resolution-WindowScreenHeight>",
                            height_text) &&
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<Resolution-FullScreenWidth>",
                            L"</Resolution-FullScreenWidth>",
                            width_text) &&
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<Resolution-FullScreenHeight>",
                            L"</Resolution-FullScreenHeight>",
                            height_text) &&
                        replace_xml_value(
                            xml,
                            capacity,
                            L"<Auto-detectBestRenderingSettings>",
                            L"</Auto-detectBestRenderingSettings>",
                            L"OFF");

                    if (updated &&
                        SetFilePointer(file, 0, nullptr, FILE_BEGIN) !=
                            INVALID_FILE_ATTRIBUTES) {
                        const BYTE bom[2] = { 0xff, 0xfe };
                        DWORD written = 0;
                        const DWORD xml_bytes =
                            text_length(xml) * sizeof(wchar_t);
                        result =
                            WriteFile(file, bom, sizeof(bom), &written, nullptr) &&
                            written == sizeof(bom) &&
                            WriteFile(
                                file, xml, xml_bytes, &written, nullptr) &&
                            written == xml_bytes &&
                            SetEndOfFile(file);
                    }
                    HeapFree(heap, 0, xml);
                }
            }
            if (raw) {
                HeapFree(heap, 0, raw);
            }
        }
        CloseHandle(file);
    }

    if (!result) {
        result = write_default_graphics_config(path, width, height);
    }

    if (existed && (original_attributes & FILE_ATTRIBUTE_READONLY)) {
        SetFileAttributesW(path, original_attributes);
    }
    return result;
}

bool patch_resolution_entries(
    DWORD old_width, DWORD old_height, DWORD width, DWORD height)
{
    BYTE* base = static_cast<BYTE*>(GetModuleHandleW(nullptr));
    if (!base || base[0] != 'M' || base[1] != 'Z') {
        return false;
    }

    const DWORD pe_offset = *reinterpret_cast<DWORD*>(base + 0x3c);
    if (pe_offset > 1024 * 1024 ||
        *reinterpret_cast<DWORD*>(base + pe_offset) != 0x00004550) {
        return false;
    }
    const DWORD image_size =
        *reinterpret_cast<DWORD*>(base + pe_offset + 0x50);
    if (image_size < 4096 || image_size > 512 * 1024 * 1024) {
        return false;
    }

    const WORD section_count =
        *reinterpret_cast<WORD*>(base + pe_offset + 6);
    const WORD optional_header_size =
        *reinterpret_cast<WORD*>(base + pe_offset + 20);
    BYTE* section = base + pe_offset + 24 + optional_header_size;
    if (section_count == 0 || section_count > 96 ||
        section + static_cast<DWORD>(section_count) * 40 >
            base + image_size) {
        return false;
    }

    BYTE* matches[8]{};
    DWORD match_count = 0;
    for (WORD section_index = 0;
         section_index < section_count;
         ++section_index, section += 40) {
        const DWORD virtual_size =
            *reinterpret_cast<DWORD*>(section + 8);
        const DWORD virtual_address =
            *reinterpret_cast<DWORD*>(section + 12);
        const DWORD raw_size =
            *reinterpret_cast<DWORD*>(section + 16);
        const DWORD characteristics =
            *reinterpret_cast<DWORD*>(section + 36);
        DWORD section_size = virtual_size > raw_size ? virtual_size : raw_size;
        if (!(characteristics & 0x40000000) ||
            virtual_address >= image_size) {
            continue;
        }
        if (section_size > image_size - virtual_address) {
            section_size = image_size - virtual_address;
        }

        BYTE* section_base = base + virtual_address;
        for (DWORD offset = 0; offset + 8 <= section_size; ++offset) {
            if (*reinterpret_cast<DWORD*>(section_base + offset) == old_width &&
                *reinterpret_cast<DWORD*>(section_base + offset + 4) ==
                    old_height) {
                if (match_count >= 8) {
                    return false;
                }
                matches[match_count++] = section_base + offset;
            }
        }
    }
    if (match_count == 0) {
        return false;
    }

    DWORD patched_count = 0;
    for (; patched_count < match_count; ++patched_count) {
        BYTE* match = matches[patched_count];
        DWORD old_protection = 0;
        if (!VirtualProtect(match, 8, PAGE_READWRITE, &old_protection)) {
            break;
        }
        *reinterpret_cast<DWORD*>(match) = width;
        *reinterpret_cast<DWORD*>(match + 4) = height;
        DWORD ignored = 0;
        VirtualProtect(match, 8, old_protection, &ignored);
        FlushInstructionCache(GetCurrentProcess(), match, 8);
    }

    if (patched_count == match_count) {
        return true;
    }

    // Avoid leaving a partly changed process if a later entry was protected.
    for (DWORD index = 0; index < patched_count; ++index) {
        DWORD old_protection = 0;
        if (VirtualProtect(
                matches[index], 8, PAGE_READWRITE, &old_protection)) {
            *reinterpret_cast<DWORD*>(matches[index]) = old_width;
            *reinterpret_cast<DWORD*>(matches[index] + 4) = old_height;
            DWORD ignored = 0;
            VirtualProtect(matches[index], 8, old_protection, &ignored);
            FlushInstructionCache(
                GetCurrentProcess(), matches[index], 8);
        }
    }
    return false;
}

struct ResolutionSelection {
    DWORD source_width;
    DWORD source_height;
    DWORD width;
    DWORD height;
};

bool select_resolution(
    const wchar_t* game_root, ResolutionSelection* selection)
{
    struct Candidate {
        const wchar_t* key;
        DWORD source_width;
        DWORD source_height;
        DWORD width;
        DWORD height;
    };
    const Candidate candidates[] = {
        // 1280x800 uses the established Steam Deck 1280x720 replacement.
        { L"Enable1280x800", 1280, 720, 1280, 800 },

        // Other modes use the established customizable 1920x1080 slot.
        { L"Enable1440x900", 1920, 1080, 1440, 900 },
        { L"Enable1680x1050", 1920, 1080, 1680, 1050 },
        { L"Enable1920x1200", 1920, 1080, 1920, 1200 },
        { L"Enable2560x1600", 1920, 1080, 2560, 1600 },
        { L"Enable2880x1800", 1920, 1080, 2880, 1800 },
        { L"Enable3840x2400", 1920, 1080, 3840, 2400 }
    };

    DWORD enabled_count = 0;
    for (const Candidate& candidate : candidates) {
        if (setting_enabled(game_root, L"Resolution", candidate.key)) {
            selection->source_width = candidate.source_width;
            selection->source_height = candidate.source_height;
            selection->width = candidate.width;
            selection->height = candidate.height;
            ++enabled_count;
        }
    }
    return enabled_count == 1;
}

void apply_resolution_fix_if_enabled(const wchar_t* game_root)
{
    ResolutionSelection selection{};
    if (!select_resolution(game_root, &selection)) {
        return;
    }

    // The executable file is never written; this changes the current process.
    // Only refresh the XML if the matching original entries were found.
    if (patch_resolution_entries(
            selection.source_width,
            selection.source_height,
            selection.width,
            selection.height)) {
        refresh_graphics_config(selection.width, selection.height);
    }
}

bool sixteen_ten_enabled(const wchar_t* game_root)
{
    return setting_enabled(game_root, L"UI", L"Enable16x10");
}

bool game_root_from_proxy(
    HMODULE proxy_module, wchar_t* root, DWORD capacity)
{
    const DWORD length = GetModuleFileNameW(proxy_module, root, capacity);
    return length != 0 && length < capacity && parent_directory(root);
}

void load_keyboard_icons_from_root(const wchar_t* root)
{

    const bool seamless = seamless_mode_requested();
    const bool sixteen_ten = sixteen_ten_enabled(root);
    const wchar_t* runtime = L"\\DS3 Keyboard Icons\\modengine2";
    if (seamless && sixteen_ten) {
        runtime = L"\\DS3 Keyboard Icons\\modengine2-seamless-16x10";
    } else if (seamless) {
        runtime = L"\\DS3 Keyboard Icons\\modengine2-seamless";
    } else if (sixteen_ten) {
        runtime = L"\\DS3 Keyboard Icons\\modengine2-16x10";
    }

    wchar_t modengine[MAX_PATH]{};
    if (!copy_text(modengine, MAX_PATH, root) ||
        !append_path(modengine, MAX_PATH, runtime) ||
        !append_path(modengine, MAX_PATH, L"\\bin\\modengine2.dll")) {
        return;
    }

    if (GetFileAttributesW(modengine) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    g_modengine = LoadLibraryExW(modengine, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}

} // namespace

extern "C" DWORD WINAPI XInputGetState(DWORD index, XINPUT_STATE* state)
{
    return g_get_state ? g_get_state(index, state) : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI XInputSetState(DWORD index, XINPUT_VIBRATION* vibration)
{
    return g_set_state ? g_set_state(index, vibration) : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI XInputGetCapabilities(
    DWORD index, DWORD flags, XINPUT_CAPABILITIES* capabilities)
{
    return g_get_capabilities
        ? g_get_capabilities(index, flags, capabilities)
        : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" void WINAPI XInputEnable(BOOL enable)
{
    if (g_enable) {
        g_enable(enable);
    }
}

extern "C" DWORD WINAPI XInputGetDSoundAudioDeviceGuids(
    DWORD index, GUID* render_guid, GUID* capture_guid)
{
    return g_get_audio_guids
        ? g_get_audio_guids(index, render_guid, capture_guid)
        : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI XInputGetBatteryInformation(
    DWORD index, BYTE device_type, XINPUT_BATTERY_INFORMATION* battery)
{
    return g_get_battery
        ? g_get_battery(index, device_type, battery)
        : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI XInputGetKeystroke(
    DWORD index, DWORD reserved, PXINPUT_KEYSTROKE keystroke)
{
    return g_get_keystroke
        ? g_get_keystroke(index, reserved, keystroke)
        : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI ProxyOrdinal100(DWORD index, XINPUT_STATE* state)
{
    return g_get_state_ex ? g_get_state_ex(index, state) : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI ProxyOrdinal101(DWORD index, DWORD flags, void* event_handle)
{
    return g_wait_for_guide
        ? g_wait_for_guide(index, flags, event_handle)
        : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI ProxyOrdinal102(DWORD index)
{
    return g_cancel_guide_wait ? g_cancel_guide_wait(index) : ERROR_DEVICE_NOT_CONNECTED;
}

extern "C" DWORD WINAPI ProxyOrdinal103(DWORD index)
{
    return g_power_off ? g_power_off(index) : ERROR_DEVICE_NOT_CONNECTED;
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        if (!load_real_xinput()) {
            return FALSE;
        }
        if (current_process_is_game()) {
            wchar_t root[MAX_PATH]{};
            if (game_root_from_proxy(module, root, MAX_PATH)) {
                apply_resolution_fix_if_enabled(root);
                load_keyboard_icons_from_root(root);
            }
        }
    }
    return TRUE;
}
