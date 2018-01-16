#pragma once

struct OpsList {

    struct Entry {
        char filler[0x10];
    };

    bool b;
    std::vector<Entry> entries;

    OpsList(bool);

};