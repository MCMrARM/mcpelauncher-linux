#include "file_picker_zenity.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstring>
#include "../../common/log.h"

const std::string ZenityFilePicker::EXECUTABLE_PATH = "/usr/bin/zenity";

std::vector<std::string> ZenityFilePicker::buildCommandLine() {
    std::vector<std::string> cmd;
    cmd.emplace_back(EXECUTABLE_PATH);
    cmd.emplace_back("--file-selection");
    if (!title.empty()) {
        cmd.emplace_back("--title");
        cmd.emplace_back(title);
    }
    if (mode == Mode::SAVE)
        cmd.emplace_back("--save");
    if (!patterns.empty()) {
        cmd.emplace_back("--file-filter");
        std::string val;
        for (std::string const& filter : patterns) {
            if (!val.empty())
                val += " ";
            val += filter;
        }
        cmd.emplace_back(val);
    }
    return std::move(cmd);
}

std::vector<const char*> ZenityFilePicker::convertToC(std::vector<std::string> const& v) {
    std::vector<const char*> ret;
    for (auto const& i : v)
        ret.push_back(i.c_str());
    ret.push_back(nullptr);
    return std::move(ret);
}

bool ZenityFilePicker::show() {
    struct stat sb;
    if (stat(EXECUTABLE_PATH.c_str(), &sb))
        throw std::runtime_error("Could not find zenity.\n\nTo be able to pick files, please install the `zenity` utility.");

    char ret[1024];

    int pipes[3][2];
    static const int PIPE_STDOUT = 0;
    static const int PIPE_STDERR = 1;
    static const int PIPE_STDIN = 2;
    static const int PIPE_READ = 0;
    static const int PIPE_WRITE = 1;

    pipe(pipes[PIPE_STDOUT]);
    pipe(pipes[PIPE_STDERR]);
    pipe(pipes[PIPE_STDIN]);

    int pid;
    if (!(pid = fork())) {
        auto argv = buildCommandLine();
        auto argvc = convertToC(argv);

        dup2(pipes[PIPE_STDOUT][PIPE_WRITE], STDOUT_FILENO);
        dup2(pipes[PIPE_STDERR][PIPE_WRITE], STDERR_FILENO);
        dup2(pipes[PIPE_STDIN][PIPE_READ], STDIN_FILENO);
        close(pipes[PIPE_STDIN][PIPE_WRITE]);
        close(pipes[PIPE_STDOUT][PIPE_WRITE]);
        close(pipes[PIPE_STDERR][PIPE_WRITE]);
        close(pipes[PIPE_STDIN][PIPE_READ]);
        close(pipes[PIPE_STDOUT][PIPE_READ]);
        close(pipes[PIPE_STDERR][PIPE_READ]);
        int r = execv(argvc[0], (char**) argvc.data());
        Log::error("ZenityFilePicker", "Show: execv() error %i %s", r, strerror(errno));
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(STDIN_FILENO);
        _exit(r);
    } else {
        close(pipes[PIPE_STDIN][PIPE_WRITE]);
        close(pipes[PIPE_STDIN][PIPE_READ]);

        close(pipes[PIPE_STDOUT][PIPE_WRITE]);
        close(pipes[PIPE_STDERR][PIPE_WRITE]);

        std::string outputStdOut;
        std::string outputStdErr;
        ssize_t r;
        if ((r = read(pipes[PIPE_STDOUT][PIPE_READ], ret, 1024)) > 0)
            outputStdOut += std::string(ret, (size_t) r);
        if ((r = read(pipes[PIPE_STDERR][PIPE_READ], ret, 1024)) > 0)
            outputStdErr += std::string(ret, (size_t) r);

        close(pipes[PIPE_STDOUT][PIPE_READ]);
        close(pipes[PIPE_STDERR][PIPE_READ]);

        int status;
        waitpid(pid, &status, 0);
        status = WEXITSTATUS(status);
        Log::trace("ZenityFilePicker", "Show: Status = %i", status);

        Log::trace("ZenityFilePicker", "Stdout = %s", outputStdOut.c_str());
        Log::trace("ZenityFilePicker", "Stderr = %s", outputStdErr.c_str());

        if (status == 0) {
            auto iof = outputStdOut.find('\n');
            if (iof != std::string::npos)
                outputStdOut = outputStdOut.substr(0, iof);
            pickedFile = outputStdOut;
            return true;
        }
    }
    return false;
}