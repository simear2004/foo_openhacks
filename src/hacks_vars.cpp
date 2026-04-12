#include "pch.h"
#include "hacks_vars.h"
#include "win32_utils.h"
#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <algorithm>

namespace OpenHacksVars
{
    std::string g_fb2k_root;
    std::string g_fb2k_profile;
    static std::vector<std::wstring> g_loadedFonts;
    static Gdiplus::PrivateFontCollection* g_pGlobalFontCollection = nullptr;

    // {A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
    static const GUID cfg_guid_show_main_menu = {0xa1b2c3d4, 0xe5f6, 0x7890, {0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90}};
    // {B2C3D4E5-F6A7-8901-BCDE-F12345678901}
    static const GUID cfg_guid_show_status_bar = {0xb2c3d4e5, 0xf6a7, 0x8901, {0xbc, 0xde, 0xf1, 0x23, 0x45, 0x67, 0x89, 0x01}};
    // {C3D4E5F6-A7B8-9012-CDEF-123456789012}
    static const GUID cfg_guid_main_window_frame_style = {0xc3d4e5f6, 0xa7b8, 0x9012, {0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12}};
    // {D4E5F6A7-B8C9-0123-DEF1-234567890123}
    static const GUID cfg_guid_enable_win10_shadow = {0xd4e5f6a7, 0xb8c9, 0x0123, {0xde, 0xf1, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23}};
    // {E5F6A7B8-C9D0-1234-EF12-345678901234}
    static const GUID cfg_guid_disable_resize_maximized = {0xe5f6a7b8, 0xc9d0, 0x1234, {0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34}};
    // {F6A7B8C9-D0E1-2345-F123-456789012345}
    static const GUID cfg_guid_disable_resize_fullscreen = {0xf6a7b8c9, 0xd0e1, 0x2345, {0xf1, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45}};
    // {A7B8C9D0-E1F2-3456-1234-567890123456}
    static const GUID cfg_guid_pseudo_caption = {0xa7b8c9d0, 0xe1f2, 0x3456, {0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56}};
    // {B8C9D0E1-F2A3-4567-2345-678901234567}
    static const GUID cfg_guid_saved_window_state = {0xb8c9d0e1, 0xf2a3, 0x4567, {0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67}};
    // {C9D0E1F2-A3B4-5678-3456-789012345678}
    static const GUID cfg_guid_auto_load_fonts = {0xc9d0e1f2, 0xa3b4, 0x5678, {0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78}};

    cfg_bool ShowMainMenu(cfg_guid_show_main_menu, true);
    cfg_bool ShowStatusBar(cfg_guid_show_status_bar, true);
    cfg_int MainWindowFrameStyle(cfg_guid_main_window_frame_style, 0);
    cfg_bool EnableWin10Shadow(cfg_guid_enable_win10_shadow, true);
    cfg_bool DisableResizeWhenMaximized(cfg_guid_disable_resize_maximized, true);
    cfg_bool DisableResizeWhenFullscreen(cfg_guid_disable_resize_fullscreen, true);
    cfg_bool AutoLoadFonts(cfg_guid_auto_load_fonts, true);
    cfg_struct_t<PseudoCaptionParam> PseudoCaptionSettings(cfg_guid_pseudo_caption);
    cfg_struct_t<WindowStateData> SavedWindowState(cfg_guid_saved_window_state);

    // runtime vars
    uint32_t DPI = USER_DEFAULT_SCREEN_DPI;

    static void LoadFontsFromDirectory(const std::wstring& fontDir)
    {
        WIN32_FIND_DATAW findData;
        std::wstring searchPath = fontDir + L"\\*.*";
        
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            console::printf("[OpenHacks] Failed to open fonts directory: %s", 
                pfc::stringcvt::string_utf8_from_wide(fontDir.c_str()).get_ptr());
            return;
        }

        int totalLoaded = 0;
        int fileCount = 0;
        int failedCount = 0;
        std::vector<std::wstring> newFonts;
        
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::wstring fileName = findData.cFileName;
                size_t dotPos = fileName.find_last_of(L'.');
                if (dotPos == std::wstring::npos || dotPos == fileName.size() - 1)
                    continue;
                    
