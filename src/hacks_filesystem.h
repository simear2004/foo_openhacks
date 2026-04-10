#pragma once
#include "pch.h"
#include <algorithm>
#include <string>

class filesystem_foobar2000 : public filesystem_v3
{
public:
    filesystem_foobar2000() {}

    // 核心协议处理
    bool is_our_path(const char* p_path) override;
    bool get_canonical_path(const char* p_path, pfc::string_base& p_out) override;
    bool get_display_path(const char* p_path, pfc::string_base& p_out) override;

    // 文件操作 (来自 filesystem 基类)
    void open(service_ptr_t<file>& p_out, const char* p_path, t_open_mode p_mode, abort_callback& p_abort) override;
    void remove(const char* p_path, abort_callback& p_abort) override;
    void move(const char* p_src, const char* p_dst, abort_callback& p_abort) override;
    
    // 状态与属性 (来自 filesystem_v2)
    bool is_remote(const char* p_src) override;
    
    // filesystem_v3 要求的纯虚函数
    t_filestats2 get_stats2(const char* p_path, uint32_t s2flags, abort_callback& p_abort) override;
    void list_directory_v3(const char* path, directory_callback_v3& callback, unsigned listMode, abort_callback& p_abort) override;
    
    // 目录操作 (来自 filesystem_v2)
    void make_directory(const char* path, abort_callback& abort, bool* didCreate = nullptr) override;
    
    // 其他接口要求的方法
    bool supports_content_types() override;
    char pathSeparator() override;

private:
    std::string ExpandFoobar2000Path(const std::string& fb2kPath) const;
    std::string GetFoobar2000Root() const;
    std::string GetProfilePath() const;
    bool StartsWith(const std::string& str, const std::string& prefix) const;
};

static service_factory_single_t<filesystem_foobar2000> g_filesystem_foobar2000_factory;