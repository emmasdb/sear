# SEAR.js - z/OS RACF Security Administration Interface

A Node.js wrapper for SEAR (Security Administration Request) library, providing a unified interface to z/OS RACF callable services.

## Installation

```bash
npm install
npm run build
```

## Quick Start

### Basic Usage

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'extract',
  admin_type: 'user',
  userid: 'MYUSER'
});
console.log(result.result);
```

### Error Handling

```javascript
const { 
  sear, 
  ValidationError, 
  SearError 
} = require('./nodejs/sear');

try {
  const result = sear({
    operation: 'extract',
    admin_type: 'user',
    userid: 'MYUSER'
  });
  if (result.isSuccess()) {
    console.log('Success:', result.result);
  } else {
    console.error('Operation failed:', result.result.error);
  }
} catch (error) {
  if (error instanceof ValidationError) {
    console.error('Invalid request:', error.details.errors);
  } else if (error instanceof SearError) {
    console.error('SEAR error:', error.message);
  } else {
    console.error('Unexpected error:', error.message);
  }
}
```

### Async Operations

For operations that might take time, use `searAsync()` to prevent blocking the event loop:

```javascript
const { searAsync } = require('./nodejs/sear');

async function getUser(userid) {
  try {
    const result = await searAsync({
      operation: 'extract',
      admin_type: 'user',
      userid,
    });
    return result.result;
  } catch (error) {
    console.error('Failed to fetch user:', error.message);
    throw error;
  }
}

// Usage
const user = await getUser('MYUSER');
```

> **Note on z/OS:** The synchronous `sear()` function is recommended for z/OS deployments. Worker threads may not have the proper security context needed for RACF system calls. Use `searAsync()` only if your deployment requires non-blocking I/O.

## API Reference

### Core Functions

#### `sear(request, debug=false): SecurityResult`

Execute a SEAR operation synchronously.

**Parameters:**

- `request` (SearRequest): The operation request with `operation`, `admin_type`, and operation-specific fields
- `debug` (boolean): Enable debug output in the native layer

**Returns:** `SecurityResult` object with `request`, `raw_request`, `raw_result`, and `result` properties

**Throws:**

- `ValidationError`: Request validation failed
- `SearError`: Operation failed

#### `searAsync(request, debug=false): Promise<SecurityResult>`

Execute a SEAR operation asynchronously using a worker thread. Prevents event loop blocking.

**Parameters:** Same as `sear()`

**Returns:** Promise resolving to `SecurityResult`

**Throws:** Same as `sear()`

### SecurityResult

Represents the result of a SEAR operation.

**Properties:**

- `request`: The original request object
- `raw_request`: Buffer containing raw request data sent to RACF
- `raw_result`: Buffer containing raw response from RACF
- `result`: Parsed result object

**Methods:**

- `toJSON()`: Returns JSON string representation
- `isSuccess()`: Returns boolean indicating success status

### Error Classes

All error classes extend `SearError` with detailed error information in `.details` property:

- **`ValidationError`**: Invalid request (missing required fields, invalid operation/admin_type)
- **`NativeError`**: Native binding failure

### Request Shape

The Node.js interface follows the Python interface: construct a plain request object and pass it to `sear()` or `searAsync()`.

### Request Object

Base structure for all SEAR requests:

```typescript
{
  operation: 'extract' | 'search' | 'alter' | 'add' | 'delete',
  admin_type: 'user' | 'group' | 'dataset' | 'keyring' | 'certificate' | 'resource' | 'group-connection' | 'permission' | 'racf-rrsf',
  // Additional fields based on operation and admin_type
}
```

**Supported Admin Types:**

| Type | Purpose | Extract Fields |
| ---- | ------- | -------------- |
| `'user'` | RACF user profiles | `userid` |
| `'group'` | RACF group profiles | `group` |
| `'dataset'` | z/OS dataset profiles | `dataset` |
| `'keyring'` | RACF keyrings | `keyring`, `owner` |
| `'certificate'` | Certificates in keyrings | `keyring`, `owner`, `label` (optional) |
| `'resource'` | RACF resource profiles | `resource`, `class_name`, `profile_type` (optional) |
| `'racf-rrsf'` | RACF RRSF (Resource Set, Function-based) | (none) |
| `'group-connection'` | User-group connections | `userid`, `group` |
| `'permission'` | Resource permissions | `dataset`/`resource`, `userid`/`group`, `traits` |

**Operation-specific requirements:**

| Operation | Admin Type | Required Fields |
| --------- | ---------- | --------------- |
| extract | user | userid |
| extract | group | group |
| extract | dataset | dataset |
| extract | keyring | keyring, owner |
| extract | certificate | keyring, owner |
| extract | resource | resource, class_name |
| extract | racf-rrsf | (none additional) |
| search | any | (varies by filter) |
| alter | permission | dataset/resource, userid/group, traits |

## Examples

### Extract User Information

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'extract',
  admin_type: 'user',
  userid: 'FDEGILIO'
});
console.log(JSON.stringify(result.result, null, 2));
```

