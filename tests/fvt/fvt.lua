local sear = require("sear")

local function pretty_print(value, indent)
  indent = indent or ""
  local value_type = type(value)

  if value_type == "table" then
    local is_array = true
    local count = 0
    for key, _ in pairs(value) do
      count = count + 1
      if type(key) ~= "number" or key < 1 or math.floor(key) ~= key then
        is_array = false
      end
    end

    local pieces = { is_array and "[" or "{" }
    local child_indent = indent .. "  "
    local first = true

    if is_array then
      for index = 1, count do
        if not first then
          table.insert(pieces, ",")
        end
        table.insert(pieces, "\n" .. child_indent .. pretty_print(value[index], child_indent))
        first = false
      end
    else
      for key, entry in pairs(value) do
        if not first then
          table.insert(pieces, ",")
        end
        table.insert(pieces, "\n" .. child_indent .. tostring(key) .. " = " .. pretty_print(entry, child_indent))
        first = false
      end
    end

    if not first then
      table.insert(pieces, "\n" .. indent)
    end

    table.insert(pieces, is_array and "]" or "}")
    return table.concat(pieces)
  end

  if value_type == "string" then
    return string.format("%q", value)
  end

  return tostring(value)
end

local missing_userid = os.getenv("SEAR_FVT_USERID")
if not missing_userid or missing_userid == "" then
  io.stderr:write(
    "The 'SEAR_FVT_USERID' environment variable must be set to a z/OS userid that does NOT exist on the system.\n"
  )
  os.exit(1)
end

local current_user = os.getenv("USER") or os.getenv("LOGNAME")
if not current_user or current_user == "" then
  io.stderr:write("The current USS userid must be available through USER or LOGNAME.\n")
  os.exit(1)
end

local extract_request = {
  admin_type = "user",
  operation = "extract",
  userid = current_user,
}

local delete_request = {
  admin_type = "user",
  operation = "delete",
  userid = missing_userid,
}

io.write("Extract Test (IRRSEQ00):\n")
local result = sear.sear(extract_request)
io.write(pretty_print(result.result) .. "\n")

io.write("Delete Test (IRRSMO00):\n")
result = sear.sear(delete_request)
io.write(pretty_print(result.result) .. "\n")

io.write("Debug Test (IRRSMO00):\n")
result = sear.sear(delete_request, true)
io.write(pretty_print(result.result) .. "\n")