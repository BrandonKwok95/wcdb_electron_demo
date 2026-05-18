#include "extra_service.h"

#include <ctime>
#include <stdexcept>

namespace kwok::wcdb_addon {

namespace {

int64_t nowMillis()
{
    return static_cast<int64_t>(std::time(nullptr)) * 1000;
}

} // namespace

bool ExtraService::initTable(WCDB::Database& database)
{
    if (!database.createTable<Extra>(kExtraTable)) {
        throwLastError(database, "create extra table");
    }
    return true;
}

Extra ExtraService::create(WCDB::Database& database, Extra item)
{
    item.updatedAt = nowMillis();
    item.isAutoIncrement = true;

    auto extraTable = table(database);
    if (!extraTable.insertObject(item)) {
        throwLastError(database, "insert extra");
    }

    item.id = *item.lastInsertedRowID;
    return item;
}

std::optional<Extra> ExtraService::get(WCDB::Database& database, int64_t id)
{
    auto items = table(database).getAllObjects(WCDB_FIELD(Extra::id) == id,
                                              WCDB::OrderingTerms(),
                                              WCDB::Expression(1));
    if (!items.succeed()) {
        throwLastError(database, "get extra");
    }
    if (items.value().empty()) {
        return std::nullopt;
    }
    return items.value().front();
}

std::vector<Extra> ExtraService::list(WCDB::Database& database)
{
    auto items = table(database).getAllObjects();
    if (!items.succeed()) {
        throwLastError(database, "list extras");
    }
    return items.value();
}

std::vector<Extra> ExtraService::listByBusinessItemId(WCDB::Database& database,
                                                      int64_t businessItemId)
{
    auto items = table(database).getAllObjects(
        WCDB_FIELD(Extra::businessItemId) == businessItemId);
    if (!items.succeed()) {
        throwLastError(database, "list extras by business item");
    }
    return items.value();
}

bool ExtraService::update(WCDB::Database& database, int64_t id, const ExtraPatch& patch)
{
    auto item = get(database, id);
    if (!item.has_value()) {
        return false;
    }

    Extra next = item.value();
    if (patch.hasKey) {
        next.key = patch.key;
    }
    if (patch.hasValue) {
        next.value = patch.value;
    }
    next.updatedAt = nowMillis();

    WCDB::Fields fields = {
        WCDB_FIELD(Extra::key),
        WCDB_FIELD(Extra::value),
        WCDB_FIELD(Extra::updatedAt),
    };

    if (!table(database).updateObject(next, fields, WCDB_FIELD(Extra::id) == id)) {
        throwLastError(database, "update extra");
    }
    return true;
}

bool ExtraService::remove(WCDB::Database& database, int64_t id)
{
    if (!table(database).deleteObjects(WCDB_FIELD(Extra::id) == id)) {
        throwLastError(database, "delete extra");
    }
    return true;
}

WCDB::Table<Extra> ExtraService::table(WCDB::Database& database)
{
    return database.getTable<Extra>(kExtraTable);
}

void ExtraService::throwLastError(WCDB::Database& database, const char* action)
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
