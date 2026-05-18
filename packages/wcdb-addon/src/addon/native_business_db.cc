#include "native_business_db.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace kwok::wcdb_addon {

namespace {

std::string getStringProperty(const Napi::Object& object,
                              const char* key,
                              const std::string& fallback = "")
{
    Napi::Value value = object.Get(key);
    if (value.IsUndefined() || value.IsNull()) {
        return fallback;
    }
    if (!value.IsString()) {
        throw Napi::TypeError::New(object.Env(), std::string(key) + " must be a string");
    }
    return value.As<Napi::String>().Utf8Value();
}

bool hasPresentProperty(const Napi::Object& object, const char* key)
{
    Napi::Value value = object.Get(key);
    return !value.IsUndefined() && !value.IsNull();
}

class PromiseWorker : public Napi::AsyncWorker {
public:
    PromiseWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, const char* name)
        : Napi::AsyncWorker(info.Env(), name),
          deferred_(Napi::Promise::Deferred::New(info.Env())),
          owner_(Napi::Persistent(info.This().As<Napi::Object>())),
          db_(db)
    {
    }

    Napi::Promise promise() const
    {
        return deferred_.Promise();
    }

    void OnError(const Napi::Error& error) override
    {
        deferred_.Reject(error.Value());
        owner_.Unref();
    }

protected:
    Napi::Promise::Deferred deferred_;
    Napi::ObjectReference owner_;
    NativeBusinessDb* db_;
};

class InitWorker final : public PromiseWorker {
public:
    InitWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db)
        : PromiseWorker(info, db, "NativeBusinessDb.init")
    {
    }

    void Execute() override
    {
        try {
            db_->initSync();
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Napi::Boolean::New(Env(), true));
        owner_.Unref();
    }
};

class CreateBusinessItemWorker final : public PromiseWorker {
public:
    CreateBusinessItemWorker(const Napi::CallbackInfo& info,
                             NativeBusinessDb* db,
                             BusinessItem item)
        : PromiseWorker(info, db, "NativeBusinessDb.businessItemCreate"),
          item_(std::move(item))
    {
    }

    void Execute() override
    {
        try {
            item_ = db_->createBusinessItemSync(std::move(item_));
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(toNapiObject(Env(), item_));
        owner_.Unref();
    }

private:
    BusinessItem item_;
};

class GetBusinessItemWorker final : public PromiseWorker {
public:
    GetBusinessItemWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, int64_t id)
        : PromiseWorker(info, db, "NativeBusinessDb.businessItemGet"),
          id_(id)
    {
    }

