#include "pch.h"
#include "hacks_filesystem.h"

filesystem_v3::ptr filesystem_foobar2000::getLocalFSv3()
{
    filesystem::ptr localFS = filesystem::getLocalFS();
    filesystem_v3::ptr v3fs;
    if (localFS->service_query_t(v3fs)) {
        return v3fs;
    }
    throw pfc::exception_not_implemented();
}

bool filesystem_foobar2000::is_our_path(const char* p_path)
{
    return foobar2000_io::matchProtocol(p_path, "fb2k");
}

bool filesystem_foobar2000::get_canonical_path(const char* p_path, pfc::string_base& p_out)
{
    if (!is_our_path(p_path)) return false;
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) return false;
    p_out = expandedPath.c_str();
    return true;
}

bool filesystem_foobar2000::get_display_path(const char* p_path, pfc::string_base& p_out)
{
    return get_canonical_path(p_path, p_out);
}

void filesystem_foobar2000::open(service_ptr_t<file>& p_out, const char* p_path, t_open_mode p_mode, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->open(p_out, expandedPath.c_str(), p_mode, p_abort);
}

void filesystem_foobar2000::remove(const char* p_path, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->remove(expandedPath.c_str(), p_abort);
}

void filesystem_foobar2000::move(const char* p_src, const char* p_dst, abort_callback& p_abort)
{
    std::string expandedSrc = ExpandFoobar2000Path(p_src);
    std::string expandedDst = ExpandFoobar2000Path(p_dst);
    if (expandedSrc.empty() || expandedDst.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->move(expandedSrc.c_str(), expandedDst.c_str(), p_abort);
}

bool filesystem_foobar2000::is_remote(const char* p_src) { return false; }

void filesystem_foobar2000::get_stats(const char* p_path, t_filestats& p_stats, bool& p_is_writeable, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->get_stats(expandedPath.c_str(), p_stats, p_is_writeable, p_abort);
}

void filesystem_foobar2000::create_directory(const char* p_path, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->create_directory(expandedPath.c_str(), p_abort);
}

void filesystem_foobar2000::list_directory(const char* p_path, directory_callback& p_out, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    filesystem::getLocalFS()->list_directory(expandedPath.c_str(), p_out, p_abort);
}

bool filesystem_foobar2000::supports_content_types() { return true; }

void filesystem_foobar2000::move_overwrite(const char* src, const char* dst, abort_callback& abort)
{
    std::string expandedSrc = ExpandFoobar2000Path(src);
    std::string expandedDst = ExpandFoobar2000Path(dst);
    if (expandedSrc.empty() || expandedDst.empty()) throw pfc::exception_invalid_params();
    getLocalFSv3()->move_overwrite(expandedSrc.c_str(), expandedDst.c_str(), abort);
}

void filesystem_foobar2000::make_directory(const char* path, abort_callback& abort, bool* didCreate)
{
    std::string expandedPath = ExpandFoobar2000Path(path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    getLocalFSv3()->make_directory(expandedPath.c_str(), abort, didCreate);
}

bool filesystem_foobar2000::directory_exists(const char* path, abort_callback& abort)
{
    std::string expandedPath = ExpandFoobar2000Path(path);
    if (expandedPath.empty()) return false;
    return getLocalFSv3()->directory_exists(expandedPath.c_str(), abort);
}

bool filesystem_foobar2000::file_exists(const char* path, abort_callback& abort)
{
    std::string expandedPath = ExpandFoobar2000Path(path);
    if (expandedPath.empty()) return false;
    return getLocalFSv3()->file_exists(expandedPath.c_str(), abort);
}

char filesystem_foobar2000::pathSeparator() { return '/'; }

void filesystem_foobar2000::list_directory_ex(const char* p_path, directory_callback& p_out, unsigned listMode, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    getLocalFSv3()->list_directory_ex(expandedPath.c_str(), p_out, listMode, p_abort);
}

t_filestats2 filesystem_foobar2000::get_stats2(const char* p_path, uint32_t s2flags, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(p_path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    return getLocalFSv3()->get_stats2(expandedPath.c_str(), s2flags, p_abort);
}

void filesystem_foobar2000::list_directory_v3(const char* path, directory_callback_v3& callback, unsigned listMode, abort_callback& p_abort)
{
    std::string expandedPath = ExpandFoobar2000Path(path);
    if (expandedPath.empty()) throw pfc::exception_invalid_params();
    getLocalFSv3()->list_directory_v3(expandedPath.c_str(), callback, listMode, p_abort);
}

std::string filesystem_foobar2000::ExpandFoobar2000Path(const std::string& fb2kPath) const
{
    const std::string prefix = "fb2k://";
    if (fb2kPath.find(prefix) != 0) return "";
    
    std::string relativePath = fb2kPath.substr(prefix.length());
    if (relativePath.empty()) return "";

    std::string result;
    if (StartsWith(relativePath, "root/") || StartsWith(relativePath, "root\\"))
    {
        std::string rootPath = GetFoobar2000Root();
        if (rootPath.empty()) return "";
        result = rootPath + "\\" + relativePath.substr(5);
    }
    else if (StartsWith(relativePath, "profile/") || StartsWith(relativePath, "profile\\"))
    {
        std::string profilePath = GetProfilePath();
        if (profilePath.empty()) return "";
        result = profilePath + "\\" + relativePath.substr(8);
    }
    else { return ""; }

    std::replace(result.begin(), result.end(), '/', '\\');
    return "file://" + result;
}

std::string filesystem_foobar2000::GetFoobar2000Root() const
{
    const char* dllPath = core_api::get_my_full_path();
    if (!dllPath || dllPath[0] == '\0') return "";
    std::string path(dllPath);
    size_t pos = path.find_last_of('\\');
    return (pos != std::string::npos) ? path.substr(0, pos) : "";
}

std::string filesystem_foobar2000::GetProfilePath() const
{
    const char* profilePath = core_api::get_profile_path();
    if (!profilePath || profilePath[0] == '\0') return "";
    std::string path(profilePath);
    if (path.find("file://") == 0) path = path.substr(7);
    return path;
}

bool filesystem_foobar2000::StartsWith(const std::string& str, const std::string& prefix) const
{
    if (str.length() < prefix.length()) return false;
    return str.compare(0, prefix.length(), prefix) == 0;
}
