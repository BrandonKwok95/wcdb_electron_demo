#pragma once

#include "../orm/extra.h"

#include <WCDB/WCDBCpp.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace kwok::wcdb_addon {

struct ExtraPatch {
    bool hasKey = false;
    std::string key;
    bool hasValue = false;
    std::string value;
};

class ExtraService {
public:
    static bool initTable(WCDB::Database& database);
    static Extra create(WCDB::Database& database, Extra item);
    static std::optional<Extra> get(WCDB::Database& database, int64_t id);
    static std::vector<Extra> list(WCDB::Database& database);
    static std::vector<Extra> listByBusinessItemId(WCDB::Database& database,
                                                   int64_t businessItemId);
    static bool update(WCDB::Database& database, int64_t id, const ExtraPatch& patch);
    static bool remove(WCDB::Database& database, int64_t id);

private:
    static WCDB::Table<Extra> table(WCDB::Database& database);
    static void throwLastError(WCDB::Database& database, const char* action);
};

} // namespace kwok::wcdb_addon
