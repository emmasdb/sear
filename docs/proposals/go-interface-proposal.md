# Proposal: Go Interface for SEAR

## Summary

This proposal adds a first-class Go interface to SEAR that follows the same core pattern used by the Python interface:

1. A thin native bridge that calls `sear()` and returns:
   - `raw_request`
   - `raw_result`
   - `result_json`
2. A language-level wrapper that:
   - accepts a structured request object or JSON payload,
   - calls the native bridge,
   - parses `result_json`,
   - returns a `SecurityResult` value.

The objective is API parity with existing SEAR capabilities while keeping implementation minimal, predictable, and thread-safe.

## Goals

- Provide a Go package for calling SEAR with a simple, idiomatic API.
- Mirror the Python result contract (`request`, `raw_request`, `raw_result`, parsed `result`).
- Preserve SEAR JSON schema compatibility and operation coverage.
- Keep the native layer small and easy to maintain.
- Support build and packaging on z/OS with IBM Open Enterprise SDK for Go.

## Non-goals (Phase 1)

- Re-implementing request validation logic in full Go parity with Node.js.
- Replacing existing Python or Node.js interfaces.
- Introducing asynchronous streaming APIs.

## Proposed API

### Package

- Module path: `github.com/Mainframe-Renewal-Project/sear/go/sear`
- Primary package name: `sear`

### Public types

```go
package sear

type SecurityResult struct {
    Request    map[string]any
    RawRequest []byte
    RawResult  []byte
    Result     map[string]any
}

type NativeResponse struct {
    RawRequest []byte
    RawResult  []byte
    ResultJSON []byte
}
```

### Public functions

```go
func Sear(request map[string]any, debug bool) (*SecurityResult, error)
func SearJSON(requestJSON []byte, debug bool) (*SecurityResult, error)
func CallSear(requestJSON []byte, debug bool) (*NativeResponse, error)
```

Behavior:

- `CallSear` is the low-level function closest to current `call_sear` behavior.
- `Sear` marshals request to JSON, calls `CallSear`, parses `ResultJSON`, and returns `SecurityResult`.
- `SearJSON` skips request marshaling and is useful for callers already producing JSON.

## Architecture

### Layering

1. **Core C/C++ layer (existing):** `sear()` in `sear/sear.h`.
2. **Go native bridge (new):** cgo boundary in `go/sear/native` calling `sear()`.
3. **Go wrapper (new):** user-facing package in `go/sear`.

### Thread safety

SEAR uses static `sear_result_t` state, and existing Python/Node bridges protect calls with a process-level mutex.

The Go bridge should do the same:

- C-level mutex (`pthread_mutex_t`) or
- Go-level `sync.Mutex` wrapping all cgo calls.

Recommendation: use Go `sync.Mutex` in the Go bridge to keep synchronization policy visible in Go and avoid duplicated lock logic.

### Memory management

- The bridge copies `raw_request`, `raw_result`, and `result_json` into Go-owned memory before returning.
- No pointers into static SEAR buffers should escape the cgo boundary.
- Inputs are passed as `[]byte` / `string` converted to C memory and freed immediately after call.

## Repository layout

Proposed new files and folders:

```text
go/
  go.mod
  sear/
    sear.go            # Public API (Sear, SearJSON, types)
    native.go          # cgo bindings and mutex-protected CallSear
    errors.go          # Typed errors
    doc.go
  examples/
    add_user/main.go
    extract_user/main.go
```

Optional (if split preferred):

```text
sear/go/_sear.c        # Native helper shim matching Python's call_sear style
```

## Build and linking

Two compatible options are possible:

1. **cgo compiles directly against SEAR sources** (faster bootstrap, less CMake wiring).
2. **cgo links against CMake-built `sear` library** (cleaner reuse of existing build graph).

Recommendation:

- Phase 1: link against existing CMake-built `sear` target to avoid source duplication.
- Add a CMake option `SEAR_ENABLE_GO` similar to `SEAR_ENABLE_PYTHON`.

Example CMake additions:

- Build shared/static library artifacts consumable by cgo.
- Export include paths for `sear.h`.
- Add install rule for Go artifacts only when enabled.

## Error model

Define typed Go errors while preserving SEAR payload details:

- `type NativeError struct { Message string }`
- `type JSONError struct { Cause error }`
- `type RequestError struct { Cause error }`

Rules:

- cgo or bridge failures return `NativeError`.
- invalid request marshal/unmarshal issues return `RequestError`/`JSONError`.
- SEAR semantic errors remain in returned `Result` content (same as Python behavior), not converted to Go exceptions.

## Testing strategy

### Unit tests (Go)

- JSON marshaling/parsing behavior.
- `SecurityResult` construction.
- error wrapping behavior.
- mutex serialization behavior (parallel goroutine call test with mocked native layer).

### Integration tests

- Mirror representative `python_tests` scenarios:
  - extract/search happy paths,
  - missing required fields,
  - operation/admin_type combinations.
- Validate parity of `result.return_codes` shape.

### CI

- Add Go lint/test jobs where Go toolchain is available.
- Keep z/OS-specific integration tests gated (manual or environment-tagged) until runners are available.

## Documentation updates

Update:

- `README.md`: add Go quick-start section next to Python usage.
- Build instructions: include `SEAR_ENABLE_GO` flow.
- Examples docs: minimal create/extract samples in Go.

## Rollout plan

### Phase 0: Design approval

- Approve API shape (`Sear`, `SearJSON`, `CallSear`).
- Approve build approach (CMake-linked cgo).

### Phase 1: Minimal viable binding

- Implement cgo bridge + mutex.
- Implement `SearJSON` and `CallSear`.
- Add smoke tests and one end-to-end example.

### Phase 2: Idiomatic wrapper parity

- Implement `Sear` typed wrapper.
- Add typed errors and docs.
- Add more integration tests aligned to `python_tests` coverage.

### Phase 3: Packaging and release

- Add tagged Go module release process.
- Add CI jobs and compatibility matrix notes.

## Risks and mitigations

- **z/OS Go + cgo toolchain variability:** document tested compiler/runtime combinations; gate CI by environment.
- **Static result buffer concurrency:** enforce mutex around all native calls.
- **Schema drift across bindings:** reuse existing JSON contract; add cross-language parity tests.
- **Build complexity:** start with minimal CMake integration and avoid introducing a second parallel build graph.

## Acceptance criteria

- Go caller can execute at least one `add` and one `extract` operation end-to-end.
- Returned object contains `RawRequest`, `RawResult`, and parsed `Result`.
- Parallel goroutine calls are safe and deterministic.
- README contains install/build/run example for Go.
- CI validates Go unit tests on supported platforms.

## Open questions

- Should Phase 1 include a typed request builder, or stay map-based for parity and speed?
- Is Go module versioning tied to SEAR release tags or published independently?
- Should Node-style pre-validation be added later, or delegated entirely to SEAR schema validation?