                std::wstring ext = fileName.substr(dotPos + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                
                if (ext == L"ttf" || ext == L"ttc" || ext == L"otf")
                {
                    fileCount++;
                    std::wstring fullPath = fontDir + L"\\" + fileName;
                    
                    console::printf("[OpenHacks] Loading font [%d]: %s", 
                        fileCount, pfc::stringcvt::string_utf8_from_wide(fileName.c_str()).get_ptr());
                    
                    int result = AddFontResourceW(fullPath.c_str());
                    
                    if (result > 0)
                    {
                        newFonts.push_back(fullPath);
                        totalLoaded += result;
                        console::printf("[OpenHacks] ✓ System registered: %s (%d face(s))", 
                            pfc::stringcvt::string_utf8_from_wide(fileName.c_str()).get_ptr(), result);
                    }
                    else
                    {
                        failedCount++;
                        DWORD err = GetLastError();
                        console::printf("[OpenHacks] ✗ System register failed: %s (Error: %lu)", 
                            pfc::stringcvt::string_utf8_from_wide(fileName.c_str()).get_ptr(), err);
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        
        console::printf("[OpenHacks] Scan complete: %d file(s) found, %d loaded, %d failed", 
            fileCount, (int)newFonts.size(), failedCount);
        
        if (!newFonts.empty())
        {
            g_loadedFonts.insert(g_loadedFonts.end(), newFonts.begin(), newFonts.end());
            
            console::printf("[OpenHacks] Broadcasting WM_FONTCHANGE...");
            SendMessageTimeoutW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0, 
                SMTO_ABORTIFHUNG, 5000, nullptr);
            
            console::printf("[OpenHacks] Initializing global GDI+ font collection...");
            
            if (!g_pGlobalFontCollection)
            {
                g_pGlobalFontCollection = new Gdiplus::PrivateFontCollection();
            }
            
            int gdiPlusSuccess = 0;
            int gdiPlusFailed = 0;
            
            for (const auto& fontPath : newFonts)
            {
                Gdiplus::Status addStatus = g_pGlobalFontCollection->AddFontFile(fontPath.c_str());
                
                if (addStatus == Gdiplus::Ok)
                {
                    gdiPlusSuccess++;
                    console::printf("[OpenHacks] ✓ GDI+ loaded: %s", 
                        pfc::stringcvt::string_utf8_from_wide(fontPath.c_str()).get_ptr());
                    
                    int familyCount = g_pGlobalFontCollection->GetFamilyCount();
                    if (familyCount > 0)
                    {
                        Gdiplus::FontFamily* families = new Gdiplus::FontFamily[familyCount];
                        int found = 0;
                        g_pGlobalFontCollection->GetFamilies(familyCount, families, &found);
                        
                        for (int i = 0; i < found; i++)
                        {
                            WCHAR familyName[LF_FACESIZE];
                            families[i].GetFamilyName(familyName);
                            console::printf("[OpenHacks]   └─ Family name: %s", 
                                pfc::stringcvt::string_utf8_from_wide(familyName).get_ptr());
                        }
                        
                        delete[] families;
                    }
                }
                else
                {
                    gdiPlusFailed++;
                    console::printf("[OpenHacks] ✗ GDI+ failed: %s (Status: %d)", 
                        pfc::stringcvt::string_utf8_from_wide(fontPath.c_str()).get_ptr(), addStatus);
                }
            }
            
            console::printf("[OpenHacks] GDI+ summary: %d success, %d failed", 
                gdiPlusSuccess, gdiPlusFailed);
            console::printf("[OpenHacks] === Font loading completed: %d file(s), %d face(s) ===", 
                (int)newFonts.size(), totalLoaded);
            console::printf("[OpenHacks] NOTE: Global font collection kept alive for persistent access");
        }
        else if (fileCount > 0)
        {
            console::printf("[OpenHacks] WARNING: No fonts were successfully loaded!");
        }
        else
        {
            console::printf("[OpenHacks] No font files found in directory");
        }
    }

    void LoadCustomFonts()
    {
        if (!AutoLoadFonts)  return;
        if (g_fb2k_profile.empty()) return;
        
        std::wstring fontsDir = pfc::stringcvt::string_wide_from_utf8(g_fb2k_profile.c_str());
        fontsDir += L"\\fonts";
        
        DWORD attrs = GetFileAttributesW(fontsDir.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (!CreateDirectoryW(fontsDir.c_str(), nullptr))
            {
                return;
            }
        }
        
        LoadFontsFromDirectory(fontsDir);
    }

    void InitialseOpenHacksVars()
    {
        const char* dllPath = core_api::get_my_full_path();
        bool isPortable = core_api::is_portable_mode_enabled();

        if (dllPath) {
            std::string path(dllPath);
            if (isPortable) {
                for (int i = 0; i < 4; i++) {
                    size_t pos = path.find_last_of('\\');
                    if (pos != std::string::npos) path = path.substr(0, pos);
                }
                g_fb2k_root = path;
            } else {
                char exePath[MAX_PATH] = {0};
                if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
                    std::string exeStr(exePath);
                    size_t pos = exeStr.find_last_of('\\');
                    if (pos != std::string::npos) g_fb2k_root = exeStr.substr(0, pos);
                }
            }
        }

        const char* profilePath = core_api::get_profile_path();
        if (profilePath) {
            g_fb2k_profile = profilePath;
            if (g_fb2k_profile.length() >= 7 && g_fb2k_profile.substr(0, 7) == "file://") {
                g_fb2k_profile = g_fb2k_profile.substr(7);
            }
        }

        if (!g_fb2k_root.empty()) {
            SetEnvironmentVariableA("fb2k", g_fb2k_root.c_str());
            SetEnvironmentVariableA("foobar2000", g_fb2k_root.c_str());
        }

        if (!g_fb2k_profile.empty()) {
            SetEnvironmentVariableA("fb2k_profile", g_fb2k_profile.c_str());
        }
        
        auto& pseudoCaption = PseudoCaptionSettings.get_value();
        if (pseudoCaption.height == 0)
        {
            auto height = Utility::GetSystemMetricsForDpi(SM_CYCAPTION, Utility::GetDPI(HWND_DESKTOP));
            height += Utility::GetSystemMetricsForDpi(SM_CYFRAME, Utility::GetDPI(HWND_DESKTOP));
            height += Utility::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, Utility::GetDPI(HWND_DESKTOP));
            pseudoCaption.height = height;
        }
    }

    void UnloadCustomFonts()
    {
        console::printf("[OpenHacks] ========================================");
        console::printf("[OpenHacks] Unloading custom fonts...");
        console::printf("[OpenHacks] ========================================");
        
        if (g_loadedFonts.empty())
        {
            console::printf("[OpenHacks] No custom fonts to unload");
            return;
        }
            
        console::printf("[OpenHacks] Unloading %d font file(s)...", (int)g_loadedFonts.size());
        
        int removedCount = 0;
        int failedCount = 0;
        
        for (const auto& fontPath : g_loadedFonts)
        {
            if (RemoveFontResourceW(fontPath.c_str()))
            {
                removedCount++;
                console::printf("[OpenHacks] ✓ Unloaded: %s", 
                    pfc::stringcvt::string_utf8_from_wide(fontPath.c_str()).get_ptr());
            }
            else
            {
                failedCount++;
                DWORD err = GetLastError();
                console::printf("[OpenHacks] ✗ Failed to unload: %s (Error: %lu)", 
                    pfc::stringcvt::string_utf8_from_wide(fontPath.c_str()).get_ptr(), err);
            }
        }
        
        if (g_pGlobalFontCollection)
        {
            delete g_pGlobalFontCollection;
            g_pGlobalFontCollection = nullptr;
            console::printf("[OpenHacks] Global GDI+ font collection destroyed");
        }
        
        g_loadedFonts.clear();
        
        if (removedCount > 0)
        {
            console::printf("[OpenHacks] Broadcasting WM_FONTCHANGE...");
            SendMessageTimeoutW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0, 
                SMTO_ABORTIFHUNG, 5000, nullptr);
            
            console::printf("[OpenHacks] === Successfully unloaded %d font file(s) ===", removedCount);
        }
        
        if (failedCount > 0)
        {
            console::printf("[OpenHacks] WARNING: Failed to unload %d font file(s)", failedCount);
        }
    }

} // namespace OpenHacksVars

