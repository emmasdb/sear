#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <pthread.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

#include "sear.h"

using json = nlohmann::json;

static pthread_mutex_t sear_mutex = PTHREAD_MUTEX_INITIALIZER;

#if LUA_VERSION_NUM < 502
static int sear_lua_absindex(lua_State *L, int i) {
  if (i < 0 && i > LUA_REGISTRYINDEX) {
    return lua_gettop(L) + i + 1;
  }
  return i;
}

#ifndef lua_rawlen
#define lua_rawlen lua_objlen
#endif

#ifndef luaL_newlib
static void luaL_newlib(lua_State *L, const luaL_Reg *l) {
  lua_newtable(L);
  luaL_register(L, nullptr, l);
}
#endif
#else
static int sear_lua_absindex(lua_State *L, int i) { return lua_absindex(L, i); }
#endif

static json lua_to_json(lua_State *L, int index);

static bool table_is_array(lua_State *L, int index, lua_Integer *length) {
  index = sear_lua_absindex(L, index);

  lua_Integer max_index = 0;
  lua_Integer count = 0;

  lua_pushnil(L);
  while (lua_next(L, index) != 0) {
    if (lua_type(L, -2) != LUA_TNUMBER) {
      lua_pop(L, 1);
      return false;
    }

    lua_Number key = lua_tonumber(L, -2);
    lua_Integer integer_key = static_cast<lua_Integer>(key);
    if (static_cast<lua_Number>(integer_key) != key || integer_key < 1) {
      lua_pop(L, 1);
      return false;
    }

    if (integer_key > max_index) {
      max_index = integer_key;
    }

    ++count;
    lua_pop(L, 1);
  }

  if (count == 0 || count != max_index) {
    return false;
  }

  if (length != nullptr) {
    *length = max_index;
  }

  return true;
}

static json lua_table_to_object(lua_State *L, int index) {
  index = sear_lua_absindex(L, index);
  json object = json::object();

  lua_pushnil(L);
  while (lua_next(L, index) != 0) {
    if (lua_type(L, -2) != LUA_TSTRING) {
      lua_pop(L, 1);
      luaL_error(L, "SEAR Lua tables must use string keys unless they are arrays");
    }

    const char *key = lua_tostring(L, -2);
    object[key] = lua_to_json(L, -1);
    lua_pop(L, 1);
  }

  return object;
}

static json lua_to_json(lua_State *L, int index) {
  switch (lua_type(L, index)) {
    case LUA_TNIL:
      return nullptr;
    case LUA_TBOOLEAN:
      return static_cast<bool>(lua_toboolean(L, index));
    case LUA_TNUMBER:
      return lua_tonumber(L, index);
    case LUA_TSTRING:
      return std::string(lua_tostring(L, index));
    case LUA_TTABLE: {
      lua_Integer length = 0;
      if (table_is_array(L, index, &length)) {
        json array = json::array();
        index = sear_lua_absindex(L, index);
        for (lua_Integer i = 1; i <= length; ++i) {
          lua_rawgeti(L, index, static_cast<lua_Integer>(i));
          array.push_back(lua_to_json(L, -1));
          lua_pop(L, 1);
        }
        return array;
      }

      return lua_table_to_object(L, index);
    }
    default:
      luaL_error(L, "SEAR Lua requests and responses only support nil, booleans, numbers, strings, and tables");
      return nullptr;
  }
}

static void push_json(lua_State *L, const json &value) {
  if (value.is_null()) {
    lua_pushnil(L);
    return;
  }

  if (value.is_boolean()) {
    lua_pushboolean(L, value.get<bool>());
    return;
  }

  if (value.is_number()) {
    lua_pushnumber(L, value.get<lua_Number>());
    return;
  }

  if (value.is_string()) {
    const std::string &text = value.get_ref<const std::string &>();
    lua_pushlstring(L, text.data(), text.size());
    return;
  }

  if (value.is_array()) {
    lua_createtable(L, static_cast<int>(value.size()), 0);
    lua_Integer index = 1;
    for (const auto &entry : value) {
      push_json(L, entry);
      lua_rawseti(L, -2, index++);
    }
    return;
  }

  if (value.is_object()) {
    lua_newtable(L);
    for (const auto &entry : value.items()) {
      lua_pushlstring(L, entry.key().data(), entry.key().size());
      push_json(L, entry.value());
      lua_settable(L, -3);
    }
    return;
  }

  lua_pushnil(L);
}

static int lua_sear_call(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  bool debug = false;
  if (lua_gettop(L) >= 2) {
    debug = lua_toboolean(L, 2) != 0;
  }

  json request_json = lua_to_json(L, 1);
  std::string request_text = request_json.dump();

  pthread_mutex_lock(&sear_mutex);
  sear_result_t *result = sear(request_text.c_str(), static_cast<int>(request_text.size()), debug);

  std::string raw_request = result->raw_request == nullptr || result->raw_request_length <= 0
                                ? std::string()
                                : std::string(result->raw_request, result->raw_request_length);
  std::string raw_result = result->raw_result == nullptr || result->raw_result_length <= 0
                               ? std::string()
                               : std::string(result->raw_result, result->raw_result_length);
  std::string result_text = result->result_json == nullptr || result->result_json_length <= 0
                                ? std::string()
                                : std::string(result->result_json, result->result_json_length);

  pthread_mutex_unlock(&sear_mutex);

  lua_newtable(L);

  lua_pushvalue(L, 1);
  lua_setfield(L, -2, "request");

  lua_pushlstring(L, raw_request.data(), raw_request.size());
  lua_setfield(L, -2, "raw_request");

  lua_pushlstring(L, raw_result.data(), raw_result.size());
  lua_setfield(L, -2, "raw_result");

  json result_json = json::parse(result_text.begin(), result_text.end(), nullptr, false);
  if (result_json.is_discarded()) {
    luaL_error(L, "SEAR returned an invalid JSON result");
  }

  push_json(L, result_json);
  lua_setfield(L, -2, "result");

  return 1;
}

static int lua_sear_version(lua_State *L) {
  lua_pushliteral(L, "sear-lua scm");
  return 1;
}

static const luaL_Reg sear_functions[] = {
    {"sear", lua_sear_call},
    {"call", lua_sear_call},
    {"version", lua_sear_version},
    {nullptr, nullptr},
};

extern "C" int luaopen_sear(lua_State *L) {
  luaL_newlib(L, sear_functions);
  return 1;
}