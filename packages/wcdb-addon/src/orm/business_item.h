#pragma once

#include <WCDB/WCDBCpp.h>

#include <cstdint>
#include <string>

namespace kwok::wcdb_addon {

constexpr const char* kBusinessItemTable = "business_items";

class BusinessItem {
public:
    int64_t id = 0;
    std::string title;
    std::string content;
    int64_t updatedAt = 0;

    WCDB_CPP_ORM_DECLARATION(BusinessItem)
};

} // namespace kwok::wcdb_addon
