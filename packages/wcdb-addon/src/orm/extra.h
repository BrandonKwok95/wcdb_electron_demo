#pragma once

#include <WCDB/WCDBCpp.h>

#include <cstdint>
#include <string>

namespace kwok::wcdb_addon {

constexpr const char* kExtraTable = "extras";

class Extra {
public:
    int64_t id = 0;
    int64_t businessItemId = 0;
    std::string key;
    std::string value;
    int64_t updatedAt = 0;

    WCDB_CPP_ORM_DECLARATION(Extra)
};

} // namespace kwok::wcdb_addon
