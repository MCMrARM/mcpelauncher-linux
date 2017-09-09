#pragma once

#include <iostream>
#include "minecraft/string.h"

struct StoreListener;
struct PurchaseInfo;
struct LinuxStore {
    virtual ~LinuxStore() {
        std::cout << "destroying store\n";
    }
    virtual bool isReadyToMakePurchases() {
        std::cout << "is ready to make purchases: false\n";
        return true;
    }
    virtual bool requiresRestorePurchasesButton() {
        std::cout << "requires restore purchases button: false\n";
        return false;
    }
    virtual bool allowsSubscriptions() {
        std::cout << "allows subscriptions: true\n";
        return true;
    }
    virtual mcpe::string getStoreId() {
        std::cout << "get store id: android.googleplay\n";
        return "android.googleplay";
    }
    virtual mcpe::string getSubPlatformStoreId() {
        std::cout << "get sub platform store id: \n";
        return "";
    }
    virtual mcpe::string getProductSkuPrefix() {
        std::cout << "get product sku prefix: \n";
        return "";
    }
    virtual mcpe::string getRealmsSkuPrefix() {
        std::cout << "get product sku prefix: \n";
        return "";
    }
    virtual void queryProducts(std::vector<std::string> const& arr) {
        std::cout << "query products\n";
    }
    virtual void purchase(std::string const& name) {
        std::cout << "purchase: " << name << "\n";
    }
    virtual void acknowledgePurchase(PurchaseInfo const& info, int type) {
        std::cout << "acknowledge purchase: type=" << type << "\n";
    }
    virtual void queryPurchases() {
        std::cout << "query purchases\n";
    }
    virtual void restorePurchases() {
        std::cout << "restore purchases\n";
    }
    virtual bool isTrial() {
        //std::cout << "is trial: false\n";
        return false;
    }
    virtual void purchaseGame() {
        std::cout << "purchase game\n";
    }
    virtual bool isGameLicensed() {
        std::cout << "is game purchased: true\n";
        return true;
    }
    virtual mcpe::string getAppReceipt() {
        std::cout << "get app receipt\n";
        return mcpe::string();
    }
    virtual void registerLicenseChangeCallback() {
        std::cout << "register license change callback\n";
    }
    virtual void handleLicenseChange() {
        std::cout << "handle license changed\n";
    }
    virtual void restoreFromCache() {
        std::cout << "restore from cache\n";
    }
    virtual void getUserAccessTokenAsync() {
        std::cout << "get user access token async\n";
    }
    virtual void getFullSKUWithMetadataFromProductSku() {
        std::cout << "get full sku with metadata from product sku\n";
    }
    virtual std::string getFullGameProductSku() {
        std::cout << "get full game product sku\n";
        return "idk";
    }
    virtual std::string getLanguageCode() {
        std::cout << "get language code\n";
        return "idk";
    }
    virtual std::string getRegionCode() {
        std::cout << "get region code\n";
        return "idk";
    }
    virtual void refreshLicenses() {
        std::cout << "refresh licenses\n";
    }
    virtual void updateXUID() {
        std::cout << "update xuid\n";
    }
    virtual void onNewPrimaryUser() {
        std::cout << "on new primary user\n";
    }
};