### Search for Users

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'search',
  admin_type: 'user',
  prefix: 'A'
});
console.log(`Found ${result.result.users?.length || 0} users starting with A`);
```

### Extract Keyring Information

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'extract',
  admin_type: 'keyring',
  keyring: 'MYKEYRING',
  owner: 'KEYRING_OWNER'
});
console.log(JSON.stringify(result.result, null, 2));
```

### Extract Certificate from Keyring

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'extract',
  admin_type: 'certificate',
  keyring: 'MYKEYRING',
  owner: 'KEYRING_OWNER',
  label: 'CERT_LABEL'
});
console.log(JSON.stringify(result.result, null, 2));
```

### Grant Dataset Permission

```javascript
const { sear } = require('./nodejs/sear');

const result = sear({
  operation: 'alter',
  admin_type: 'permission',
  dataset: 'PROD.DATA',
  userid: 'NEWUSER',
  generic: true,
  traits: { 'base:access': 'READ' }
});

if (result.isSuccess()) {
    console.log('Permission granted');
} else {
    console.error('Failed to grant permission:', result.result.error);
}
```

### Batch Processing with Async

```javascript
const { searAsync } = require('./nodejs/sear');

async function batchExtract(userids) {
  const promises = userids.map(id => 
    searAsync({
      operation: 'extract',
      admin_type: 'user',
      userid: id,
    }).catch(err => ({ error: err.message }))
  );
  return Promise.all(promises);
}

const results = await batchExtract(['USER1', 'USER2', 'USER3']);
```

### TypeScript Usage

With TypeScript support (index.d.ts provided):

```typescript
import { sear, ValidationError, SearRequest } from './nodejs/sear';

const request: SearRequest = {
  operation: 'extract',
  admin_type: 'user',
  userid: 'MYUSER',
};
const result = sear(request);
if (result.isSuccess()) {
  console.log(result.result);
}
```

## Development

### Running Tests

```bash
npm test
npm run smoke
```

### Building from Source

```bash
npm run prebuild
npm run build
```

## Platform Requirements

- Node.js v18+ on z/OS (os390)
- ibm-clang64/ibm-clang++64 compiler
- z/OS RACF security kernel

## Thread Safety

The native binding uses pthread mutexes for thread-safe access to RACF callable services. Both `sear()` and `searAsync()` are safe to call concurrently.

## Performance Considerations

- **Synchronous calls** (`sear()`): Lower latency, blocks the event loop
- **Async calls** (`searAsync()`): Higher latency (worker thread overhead), non-blocking
- Use `sear()` for quick operations in non-critical paths
- Use `searAsync()` for server applications where event loop blocking is unacceptable
- Batch operations when possible to minimize call overhead

## License

See [LICENSE](../../LICENSE) in the repository root.
