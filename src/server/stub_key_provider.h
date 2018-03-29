#pragma once

#include <unordered_map>
#include "../minecraft/string.h"
#include "../minecraft/UUID.h"
#include "../minecraft/ResourcePack.h"

class StubKeyProvider : public IContentKeyProvider {

public:
    virtual ~StubKeyProvider() { }
    virtual mcpe::string getContentKey(mce::UUID const&) {
        return mcpe::string();
    }
    virtual void setTempContentKeys(std::unordered_map<std::string, std::string> const&) { }
    virtual void clearTempContentKeys() { }

};