// --- Custom Path Field Provider Implementation ---
static const struct {
    const char* name;
} kDisplayFields[] = {
    {"fb2k"},
    {"foobar2000"},
    {"fb2k_profile"}
};

uint32_t custom_path_field_provider::get_field_count() {
    return _countof(kDisplayFields);
}

void custom_path_field_provider::get_field_name(uint32_t index, pfc::string_base& out) {
    if (index < get_field_count()) {
        out.set_string(kDisplayFields[index].name);
    } else {
        out.reset();
    }
}

bool custom_path_field_provider::process_field(uint32_t index, metadb_handle* handle, titleformat_text_out* out) {
    if (index >= get_field_count()) return false;

    const char* path_str = nullptr;
    
    switch (index) {
    case 0: // fb2k
    case 1: // foobar2000 (Same as fb2k)
        if (!OpenHacksVars::g_fb2k_root.empty()) {
            path_str = OpenHacksVars::g_fb2k_root.c_str();
        }
        break;
    case 2: // fb2k_profile
        if (!OpenHacksVars::g_fb2k_profile.empty()) {
            path_str = OpenHacksVars::g_fb2k_profile.c_str();
        }
        break;
    }

    if (path_str) {
        out->write(titleformat_inputtypes::unknown, path_str, strlen(path_str));
        return true;
    }
    
    return false;
}

static service_factory_single_t<custom_path_field_provider> g_custom_path_field_provider;
