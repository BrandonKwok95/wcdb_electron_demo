#include "business_item.h"

namespace kwok::wcdb_addon {

WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(BusinessItem)
WCDB_CPP_SYNTHESIZE(id)
WCDB_CPP_SYNTHESIZE(title)
WCDB_CPP_SYNTHESIZE(content)
WCDB_CPP_SYNTHESIZE(updatedAt)
WCDB_CPP_PRIMARY_ASC_AUTO_INCREMENT(id)
WCDB_CPP_ORM_IMPLEMENTATION_END

} // namespace kwok::wcdb_addon
