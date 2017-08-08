#pragma once

#include <string>
#include <zip.h>

class ExtractHelper {

private:
    static void extractFile(zip* za, zip_uint64_t fileId, const char* path, char* buf, size_t bufSize);

public:
    static void extractApk(std::string const& apk);

};