    void Execute() override
    {
        try {
            item_ = db_->getBusinessItemSync(id_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        if (item_.has_value()) {
            deferred_.Resolve(toNapiObject(Env(), item_.value()));
        } else {
            deferred_.Resolve(Env().Null());
        }
        owner_.Unref();
    }

private:
    int64_t id_;
    std::optional<BusinessItem> item_;
};

class ListBusinessItemsWorker final : public PromiseWorker {
public:
    ListBusinessItemsWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db)
        : PromiseWorker(info, db, "NativeBusinessDb.businessItemList")
    {
    }

    void Execute() override
    {
        try {
            items_ = db_->listBusinessItemsSync();
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        Napi::Array result = Napi::Array::New(Env(), items_.size());
        uint32_t index = 0;
        for (const auto& item : items_) {
            result.Set(index++, toNapiObject(Env(), item));
        }
        deferred_.Resolve(result);
        owner_.Unref();
    }

private:
    std::vector<BusinessItem> items_;
};

class UpdateBusinessItemWorker final : public PromiseWorker {
public:
    UpdateBusinessItemWorker(const Napi::CallbackInfo& info,
                             NativeBusinessDb* db,
                             int64_t id,
                             BusinessItemPatch patch)
        : PromiseWorker(info, db, "NativeBusinessDb.businessItemUpdate"),
          id_(id),
          patch_(std::move(patch))
    {
    }

    void Execute() override
    {
        try {
            updated_ = db_->updateBusinessItemSync(id_, patch_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Napi::Boolean::New(Env(), updated_));
        owner_.Unref();
    }

private:
    int64_t id_;
    BusinessItemPatch patch_;
    bool updated_ = false;
};

class RemoveBusinessItemWorker final : public PromiseWorker {
public:
    RemoveBusinessItemWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, int64_t id)
        : PromiseWorker(info, db, "NativeBusinessDb.businessItemRemove"),
          id_(id)
    {
    }

    void Execute() override
    {
        try {
            removed_ = db_->removeBusinessItemSync(id_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Napi::Boolean::New(Env(), removed_));
        owner_.Unref();
    }

private:
    int64_t id_;
    bool removed_ = false;
};

class CreateExtraWorker final : public PromiseWorker {
public:
    CreateExtraWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, Extra item)
        : PromiseWorker(info, db, "NativeBusinessDb.extraCreate"),
          item_(std::move(item))
    {
    }

    void Execute() override
    {
        try {
            item_ = db_->createExtraSync(std::move(item_));
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(toNapiObject(Env(), item_));
        owner_.Unref();
    }

private:
    Extra item_;
};

class GetExtraWorker final : public PromiseWorker {
public:
    GetExtraWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, int64_t id)
        : PromiseWorker(info, db, "NativeBusinessDb.extraGet"),
          id_(id)
    {
    }

    void Execute() override
    {
        try {
            item_ = db_->getExtraSync(id_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        if (item_.has_value()) {
            deferred_.Resolve(toNapiObject(Env(), item_.value()));
        } else {
            deferred_.Resolve(Env().Null());
        }
        owner_.Unref();
    }

private:
    int64_t id_;
    std::optional<Extra> item_;
};

class ListExtrasWorker final : public PromiseWorker {
public:
    ListExtrasWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db)
        : PromiseWorker(info, db, "NativeBusinessDb.extraList")
    {
    }

    void Execute() override
    {
        try {
            items_ = db_->listExtrasSync();
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        Napi::Array result = Napi::Array::New(Env(), items_.size());
        uint32_t index = 0;
        for (const auto& item : items_) {
            result.Set(index++, toNapiObject(Env(), item));
        }
        deferred_.Resolve(result);
        owner_.Unref();
    }

private:
    std::vector<Extra> items_;
};

class ListExtrasByBusinessItemIdWorker final : public PromiseWorker {
public:
    ListExtrasByBusinessItemIdWorker(const Napi::CallbackInfo& info,
                                     NativeBusinessDb* db,
                                     int64_t businessItemId)
        : PromiseWorker(info, db, "NativeBusinessDb.extraListByBusinessItemId"),
          businessItemId_(businessItemId)
    {
    }

    void Execute() override
    {
        try {
            items_ = db_->listExtrasByBusinessItemIdSync(businessItemId_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        Napi::Array result = Napi::Array::New(Env(), items_.size());
        uint32_t index = 0;
        for (const auto& item : items_) {
            result.Set(index++, toNapiObject(Env(), item));
        }
        deferred_.Resolve(result);
        owner_.Unref();
    }

private:
    int64_t businessItemId_;
    std::vector<Extra> items_;
};

class UpdateExtraWorker final : public PromiseWorker {
public:
    UpdateExtraWorker(const Napi::CallbackInfo& info,
                      NativeBusinessDb* db,
                      int64_t id,
                      ExtraPatch patch)
        : PromiseWorker(info, db, "NativeBusinessDb.extraUpdate"),
          id_(id),
          patch_(std::move(patch))
    {
    }

    void Execute() override
    {
        try {
            updated_ = db_->updateExtraSync(id_, patch_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Napi::Boolean::New(Env(), updated_));
        owner_.Unref();
    }

private:
    int64_t id_;
    ExtraPatch patch_;
    bool updated_ = false;
};

class RemoveExtraWorker final : public PromiseWorker {
public:
    RemoveExtraWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db, int64_t id)
        : PromiseWorker(info, db, "NativeBusinessDb.extraRemove"),
          id_(id)
    {
    }

    void Execute() override
    {
        try {
            removed_ = db_->removeExtraSync(id_);
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Napi::Boolean::New(Env(), removed_));
        owner_.Unref();
    }

private:
    int64_t id_;
    bool removed_ = false;
};

class CloseWorker final : public PromiseWorker {
public:
    CloseWorker(const Napi::CallbackInfo& info, NativeBusinessDb* db)
        : PromiseWorker(info, db, "NativeBusinessDb.close")
    {
    }

    void Execute() override
    {
        try {
            db_->closeSync();
        } catch (const std::exception& error) {
            SetError(error.what());
        }
    }

    void OnOK() override
    {
        deferred_.Resolve(Env().Undefined());
        owner_.Unref();
    }
};

} // namespace

NativeBusinessDb::NativeBusinessDb(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<NativeBusinessDb>(info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        throw Napi::TypeError::New(env, "NativeBusinessDb requires a database path");
    }

    dbPath_ = info[0].As<Napi::String>().Utf8Value();
}

Napi::Object NativeBusinessDb::init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(
        env,
        "NativeBusinessDb",
        {
            InstanceMethod("init", &NativeBusinessDb::initDb),
            InstanceMethod("businessItemCreate", &NativeBusinessDb::businessItemCreate),
            InstanceMethod("businessItemGet", &NativeBusinessDb::businessItemGet),
            InstanceMethod("businessItemList", &NativeBusinessDb::businessItemList),
            InstanceMethod("businessItemUpdate", &NativeBusinessDb::businessItemUpdate),
            InstanceMethod("businessItemRemove", &NativeBusinessDb::businessItemRemove),
            InstanceMethod("extraCreate", &NativeBusinessDb::extraCreate),
            InstanceMethod("extraGet", &NativeBusinessDb::extraGet),
            InstanceMethod("extraList", &NativeBusinessDb::extraList),
            InstanceMethod("extraListByBusinessItemId",
                           &NativeBusinessDb::extraListByBusinessItemId),
            InstanceMethod("extraUpdate", &NativeBusinessDb::extraUpdate),
            InstanceMethod("extraRemove", &NativeBusinessDb::extraRemove),
            InstanceMethod("close", &NativeBusinessDb::close),
        });

    exports.Set("NativeBusinessDb", func);
    return exports;
}

bool NativeBusinessDb::initSync()
{
    auto database = openDatabase(false);
    BusinessItemService::initTable(database);
    ExtraService::initTable(database);

    std::lock_guard<std::mutex> lock(stateMutex_);
    if (closed_) {
        throw std::runtime_error("database is closed");
    }
    initialized_ = true;
    return true;
}

BusinessItem NativeBusinessDb::createBusinessItemSync(BusinessItem item)
{
    auto database = openDatabase(true);
    return BusinessItemService::create(database, std::move(item));
}

std::optional<BusinessItem> NativeBusinessDb::getBusinessItemSync(int64_t id)
{
    auto database = openDatabase(true);
    return BusinessItemService::get(database, id);
}

std::vector<BusinessItem> NativeBusinessDb::listBusinessItemsSync()
{
    auto database = openDatabase(true);
    return BusinessItemService::list(database);
}

bool NativeBusinessDb::updateBusinessItemSync(int64_t id, const BusinessItemPatch& patch)
{
    auto database = openDatabase(true);
    return BusinessItemService::update(database, id, patch);
}

bool NativeBusinessDb::removeBusinessItemSync(int64_t id)
{
    auto database = openDatabase(true);
    return BusinessItemService::remove(database, id);
}

Extra NativeBusinessDb::createExtraSync(Extra item)
{
    auto database = openDatabase(true);
    return ExtraService::create(database, std::move(item));
}

std::optional<Extra> NativeBusinessDb::getExtraSync(int64_t id)
{
    auto database = openDatabase(true);
    return ExtraService::get(database, id);
}

std::vector<Extra> NativeBusinessDb::listExtrasSync()
{
    auto database = openDatabase(true);
    return ExtraService::list(database);
}

std::vector<Extra> NativeBusinessDb::listExtrasByBusinessItemIdSync(int64_t businessItemId)
{
    auto database = openDatabase(true);
    return ExtraService::listByBusinessItemId(database, businessItemId);
}

bool NativeBusinessDb::updateExtraSync(int64_t id, const ExtraPatch& patch)
{
    auto database = openDatabase(true);
    return ExtraService::update(database, id, patch);
}

bool NativeBusinessDb::removeExtraSync(int64_t id)
{
    auto database = openDatabase(true);
    return ExtraService::remove(database, id);
}

void NativeBusinessDb::closeSync()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    closed_ = true;
    initialized_ = false;
}

WCDB::Database NativeBusinessDb::openDatabase(bool requireInitialized)
{
    std::string dbPath;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (closed_) {
            throw std::runtime_error("database is closed");
        }
        if (requireInitialized && !initialized_) {
            throw std::runtime_error("database is not initialized");
        }
        dbPath = dbPath_;
    }

    WCDB::Database database(dbPath);
    if (!database.canOpen()) {
        throwLastError(database, "open database");
    }
    return database;
}

void NativeBusinessDb::throwLastError(WCDB::Database& database, const char* action)
{
    const WCDB::Error& error = database.getError();
    std::string message = std::string(action) + " failed";
    if (!error.getMessage().empty()) {
        message += ": ";
        message += error.getMessage();
    }
    throw std::runtime_error(message);
}

Napi::Value NativeBusinessDb::initDb(const Napi::CallbackInfo& info)
{
    auto* worker = new InitWorker(info, this);
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::businessItemCreate(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        throw Napi::TypeError::New(env, "businessItem.create requires an item object");
    }

    auto* worker = new CreateBusinessItemWorker(
        info, this, businessItemFromInput(info[0].As<Napi::Object>()));
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::businessItemGet(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, "businessItem.get requires a numeric id");
    }

    auto* worker = new GetBusinessItemWorker(
        info, this, info[0].As<Napi::Number>().Int64Value());
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::businessItemList(const Napi::CallbackInfo& info)
{
    auto* worker = new ListBusinessItemsWorker(info, this);
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::businessItemUpdate(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsObject()) {
        throw Napi::TypeError::New(env, "businessItem.update requires an id and patch object");
    }

    auto* worker = new UpdateBusinessItemWorker(info,
                                                this,
                                                info[0].As<Napi::Number>().Int64Value(),
                                                businessItemPatchFromInput(info[1].As<Napi::Object>()));
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::businessItemRemove(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, "businessItem.remove requires a numeric id");
    }

    auto* worker = new RemoveBusinessItemWorker(
        info, this, info[0].As<Napi::Number>().Int64Value());
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraCreate(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        throw Napi::TypeError::New(env, "extra.create requires an item object");
    }

    auto* worker = new CreateExtraWorker(info, this, extraFromInput(info[0].As<Napi::Object>()));
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraGet(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, "extra.get requires a numeric id");
    }

