#pragma once

#include <iostream>
#include "minecraft/string.h"
#include "log.h"

struct StoreListener;
struct PurchaseInfo;
struct LinuxStore {
    virtual ~LinuxStore() {
        Log::trace("Store", "Destroying LinuxStore");
    }
    virtual bool isReadyToMakePurchases() {
        Log::trace("Store", "isReadyToMakePurchases: false");
        return true;
    }
    virtual bool requiresRestorePurchasesButton() {
        Log::trace("Store", "requiresRestorePurchasesButton: false");
        return false;
    }
    virtual bool allowsSubscriptions() {
        Log::trace("Store", "allowsSubscriptions: true");
        return true;
    }
    virtual mcpe::string getStoreId() {
        Log::trace("Store", "getStoreId: android.googleplay");
        return "android.googleplay";
    }
    virtual mcpe::string getSubPlatformStoreId() {
        Log::trace("Store", "getSubPlatformStoreId: ");
        return "";
    }
    virtual mcpe::string getProductSkuPrefix() {
        Log::trace("Store", "getProductSkuPrefix: ");
        return "";
    }
    virtual mcpe::string getRealmsSkuPrefix() {
        Log::trace("Store", "getRealmsSkuPrefix: ");
        return "";
    }
    virtual void queryProducts(std::vector<std::string> const& arr) {
        Log::trace("Store", "queryProducts");
    }
    virtual void purchase(std::string const& name) {
        Log::trace("Store", "purchase: %s", name.c_str());
    }
    virtual void acknowledgePurchase(PurchaseInfo const& info, int type) {
        Log::trace("Store", "acknowledgePurchase: type=%i", type);
    }
    virtual void queryPurchases() {
        Log::trace("Store", "queryPurchases");
    }
    virtual void restorePurchases() {
        Log::trace("Store", "restorePurchases");
    }
    virtual bool isTrial() {
        // Log::trace("Store", "isTrial: false");
        return false;
    }
    virtual void purchaseGame() {
        Log::trace("Store", "purchaseGame");
    }
    virtual bool isGameLicensed() {
        Log::trace("Store", "isGameLicensed: true");
        return true;
    }
    virtual mcpe::string getAppReceipt() {
        Log::trace("Store", "getAppReceipt");
        return mcpe::string();
    }
    virtual void registerLicenseChangeCallback() {
        Log::trace("Store", "registerLicenseChangeCallback");
    }
    virtual void handleLicenseChange() {
        Log::trace("Store", "handleLicenseChange");
    }
    virtual void restoreFromCache() {
        Log::trace("Store", "restoreFromCache");
    }
    virtual void getUserAccessTokenAsync() {
        Log::trace("Store", "getUserAccessTokenAsync");
    }
    virtual void getFullSKUWithMetadataFromProductSku() {
        Log::trace("Store", "getFullSKUWithMetadataFromProductSku");
    }
    virtual std::string getFullGameProductSku() {
        Log::trace("Store", "getFullGameProductSku");
        return "idk";
    }
    virtual std::string getLanguageCode() {
        Log::trace("Store", "getLanguageCode");
        return "idk";
    }
    virtual std::string getRegionCode() {
        Log::trace("Store", "getRegionCode");
        return "idk";
    }
    virtual void refreshLicenses() {
        Log::trace("Store", "refreshLicenses");
    }
    virtual void updateXUID() {
        Log::trace("Store", "updateXUID");
    }
    virtual void onNewPrimaryUser() {
        Log::trace("Store", "onNewPrimaryUser");
    }
};