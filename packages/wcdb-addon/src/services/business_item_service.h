#pragma once

#include "../orm/business_item.h"

#include <WCDB/WCDBCpp.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace kwok::wcdb_addon {

struct BusinessItemPatch {
    bool hasTitle = false;
    std::string title;
    bool hasContent = false;
    std::string content;
};

class BusinessItemService {
public:
    static bool initTable(WCDB::Database& database);
    static BusinessItem create(WCDB::Database& database, BusinessItem item);
    static std::optional<BusinessItem> get(WCDB::Database& database, int64_t id);
    static std::vector<BusinessItem> list(WCDB::Database& database);
    static bool update(WCDB::Database& database, int64_t id, const BusinessItemPatch& patch);
    static bool remove(WCDB::Database& database, int64_t id);

private:
    static WCDB::Table<BusinessItem> table(WCDB::Database& database);
    static void throwLastError(WCDB::Database& database, const char* action);
};

} // namespace kwok::wcdb_addon