    auto* worker = new GetExtraWorker(info, this, info[0].As<Napi::Number>().Int64Value());
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraList(const Napi::CallbackInfo& info)
{
    auto* worker = new ListExtrasWorker(info, this);
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraListByBusinessItemId(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, "extra.listByBusinessItemId requires a numeric id");
    }

    auto* worker = new ListExtrasByBusinessItemIdWorker(
        info, this, info[0].As<Napi::Number>().Int64Value());
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraUpdate(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsObject()) {
        throw Napi::TypeError::New(env, "extra.update requires an id and patch object");
    }

    auto* worker = new UpdateExtraWorker(info,
                                         this,
                                         info[0].As<Napi::Number>().Int64Value(),
                                         extraPatchFromInput(info[1].As<Napi::Object>()));
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::extraRemove(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
        throw Napi::TypeError::New(env, "extra.remove requires a numeric id");
    }

    auto* worker = new RemoveExtraWorker(info, this, info[0].As<Napi::Number>().Int64Value());
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Value NativeBusinessDb::close(const Napi::CallbackInfo& info)
{
    auto* worker = new CloseWorker(info, this);
    auto promise = worker->promise();
    worker->Queue();
    return promise;
}

Napi::Object toNapiObject(Napi::Env env, const BusinessItem& item)
{
    Napi::Object object = Napi::Object::New(env);
    object.Set("id", Napi::Number::New(env, static_cast<double>(item.id)));
    object.Set("title", Napi::String::New(env, item.title));
    object.Set("content", Napi::String::New(env, item.content));
    object.Set("updatedAt", Napi::Number::New(env, static_cast<double>(item.updatedAt)));
    return object;
}

Napi::Object toNapiObject(Napi::Env env, const Extra& item)
{
    Napi::Object object = Napi::Object::New(env);
    object.Set("id", Napi::Number::New(env, static_cast<double>(item.id)));
    object.Set("businessItemId",
               Napi::Number::New(env, static_cast<double>(item.businessItemId)));
    object.Set("key", Napi::String::New(env, item.key));
    object.Set("value", Napi::String::New(env, item.value));
    object.Set("updatedAt", Napi::Number::New(env, static_cast<double>(item.updatedAt)));
    return object;
}

BusinessItem businessItemFromInput(const Napi::Object& input)
{
    BusinessItem item;
    item.title = getStringProperty(input, "title");
    item.content = getStringProperty(input, "content");
    return item;
}

BusinessItemPatch businessItemPatchFromInput(const Napi::Object& input)
{
    BusinessItemPatch patch;
    patch.hasTitle = hasPresentProperty(input, "title");
    patch.hasContent = hasPresentProperty(input, "content");
    if (patch.hasTitle) {
        patch.title = getStringProperty(input, "title");
    }
    if (patch.hasContent) {
        patch.content = getStringProperty(input, "content");
    }
    return patch;
}

Extra extraFromInput(const Napi::Object& input)
{
    Napi::Env env = input.Env();
    Napi::Value businessItemId = input.Get("businessItemId");
    if (!businessItemId.IsNumber()) {
        throw Napi::TypeError::New(env, "businessItemId must be a number");
    }

    Extra item;
    item.businessItemId = businessItemId.As<Napi::Number>().Int64Value();
    item.key = getStringProperty(input, "key");
    item.value = getStringProperty(input, "value");
    return item;
}

ExtraPatch extraPatchFromInput(const Napi::Object& input)
{
    ExtraPatch patch;
    patch.hasKey = hasPresentProperty(input, "key");
    patch.hasValue = hasPresentProperty(input, "value");
    if (patch.hasKey) {
        patch.key = getStringProperty(input, "key");
    }
    if (patch.hasValue) {
        patch.value = getStringProperty(input, "value");
    }
    return patch;
}

} // namespace kwok::wcdb_addon
