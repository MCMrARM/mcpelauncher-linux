#include "openssl_multithread.h"

#include <openssl/crypto.h>
#include <thread>

std::vector<std::mutex> OpenSSLMultithreadHelper::mutexes;

void OpenSSLMultithreadHelper::init() {
    mutexes = std::vector<std::mutex>(CRYPTO_num_locks());
    CRYPTO_set_id_callback([]() {
        return std::this_thread::get_id();
    });
    CRYPTO_set_locking_callback([](int mode, int n, const char*, int) {
        if (mode & CRYPTO_LOCK)
            mutexes[n].lock();
        else
            mutexes[n].unlock();
    });
}