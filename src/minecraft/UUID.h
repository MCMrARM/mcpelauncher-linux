#pragma once

namespace mce {

class UUID {

public:

    static UUID* EMPTY;

    static mce::UUID (*fromString)(mcpe::string const&);

    char filler[0x14];

};

}