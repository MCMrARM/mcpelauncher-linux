#pragma once

namespace minecraft {
namespace api {

class Api {

public:

    void** vtable;
    mcpe::string envPath;
    void** playerIfaceVtable;
    void** entityIfaceVtable;
    void** networkIfaceVtable;
    void** playerInteractionsIfaceVtable;

};

}
}