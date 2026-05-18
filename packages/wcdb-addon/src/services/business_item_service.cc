#include "business_item_service.h"

#include <ctime>
#include <stdexcept>

namespace kwok::wcdb_addon {

namespace {

int64_t nowMillis()
{
    return static_cast<int64_t>(std::time(nullptr)) * 1000;
}

} // namespace

bool BusinessItemService::initTable(WCDB::Database& database)
{
    if (!database.createTable<BusinessItem>(kBusinessItemTable)) {
        throwLastError(database, "create business item table");
    }
    return true;
}

BusinessItem BusinessItemService::create(WCDB::Database& database, BusinessItem item)
{
    item.updatedAt = nowMillis();
    item.isAutoIncrement = true;

    auto businessItemTable = table(database);
    if (!businessItemTable.insertObject(item)) {
        throwLastError(database, "insert business item");
    }

    item.id = *item.lastInsertedRowID;
    return item;
}

std::optional<BusinessItem> BusinessItemService::get(WCDB::Database& database, int64_t id)
{
    auto items = table(database).getAllObjects(WCDB_FIELD(BusinessItem::id) == id,
                                              WCDB::OrderingTerms(),
                                              WCDB::Expression(1));
    if (!items.succeed()) {
        throwLastError(database, "get business item");
    }
    if (items.value().empty()) {
        return std::nullopt;
    }
    return items.value().front();
}

std::vector<BusinessItem> BusinessItemService::list(WCDB::Database& database)
{
    auto items = table(database).getAllObjects();
    if (!items.succeed()) {
        throwLastError(database, "list business items");
    }
    return items.value();
}

bool BusinessItemService::update(WCDB::Database& database,
                                 int64_t id,
                                 const BusinessItemPatch& patch)
{
    auto item = get(database, id);
    if (!item.has_value()) {
        return false;
    }

    BusinessItem next = item.value();
    if (patch.hasTitle) {
        next.title = patch.title;
    }
    if (patch.hasContent) {
        next.content = patch.content;
    }
    next.updatedAt = nowMillis();

    WCDB::Fields fields = {
        WCDB_FIELD(BusinessItem::title),
        WCDB_FIELD(BusinessItem::content),
        WCDB_FIELD(BusinessItem::updatedAt),
    };

    if (!table(database).updateObject(next, fields, WCDB_FIELD(BusinessItem::id) == id)) {
        throwLastError(database, "update business item");
    }
    return true;
}

bool BusinessItemService::remove(WCDB::Database& database, int64_t id)
{
    if (!table(database).deleteObjects(WCDB_FIELD(BusinessItem::id) == id)) {
        throwLastError(database, "delete business item");
    }
    return true;
}

WCDB::Table<BusinessItem> BusinessItemService::table(WCDB::Database& database)
{
    return database.getTable<BusinessItem>(kBusinessItemTable);
}

void BusinessItemService::throwLastError(WCDB::Database& database, const char* action)
{
    const WCDB::Error& error = database.getError();
    std::string message = std::string(action) + " failed";
    if (!error.getMessage().empty()) {
        message += ": ";
        message += error.getMessage();
    }
    throw std::runtime_error(message);
}

} // namespace kwok::wcdb_addon
