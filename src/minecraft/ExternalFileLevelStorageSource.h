#pragma once

#include <memory>
#include "SaveTransactionManager.h"
class FilePathManager;
class Scheduler;

class LevelStorage {
public:
    virtual ~LevelStorage() = 0;
};

struct ExternalFileLevelStorageSource {

public:
    char filler[0x10];

    ExternalFileLevelStorageSource(FilePathManager*, std::shared_ptr<SaveTransactionManager>);

    /// @symbol _ZN30ExternalFileLevelStorageSource18createLevelStorageER9SchedulerRKSsS3_RK19IContentKeyProvider
    std::unique_ptr<LevelStorage> createLevelStorage(Scheduler&, mcpe::string const&, mcpe::string const&, IContentKeyProvider const&);

};