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
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "cflags_cc": [
        "-fexceptions"
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1,
          "AdditionalOptions": [
            "/std:c++17"
          ]
        }
      },
      "conditions": [
        [
          "OS=='win'",
          {
            "include_dirs": [
              "vendor/wcdb/windows-x64/include"
            ],
            "libraries": [
              "<(module_root_dir)/vendor/wcdb/windows-x64/lib/WCDB.lib",
              "<(module_root_dir)/vendor/wcdb/windows-x64/lib/sqlcipher.lib",
              "<(module_root_dir)/vendor/wcdb/windows-x64/lib/zstd.lib"
            ],
            "copies": [
              {
                "destination": "<(PRODUCT_DIR)",
                "files": [
                  "vendor/wcdb/windows-x64/lib/WCDB.dll"
                ]
              }
            ]
          }
        ],
        [
          "OS=='mac' and target_arch=='arm64'",
          {
            "include_dirs": [
              "vendor/wcdb/darwin-arm64/include"
            ],
            "libraries": [
              "-framework WCDB"
            ],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "11.0",
              "OTHER_LDFLAGS": [
                "-F<(module_root_dir)/vendor/wcdb/darwin-arm64/lib",
                "-Wl,-rpath,@loader_path"
              ]
            },
            "copies": [
              {
                "destination": "<(PRODUCT_DIR)",
                "files": [
                  "vendor/wcdb/darwin-arm64/lib/WCDB.framework"
                ]
              }
            ]
          }
        ]
      ]
    }
  ]
}
