#include "path_helper.h"
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdexcept>
#include <cstring>

std::string const PathHelper::appDirName = "mcpelauncher";
PathHelper::PathInfo PathHelper::pathInfo;

PathHelper::PathInfo::PathInfo() {
    appDir = findAppDir();
    homeDir = findUserHome();
    char* env = getenv("XDG_DATA_HOME");
    if (env != nullptr)
        dataHome = std::string(env);
    if (dataHome.empty())
        dataHome = homeDir + "/.local/share";
    env = getenv("XDG_DATA_DIRS");
    if (env != nullptr) {
        char* s = env;
        while (true) {
            char* r = strchr(s, ':');
            if (r == nullptr){
                if (strlen(s) > 0)
                    dataDirs.push_back(std::string(s));
                break;
            }
            dataDirs.push_back(std::string(s, r - s));
            s = r + 1;
        }
    }
    if (dataDirs.empty())
        dataDirs = {"/usr/local/share/", "/usr/share/"};
    env = getenv("XDG_CACHE_HOME");
    if (env != nullptr)
        cacheHome = std::string(env);
    if (cacheHome.empty())
        cacheHome = homeDir + "/.cache";
}

std::string PathHelper::findAppDir() {
    char buf[1024];
    ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (size <= 0)
        return std::string();
    buf[size] = '\0';
    char* dirs = strrchr(buf, '/');
    if (dirs != nullptr)
        dirs[0] = '\0';
    return std::string(buf);
}

std::string PathHelper::findUserHome() {
    char* env = getenv("HOME");
    if (env != nullptr)
        return env;

    struct passwd pwd;
    int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;
    char* buf = new char[bufsize];
    struct passwd *result;
    getpwuid_r(getuid(), &pwd, buf, (size_t) bufsize, &result);
    if (result == NULL)
        throw std::runtime_error("getpwuid failed");
    std::string ret(result->pw_dir);
    delete[] buf;
    return ret;
}

bool PathHelper::fileExists(std::string const& path) {
    struct stat sb;
    return !stat(path.c_str(), &sb);
}

std::string PathHelper::findDataFile(std::string const& path) {
    std::string p = pathInfo.dataHome + "/" + appDirName + "/" + path;
    if (fileExists(p))
        return p;
    for (const auto& dir : pathInfo.dataDirs) {
        p = dir + "/" + appDirName + "/" + path;
        if (fileExists(p))
            return p;
    }
    p = pathInfo.appDir + "/" + path;
    if (fileExists(p))
        return p;
    throw std::runtime_error("Failed to find data file: " + path);
}