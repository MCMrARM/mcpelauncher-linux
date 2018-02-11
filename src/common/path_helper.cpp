#include "path_helper.h"
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdexcept>
#include <cstring>
#include <climits>
#ifdef __APPLE__
#include <sys/param.h>
#include <mach-o/dyld.h>
#endif

std::string const PathHelper::appDirName = "mcpelauncher";
PathHelper::PathInfo PathHelper::pathInfo;

PathHelper::PathInfo::PathInfo() {
    appDir = findAppDir();
    homeDir = findUserHome();
    if (fileExists(getWorkingDir() + "libs/libminecraftpe.so")) {
        overrideDataDir = getWorkingDir();
        return;
    }
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
#ifdef __APPLE__
    char buf[MAXPATHLEN];
    char tbuf[MAXPATHLEN];
    uint32_t size = sizeof(tbuf) - 1;
    if (_NSGetExecutablePath(tbuf, &size) || size <= 0)
        return std::string();
    if (!realpath(tbuf, buf))
        return std::string();
    size = strlen(buf);
#else
    char buf[PATH_MAX];
    ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (size <= 0)
        return std::string();
#endif
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

std::string PathHelper::getWorkingDir() {
    char _cwd[256];
    getcwd(_cwd, 256);
    return std::string(_cwd) + "/";
}

std::string PathHelper::getParentDir(std::string const& path) {
    auto i = path.rfind('/', path.length() - 1); // we don't want to get the last / if the string ends up with a slash
    if (i == std::string::npos)
        return std::string();
    return path.substr(0, i);
}

bool PathHelper::fileExists(std::string const& path) {
    struct stat sb;
    return !stat(path.c_str(), &sb);
}

std::string PathHelper::findDataFile(std::string const& path) {
    if (!pathInfo.overrideDataDir.empty()) {
        std::string p = pathInfo.overrideDataDir + path;
        if (fileExists(p))
            return p;
        throw std::runtime_error("Failed to find data file: " + path);
    }
    std::string p = pathInfo.appDir + "/" + path;
    if (fileExists(p))
        return p;
    p = pathInfo.dataHome + "/" + appDirName + "/" + path;
    if (fileExists(p))
        return p;
    for (const auto& dir : pathInfo.dataDirs) {
        p = dir + "/" + appDirName + "/" + path;
        if (fileExists(p))
            return p;
    }
    p = getParentDir(pathInfo.appDir) + "/" + path;
    if (fileExists(p))
        return p;
    throw std::runtime_error("Failed to find data file: " + path);
}