package = "sear-lua"
version = "scm-1"

source = {
  url = "git+https://github.com/Mainframe-Renewal-Project/sear",
  branch = "main",
}

description = {
  summary = "Lua interface for SEAR",
  detailed = "Lua bindings for the SEAR Security API for RACF, packaged for LuaRocks.",
  homepage = "https://github.com/Mainframe-Renewal-Project/sear",
  license = "Apache-2.0",
}

dependencies = {
  "lua >= 5.1",
}

build = {
  type = "cmake",
  variables = {
    SEAR_ENABLE_LUA = "ON",
  },
}