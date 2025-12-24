#pragma once

#include <ogcsys.h>
#include <string>
#include <array>
#include <vector>

struct File {
    void* data;
    size_t size;
    std::string error;
    s32 error_code;
};

constexpr s32 ISFS_ENOENT = -106;

File* ISFS_GetFile(std::string_view path);