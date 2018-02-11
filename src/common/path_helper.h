#pragma once

#include <string>
#include <vector>

class PathHelper {

private:
    static std::string const appDirName;

    struct PathInfo {
        std::string appDir;
        std::string homeDir;
        std::string dataHome;
        std::vector<std::string> dataDirs;
        std::string cacheHome;
        std::string overrideDataDir;

        PathInfo();
    };

    static PathInfo pathInfo;

    static std::string findUserHome();

    static std::string findAppDir();

public:

    static bool fileExists(std::string const& path);

    static std::string getParentDir(std::string const& path);

    static std::string getWorkingDir();

    static std::string findDataFile(std::string const& path);

    static std::string getPrimaryDataDirectory() {
        if (!pathInfo.overrideDataDir.empty())
            return pathInfo.overrideDataDir;
        return pathInfo.dataHome + "/" + appDirName + "/";
    }

    static std::string getCacheDirectory() {
        if (!pathInfo.overrideDataDir.empty())
            return pathInfo.overrideDataDir;
        return pathInfo.cacheHome + "/" + appDirName + "/";
    }

    static std::string getIconPath() {
        return findDataFile("mcpelauncher-icon.png");
    }

};
