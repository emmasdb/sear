{
  "variables": {
    "openssl_root%": "<!(node -p \"process.env.OPENSSL_ROOT || ((process.env.ZOPEN_ROOTFS && process.env.ZOPEN_ROOTFS + '/usr/local') || '')\")",
    "zoslib_root%": "<!(node -p \"process.env.ZOSLIB_ROOT || ((process.env.ZOPEN_ROOTFS && process.env.ZOPEN_ROOTFS + '/usr/local') || '')\")"
  },
  "targets": [{
    "target_name": "_sear",
    "sources": [
      "sear/nodejs/_sear.cpp",
      "sear/sear.cpp",
      "sear/conversion.cpp",
      "sear/logger.cpp",
      "sear/sear_error.cpp",
      "sear/security_admin.cpp",
      "sear/security_request.cpp",
      "sear/irrsdl00/irrsdl00.cpp",
      "sear/irrsdl00/keyring_extractor.cpp",
      "sear/irrsdl00/keyring_modifier.cpp",
      "sear/irrsdl00/keyring_post_processor.cpp",
      "sear/irrseq00/profile_extractor.cpp",
      "sear/irrseq00/profile_post_processor.cpp",
      "sear/irrsmo00/irrsmo00.cpp",
      "sear/irrsmo00/irrsmo00_error.cpp",
      "sear/irrsmo00/xml_generator.cpp",
      "sear/irrsmo00/xml_parser.cpp",
      "sear/key_map/key_map.cpp",
      "sear/validation/trait_validation.cpp",
      "externals/json-schema-validator/json-patch.cpp",
      "externals/json-schema-validator/json-schema-draft7.json.cpp",
      "externals/json-schema-validator/json-uri.cpp",
      "externals/json-schema-validator/json-validator.cpp",
      "externals/json-schema-validator/smtp-address-validator.cpp",
      "externals/json-schema-validator/string-format-check.cpp",
      "externals/pugixml/pugixml.cpp"
    ],
    "include_dirs": [
      "sear",
      "sear/irrsdl00",
      "sear/irrseq00",
      "sear/irrsmo00",
      "sear/key_map",
      "sear/validation",
      "externals/json",
      "externals/json-schema-validator",
      "externals/pugixml",
      "externals/iconv",
      "<(openssl_root)/include"
    ],
    "cflags!": [
      "-fno-exceptions",
      "-q64",
      "-qlonglong",
      "-qenum=int",
      "-qxclang=-fexec-charset=ISO8859-1",
      "-qmakedep=gcc"
    ],
    "cflags_cc!": [
      "-fno-exceptions",
      "-q64",
      "-qlonglong",
      "-qenum=int",
      "-qxclang=-fexec-charset=ISO8859-1",
      "-qmakedep=gcc"
    ],
    "extra_link_args": [
      "-m64",
      "-Wl,-b,edit=no",
      "<!(node -p \"require('path').resolve('artifacts/irrseq00.o')\")",
      "-Wl,<(openssl_root)/lib/libcrypto.a",
      "-Wl,<(openssl_root)/lib/libssl.a",
      "-Wl,<(zoslib_root)/lib/libzoslib.a"
    ],
    "cflags": ["-std=c99", "-m64", "-fzos-le-char-mode=ascii"],
    "cflags_cc": ["-std=c++17", "-m64", "-fzos-le-char-mode=ascii", "-fexceptions"]
  }]
}