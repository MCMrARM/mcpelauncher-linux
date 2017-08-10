#include "extract.h"
#include "path_helper.h"
#include <zip.h>
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <ftw.h>

// https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
static int removeFile(const char* pathname, const struct stat* sbuf, int type, struct FTW* ftwb) {
    if (remove(pathname) < 0) {
        perror("ERROR: remove");
        return -1;
    }
    return 0;
}


static void _rmkdir(char* pathname) {
    struct stat sb;
    if (stat(pathname, &sb)) {
        char* i = strrchr(pathname, '/');
        if (i != nullptr) {
            i[0] = '\0';
            _rmkdir(pathname);
            i[0] = '/';
        }
        if (mkdir(pathname, 0755))
            throw std::runtime_error("mkdir failed");
    }
}

void ExtractHelper::extractFile(zip* za, zip_uint64_t fileId, std::string const& path, char* buf, size_t bufSize) {
    {
        std::string dir(path);
        dir = dir.substr(0, dir.find_last_of('/'));
        _rmkdir(&dir[0]);
    }

    zip_file* src = zip_fopen_index(za, fileId, 0);
    FILE* out = fopen(path.c_str(), "wb");
    if (!src || !out)
        throw std::runtime_error("Failed to open file");

    zip_int64_t r;
    while ((r = zip_fread(src, buf, bufSize)) > 0) {
        fwrite(buf, sizeof(char), (size_t) r, out);
    }
    fclose(out);
    zip_fclose(src);
}

void ExtractHelper::extractApk(std::string const& apk) {
    std::string basePath = PathHelper::getPrimaryDataDirectory();

    printf("Deleting assets/\n");
    nftw(std::string(basePath + "assets/").c_str(), removeFile, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);

    printf("Extracting apk: %s\n", apk.c_str());
    int err = 0;
    zip* za = zip_open(apk.c_str(), 0, &err);
    if (za == nullptr)
        throw std::runtime_error("Failed to open zip " + std::to_string(err));

    struct zip_stat zs;

    const size_t bufSize = 128 * 1024;
    char* buf = new char[bufSize];

    zip_int64_t n = zip_get_num_entries(za, 0);
    for (zip_int64_t i = 0; i < n; i++) {
        if (zip_stat_index(za, (zip_uint64_t) i, 0, &zs) != 0) {
            printf("zip_stat_index failed\n");
            continue;
        }
        int nameLen = strlen(zs.name);
        if (nameLen >= 7 && memcmp(zs.name, "assets/", 7) == 0) {
            printf("Extracting asset: %s\n", zs.name);
            if (zs.name[nameLen - 1] == '/')
                continue;
            extractFile(za, (zip_uint64_t) i, basePath + zs.name, buf, bufSize);
        } else if (strcmp(zs.name, "res/raw/xboxservices.config") == 0) {
            extractFile(za, (zip_uint64_t) i, basePath + "assets/xboxservices.config", buf, bufSize);
        } else if (strcmp(zs.name, "lib/x86/libminecraftpe.so") == 0) {
            extractFile(za, (zip_uint64_t) i, basePath + "libs/libminecraftpe.so", buf, bufSize);
        } else {
            printf("Skipping: %s\n", zs.name);
        }
    }

    zip_close(za);
    delete[] buf;
}