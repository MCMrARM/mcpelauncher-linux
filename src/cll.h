#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <random>
#include <map>

class MSAAccount;

class CLL {

private:

    static std::string const VORTEX_URL;
    static int MAXEVENTSIZEINBYTES;
    static int MAXEVENTSPERPOST;

    struct Event {
        std::string ticket;
        std::string name;
        std::string data;
        std::chrono::system_clock::time_point time;
    };

    std::vector<Event> events;
    std::mutex eventsMutex;
    std::thread thread;
    std::condition_variable threadLock;
    std::mutex threadMutex;
    bool shouldStopThread = false;
    std::mt19937 random;
    long long seqNum = 0LL;
    std::mutex accountMutex;
    std::string authXToken;
    std::string authXTicket;
    std::string msaDeviceTicket;
    std::shared_ptr<MSAAccount> account;

    void runThread();

    void uploadEvents();

    void sendEvent(std::string const& data, bool compress = false);

    static std::string compress(std::string const& data);

    std::string getMSADeviceTicket();

    std::pair<std::string, std::string> getXTokenAndTicket();

public:

    CLL();

    ~CLL();

    void setMSAAccount(std::shared_ptr<MSAAccount> account);

    void addEvent(std::string const& ticket, std::string const& name, std::string const& data);

};