#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <gccore.h>

static fstats stats ATTRIBUTE_ALIGN(32);

File* ISFS_GetFile(std::string_view path) {
  u32 size{};
  File* file = new File();

  s32 fd = ISFS_Open(path.data(), ISFS_OPEN_READ);
  if (fd < 0) {
    file->error = std::format("ISFS_GetFile: unable to open file {} (error {})\n", path, fd);
    file->error_code = fd;
    return file;
  }

  void *buf = nullptr;
  memset(&stats, 0, sizeof(fstats));

  s32 ret = ISFS_GetFileStats(fd, &stats);
  if (ret >= 0) {
    s32 length = stats.file_length;

    // We must align our length by 32.
    // memalign itself is dreadfully broken for unknown reasons.
    s32 aligned_length = length;
    s32 remainder = aligned_length % 32;
    if (remainder != 0) {
      aligned_length += 32 - remainder;
    }

    buf = aligned_alloc(32, aligned_length);

    if (buf != nullptr) {
      s32 tmp_size = ISFS_Read(fd, buf, length);
      if (tmp_size == length) {
        // We were successful.
        size = tmp_size;
      } else {
        // If positive, the file could not be fully read.
        // If negative, it is most likely an underlying /dev/fs
        // error.
        if (tmp_size >= 0) {
          file->error = std::format("ISFS_GetFile: only able to read {} out of {} bytes!", tmp_size, length);
          file->error_code = tmp_size;
        } else if (tmp_size == ISFS_ENOENT) {
          file->error = std::format("ISFS_GetFile: file not found (error {})", tmp_size);
          file->error_code = tmp_size;
        } else {
          file->error = std::format("ISFS_GetFile: ISFS_Open failed! (error {})", tmp_size);
          file->error_code = tmp_size;
        }

        free(buf);
      }
    } else {
      file->error = "ISFS_GetFile: failed to allocate buffer!";
      file->error_code = -1;
    }
  } else {
    file->error = std::format("ISFS_GetFile: unable to retrieve file stats (error {})", ret);
    file->error_code = ret;
  }
  ISFS_Close(fd);
  file->data = buf;
  file->size = size;

  return file;
}
