#pragma once

#include <functional>

class SaveTransactionManager {

public:
    char filler[0x10];

    /// @symbol _ZN22SaveTransactionManagerC2ESt8functionIFvbEE
    SaveTransactionManager(std::function<void (bool)>);

};