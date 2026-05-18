#include "native_business_db.h"

#include <napi.h>

namespace kwok::wcdb_addon {

Napi::Object initAddon(Napi::Env env, Napi::Object exports)
{
    return NativeBusinessDb::init(env, exports);
}

NODE_API_MODULE(kwok_wcdb_addon, initAddon)

} // namespace kwok::wcdb_addon
