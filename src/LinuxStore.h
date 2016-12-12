#pragma once

#include <iostream>

struct StoreListener;
struct PurchaseInfo;
struct LinuxStore {
    virtual ~LinuxStore() {
        std::cout << "destroying store\n";
    }
    virtual bool requiresRestorePurchasesButton() {
        std::cout << "requires restore purchases button: false\n";
        return false;
    }
    virtual bool allowsSubscriptions() {
        std::cout << "allows subscriptions: false\n";
        return false;
    }
    virtual std::string getStoreId() {
        std::cout << "get store id: LinuxStore\n";
        return "LinuxStore";
    }
    virtual std::string getSubPlatformStoreId() {
        std::cout << "get sub platform store id: LinuxStore\n";
        return "LinuxStore";
    }
    virtual std::string getProductSkuPrefix() {
        std::cout << "get product sku prefix: linux";
        return "linux";
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
    virtual void getAppReceipt() {
        std::cout << "get app receipt\n";
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
};