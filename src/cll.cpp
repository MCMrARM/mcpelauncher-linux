#include "cll.h"
#include "base64.h"

#include <sstream>
#include <ctime>
#include <cstring>
#include <functional>
#include <curl/curl.h>
#include <zlib.h>

std::string const CLL::VORTEX_URL = "https://vortex.data.microsoft.com/collect/v1";
// TODO: Those are normally fetched from config
int CLL::MAXEVENTSIZEINBYTES = 64000;
int CLL::MAXEVENTSPERPOST = 500;

CLL::CLL() : random(time(0)) {
    thread = std::thread(std::bind(&CLL::runThread, this));
}

CLL::~CLL() {
    if (thread.joinable()) {
        std::unique_lock<std::mutex> lock(threadMutex);
        shouldStopThread = true;
        lock.unlock();
        threadLock.notify_all();

        thread.join();
    }
}

void CLL::addEvent(std::string const& ticket, std::string const& name, std::string const& data) {
    std::unique_lock<std::mutex> lock (eventsMutex);
    events.push_back({ticket, name, data, std::chrono::system_clock::now()});
}

void CLL::uploadEvents() {
    using namespace std::chrono;
    std::uniform_int_distribution<long long> ldist(0, std::numeric_limits<long long>::max());
    std::vector<Event> events;
    eventsMutex.lock();
    events = std::move(this->events);
    eventsMutex.unlock();
    char timestamp[32];
    std::stringstream batch;
    int batched = -1;
    for (auto it = events.begin(); it != events.end(); it++) {
        auto event = *it;
        time_t time = system_clock::to_time_t(event.time);
        strftime(timestamp, sizeof(timestamp), "%FT%T", gmtime(&time));
        auto timestampLen = strlen(timestamp);
        auto timeMs = duration_cast<milliseconds>(event.time.time_since_epoch()) - duration_cast<milliseconds>(duration_cast<seconds>(event.time.time_since_epoch()));
        snprintf(&timestamp[timestampLen], sizeof(timestamp) - timestampLen, ".%03dZ", (int) timeMs.count());

        long long epoch = ldist(random);

        seqNum++;

        std::string deviceLocalId = ""; // As far as I understand it should be alright to leave this empty
        std::stringstream ss;
        ss << "{"
           << "\"ver\":\"2.1\","
           << "\"name\":\"" << event.name << "\","
           << "\"time\":\"" << timestamp << "\","
           << "\"popSample\":100.0,"
           << "\"epoch\":\"" << epoch << "\","
           << "\"seqNum\":" << seqNum << ","
           << "\"iKey\":\"P-XBL-T1739947436\","
           << "\"flags\":514,"
           << "\"os\":\"Android\","
           << "\"osVer\":\"7.1.1\","
           << "\"appId\":\"A:com.mojang.minecraftpe\","
           << "\"appVer\":\"1.2.0.7\"," // TODO: This one should be somehow based off MCPE itself
           << "\"ext\":{"
           << "\"user\":{\"ver\":\"1.0\",\"localId\":\"\"},"
           << "\"os\":{\"ver\":\"1.0\",\"locale\":\"en-US\"},"
           << "\"device\":{\"ver\":\"1.0\",\"localId\":\"" << deviceLocalId << "\",\"deviceClass\":\"Android.PC\"},"
           << "\"android\":{\"ver\":\"1.0\",\"libVer\":\"3.160623.0\",\"tickets\":[\"" << event.ticket << "\"]}"
           << "},"
           << "\"data\":" << event.data
           << "}";
        if (batched != -1 && (batch.str().size() + ss.str().size() + 2 >= MAXEVENTSIZEINBYTES || batched >= MAXEVENTSPERPOST)) {
            sendEvent(batch.str(), true);
            batched = -1;
        }
        if (batched == -1 && (it + 1) == events.end()) {
            sendEvent(ss.str(), false);
        } else {
            batch << ss.str() << "\r\n";
            batched++;
        }
    }
    if (batched != -1)
        sendEvent(batch.str(), true);
}

static size_t curl_stringstream_write_func(void* ptr, size_t size, size_t nmemb, std::stringstream* s) {
    s->write((char*) ptr, size * nmemb);
    return size * nmemb;
}
void CLL::sendEvent(std::string const& data, bool compress) {
    using namespace std::chrono;
    char timestamp[64];
    strcpy(timestamp, "X-UploadTime: ");
    auto timestampLen = strlen(timestamp);
    auto time = system_clock::now();
    time_t timec = system_clock::to_time_t(time);
    strftime(&timestamp[timestampLen], sizeof(timestamp) - timestampLen, "%FT%T", gmtime(&timec));
    timestampLen = strlen(timestamp);
    auto timeMs = duration_cast<microseconds>(time.time_since_epoch()) - duration_cast<microseconds>(duration_cast<seconds>(time.time_since_epoch()));
    snprintf(&timestamp[timestampLen], sizeof(timestamp) - timestampLen, ".%06dZ", (int) timeMs.count());

    printf("Request: %s\n", data.c_str());

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, VORTEX_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    std::string compressed;
    if (compress) {
        compressed = CLL::compress(data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) compressed.length());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, compressed.c_str());
        curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip, deflate");
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    } else {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) data.length());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/x-json-stream; charset=utf-8");
    headers = curl_slist_append(headers, timestamp);
    if (compress) {
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
        headers = curl_slist_append(headers, "Content-Encoding: deflate");
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::stringstream output;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_stringstream_write_func);
    curl_easy_perform(curl);
    printf("Response: %i %s\n", (int)output.str().length(), output.str().c_str());
}

std::string CLL::compress(std::string const& data) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    int ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        throw std::runtime_error("Failed to call deflateInit");
    zs.avail_in = (uInt) data.length();
    zs.next_in = (unsigned char*) data.data();
    std::string out;
    while (true) {
        out.resize(out.size() + 4096);
        zs.next_out = (unsigned char*) &out[out.size() - 4096];
        zs.avail_out = 4096;
        ret = deflate(&zs, Z_FINISH);
        zs.avail_in = 0;
        if (zs.avail_out != 0)
            out.resize(out.size() - zs.avail_out);
        if (ret == Z_STREAM_END)
            break;
        if (ret != Z_OK)
            throw std::runtime_error("deflate error");
    }
    deflateEnd(&zs);
    return std::move(out);
}

void CLL::runThread() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(threadMutex);
            threadLock.wait_for(lock, std::chrono::milliseconds(10));
        }
        uploadEvents();
        {
            if (shouldStopThread)
                return;
        }
    }
}