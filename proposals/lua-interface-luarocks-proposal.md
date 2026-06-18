# Proposal: SEAR Lua Interface Packaged with LuaRocks

## Summary

This proposal describes a Lua interface for SEAR that can be distributed and installed through LuaRocks. The goal is to make SEAR accessible to Lua applications on z/OS while preserving the existing SEAR core implementation and JSON-based request model.

## Goals

- Provide a native Lua API for invoking SEAR operations.
- Package the interface as a LuaRocks rock for simple installation and versioning.
- Reuse the existing SEAR C/C++ core instead of reimplementing RACF logic.
- Offer a small, idiomatic Lua surface that maps cleanly to SEAR requests and responses.
- Support both standalone installation and integration into larger Lua applications.

## Non-goals

- Rewriting SEAR in Lua.
- Replacing the existing Python or Node.js interfaces.
- Introducing a Lua-specific security model beyond the capabilities already exposed by SEAR.
- Adding RACF functionality that does not already exist in the SEAR core.

## Proposed Architecture

### High-level design

The Lua interface would consist of:

1. A Lua C module that binds to the SEAR native library.
2. A thin translation layer between Lua tables and SEAR JSON request/response objects.
3. A LuaRocks rockspec that builds and installs the module.
4. Optional helper Lua code for request construction, error handling, and examples.

### Interface shape

A minimal API could look like this:

- `sear.call(request_table)`
- `sear.version()`
- `sear.supported_operations()`

The main entry point would accept a Lua table describing a SEAR request and return a Lua table containing the parsed response. This keeps the interface consistent with the current JSON-centric design and avoids forcing Lua applications to manipulate raw JSON unless they want to.

## Packaging Approach with LuaRocks

### Rock layout

A proposed rock layout would be:

- `src/` for C/C++ binding sources
- `lua/` for Lua helper modules
- `examples/` for sample scripts
- `sear-lua-<version>.rockspec` for packaging metadata

### Build integration

LuaRocks packaging should integrate with the existing CMake-based build as much as possible. The preferred approach is:

- compile the Lua module as part of the native build
- expose the resulting shared object or loadable module to LuaRocks
- use the rockspec to describe include paths, link flags, and install locations

If direct CMake-to-LuaRocks integration is too complex for the initial release, a simpler two-step packaging model can be used:

1. build the native SEAR library and Lua binding
2. package the produced artifacts with LuaRocks

### Rockspec metadata

The rockspec should define:

- package name and version
- description and license
- source location or local build path
- dependencies on Lua and LuaRocks-compatible tooling
- build commands for CMake or native compilation
- installed modules and binary artifacts

## API Design Considerations

### Request mapping

Lua tables should map naturally to SEAR request fields:

- scalar fields become Lua strings, numbers, or booleans
- nested request sections become nested tables
- arrays become Lua arrays
- missing optional fields should be omitted rather than set to `nil`

### Response mapping

SEAR responses should be returned as Lua tables with:

- the parsed result payload
- operation status
- error information when applicable
- any diagnostic or warning metadata that the core already provides

### Error handling

Errors should be exposed in an idiomatic Lua form, such as:

- returning `nil, err`
- or raising a Lua error for unrecoverable binding issues

Operational errors from SEAR should remain distinguishable from binding or packaging errors.

## Deployment Model

### On z/OS

The interface should be optimized for z/OS environments where Lua is already in use for automation or glue logic. Packaging with LuaRocks would allow teams to install the module into a Lua environment without manually copying shared libraries and scripts.

### Versioning

The Lua package version should track SEAR core releases closely, with clear compatibility notes for each release.

### Distribution options

- local rock installation for site-specific use
- server/internal rock server distribution
- source rock for environments that prefer local compilation

## Testing Strategy

Testing should cover:

- Lua table to JSON translation
- JSON to Lua table translation
- error propagation
- module load/unload behavior
- basic smoke tests against SEAR operations
- packaging validation with LuaRocks

Where possible, tests should reuse existing SEAR test data and validation logic.

## Documentation Requirements

The Lua interface should ship with:

- installation instructions
- rockspec usage notes
- API reference
- example scripts
- compatibility notes for z/OS and Lua versions
- troubleshooting guidance for shared library loading and authorization failures

## Risks and Mitigations

### Native binding complexity

Binding a C/C++ library to Lua adds portability and build complexity.

Mitigation: keep the binding thin and expose only a small API initially.

### Packaging differences across z/OS environments

LuaRocks installations may differ across sites.

Mitigation: support both source-based and prebuilt installation flows.

### Error model mismatch

Lua and SEAR may report errors differently.

Mitigation: define a consistent error contract early and document it clearly.

## Phased Delivery Plan

### Phase 1: Prototype

- create a minimal Lua module
- expose one request entry point
- validate table-to-JSON conversion
- confirm loading through LuaRocks

### Phase 2: Initial release

- support core SEAR operations
- add error handling and examples
- publish a rockspec
- add automated tests

### Phase 3: Production hardening

- expand documentation
- improve packaging automation
- validate across supported z/OS levels
- add compatibility checks for Lua and LuaRocks versions

## Open Questions

- Should the Lua interface expose raw JSON as an alternative to Lua tables?
- Should the package be published as a source rock only, or also provide binary rocks for selected environments?
- Should the Lua API mirror the Python interface naming, or use Lua-specific conventions?
- Should the binding live in the main SEAR repository or in a separate repository maintained alongside SEAR?

## Recommendation

Start with a thin Lua binding around the existing SEAR native library and package it as a LuaRocks source rock. This keeps the first implementation small, avoids duplicating business logic, and provides a clear path to broader Lua adoption once the API and packaging model are proven.