#pragma once

#include <memory>

namespace Social {
class UserManager {

public:

    static std::unique_ptr<Social::UserManager> CreateUserManager();


};
}