{
  "targets": [
    {
      "target_name": "kwok_wcdb_addon",
      "sources": [
        "src/addon/addon.cc",
        "src/addon/native_business_db.cc",
        "src/orm/business_item.cc",
        "src/orm/extra.cc",
        "src/services/business_item_service.cc",
        "src/services/extra_service.cc"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "vendor/wcdb/windows-x64/include"
      ],
      "libraries": [
        "<(module_root_dir)/vendor/wcdb/windows-x64/lib/WCDB.lib",
        "<(module_root_dir)/vendor/wcdb/windows-x64/lib/sqlcipher.lib",
        "<(module_root_dir)/vendor/wcdb/windows-x64/lib/zstd.lib"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1,
          "AdditionalOptions": [
            "/std:c++17"
          ]
        }
      },
      "copies": [
        {
          "destination": "<(PRODUCT_DIR)",
          "files": [
            "vendor/wcdb/windows-x64/lib/WCDB.dll"
          ]
        }
      ]
    }
  ]
}
