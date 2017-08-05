#pragma once

#include <string>

class Base64 {

private:

    static char table[65];
    static unsigned char reverseTable[256];
    static bool reverseTableInitialized;

    static void initReverseTable();

public:

    static std::string encode(const std::string& input);

    static std::string decode(const std::string& input, const char* skipChars = "\r\n");

};