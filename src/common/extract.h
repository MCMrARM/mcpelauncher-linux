#pragma once

#include <string>
#include <zip.h>
#include "path_helper.h"

class ExtractHelper {

private:
    static void extractFile(zip* za, zip_uint64_t fileId, std::string const& path, char* buf, size_t bufSize);

public:
    static void extractApk(std::string const& apk, std::string const& targetDir = PathHelper::getPrimaryDataDirectory());

};