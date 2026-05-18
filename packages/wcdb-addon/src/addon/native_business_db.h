#pragma once

#include "../orm/business_item.h"
#include "../orm/extra.h"
#include "../services/business_item_service.h"
#include "../services/extra_service.h"

#include <WCDB/WCDBCpp.h>
#include <napi.h>

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace kwok::wcdb_addon {

class NativeBusinessDb : public Napi::ObjectWrap<NativeBusinessDb> {
public:
    static Napi::Object init(Napi::Env env, Napi::Object exports);

    explicit NativeBusinessDb(const Napi::CallbackInfo& info);

    bool initSync();
    BusinessItem createBusinessItemSync(BusinessItem item);
    std::optional<BusinessItem> getBusinessItemSync(int64_t id);
    std::vector<BusinessItem> listBusinessItemsSync();
    bool updateBusinessItemSync(int64_t id, const BusinessItemPatch& patch);
    bool removeBusinessItemSync(int64_t id);
    Extra createExtraSync(Extra item);
    std::optional<Extra> getExtraSync(int64_t id);
    std::vector<Extra> listExtrasSync();
    std::vector<Extra> listExtrasByBusinessItemIdSync(int64_t businessItemId);
    bool updateExtraSync(int64_t id, const ExtraPatch& patch);
    bool removeExtraSync(int64_t id);
    void closeSync();

private:
    WCDB::Database openDatabase(bool requireInitialized);
    static void throwLastError(WCDB::Database& database, const char* action);

    Napi::Value initDb(const Napi::CallbackInfo& info);
    Napi::Value businessItemCreate(const Napi::CallbackInfo& info);
    Napi::Value businessItemGet(const Napi::CallbackInfo& info);
    Napi::Value businessItemList(const Napi::CallbackInfo& info);
    Napi::Value businessItemUpdate(const Napi::CallbackInfo& info);
    Napi::Value businessItemRemove(const Napi::CallbackInfo& info);
    Napi::Value extraCreate(const Napi::CallbackInfo& info);
    Napi::Value extraGet(const Napi::CallbackInfo& info);
    Napi::Value extraList(const Napi::CallbackInfo& info);
    Napi::Value extraListByBusinessItemId(const Napi::CallbackInfo& info);
    Napi::Value extraUpdate(const Napi::CallbackInfo& info);
    Napi::Value extraRemove(const Napi::CallbackInfo& info);
    Napi::Value close(const Napi::CallbackInfo& info);

    std::string dbPath_;
    bool closed_ = false;
    bool initialized_ = false;
    std::mutex stateMutex_;
};

Napi::Object toNapiObject(Napi::Env env, const BusinessItem& item);
Napi::Object toNapiObject(Napi::Env env, const Extra& item);
BusinessItem businessItemFromInput(const Napi::Object& input);
BusinessItemPatch businessItemPatchFromInput(const Napi::Object& input);
Extra extraFromInput(const Napi::Object& input);
ExtraPatch extraPatchFromInput(const Napi::Object& input);

} // namespace kwok::wcdb_addon
