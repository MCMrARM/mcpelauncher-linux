#pragma once

struct Whitelist {

    struct Entry {
        char filler[0x20];
    };

    std::vector<Entry> entries;

};