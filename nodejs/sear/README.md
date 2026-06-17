# SEAR.js - z/OS RACF Security Administration Interface

A production-grade Node.js wrapper for SEAR (Security Administration Request) library, providing a unified interface to z/OS RACF callable services.

## Installation

```bash
npm install
npm run build
```

## Quick Start

### Basic Usage

```javascript
const { sear, extractUser } = require('./nodejs/sear');

// Method 1: Direct object
const result = sear({
  operation: 'extract',
  admin_type: 'user',
  userid: 'MYUSER'
});
console.log(result.result);

// Method 2: Using builder functions
const result = sear(extractUser('MYUSER'));
console.log(result.result);
```

### Error Handling

```javascript
const { 
  sear, 
  extractUser,
  ValidationError, 
  SearError 
} = require('./nodejs/sear');

try {
  const result = sear(extractUser('MYUSER'));
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
const { searAsync, extractUser } = require('./nodejs/sear');

async function getUser(userid) {
  try {
    const result = await searAsync(extractUser(userid));
    return result.result;
  } catch (error) {
    console.error('Failed to fetch user:', error.message);
    throw error;
  }
}

// Usage
const user = await getUser('MYUSER');
```

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
- **`RequestError`**: Operation-level failure
- **`NativeError`**: Native binding failure

### Request Builders

Helper functions to construct valid request objects:

#### `extractUser(userid): SearRequest`
Extract a user by ID.

#### `extractGroup(groupid): SearRequest`
Extract a group by ID.

#### `extractDataset(dataset): SearRequest`
Extract a dataset by name.

#### `searchUsers(filter={}): SearRequest`
Search for users matching filter criteria.

**Example:**
```javascript
const result = sear(searchUsers({ prefix: 'J' }));
```

#### `searchGroups(filter={}): SearRequest`
Search for groups matching filter criteria.

#### `listResources(admin_type): SearRequest`
List all resources of a given type.

**Example:**
```javascript
const result = sear(listResources('user'));
```

#### `extractKeyring(keyring, owner): SearRequest`
Extract a keyring by name and owner.

**Example:**
```javascript
const result = sear(extractKeyring('MYKEYRING', 'KEYRING_OWNER'));
```

#### `extractCertificate(keyring, owner, label?): SearRequest`
Extract a certificate from a keyring.

**Example:**
```javascript
const result = sear(extractCertificate('MYKEYRING', 'KEYRING_OWNER', 'MYCERT'));
```

#### `extractRRSF(): SearRequest`
Extract RACF RRSF (Resource Set, Function-based) information.

**Example:**
```javascript
const result = sear(extractRRSF());
```

#### `extractResource(resource, profile_type?): SearRequest`
Extract a resource by name.

**Example:**
```javascript
const result = sear(extractResource('RESOURCE1'));
```

#### `groupConnection(criteria): SearRequest`
Build a group connection request with user and group information.

**Example:**
```javascript
const result = sear(groupConnection({ userid: 'USER1', groupid: 'GROUP1' }));
```

#### `alterPermission(criteria): SearRequest`
Build a permission alteration request (grant/revoke access to datasets or resources).

**Example:**
```javascript
const result = sear(alterPermission({
    operation: 'alter',
    dataset: 'PROD.DATA',
    userid: 'USER1',
    generic: true,
    traits: { 'base:access': 'READ' }
}));
```

### Request Object

Base structure for all SEAR requests:

```typescript
{
  operation: 'extract' | 'search' | 'list' | 'check' | 'alter' | 'add' | 'delete',
  admin_type: 'user' | 'group' | 'dataset' | 'keyring' | 'certificate' | 'resource' | 'group-connection' | 'permission' | 'racf-rrsf',
  // Additional fields based on operation and admin_type
}
```

**Supported Admin Types:**

| Type | Purpose | Extract Fields |
|------|---------|-----------------|
| `'user'` | RACF user profiles | `userid` |
| `'group'` | RACF group profiles | `groupid` |
| `'dataset'` | z/OS dataset profiles | `dataset` |
| `'keyring'` | RACF keyrings | `keyring`, `owner` |
| `'certificate'` | Certificates in keyrings | `keyring`, `owner`, `label` (optional) |
| `'resource'` | RACF resource profiles | `resource`, `profile_type` (optional) |
| `'racf-rrsf'` | RACF RRSF (Resource Set, Function-based) | (none) |
| `'group-connection'` | User-group connections | `userid`, `groupid` |
| `'permission'` | Resource permissions | `dataset`/`resource`, `userid`/`groupid`, `traits` |

**Operation-specific requirements:**

| Operation | Admin Type | Required Fields |
|-----------|-----------|-----------------|
| extract | user | userid |
| extract | group | groupid |
| extract | dataset | dataset |
| extract | keyring | keyring, owner |
| extract | certificate | keyring, owner |
| extract | resource | resource |
| extract | racf-rrsf | (none additional) |
| search | any | (varies by filter) |
| list | any | (none additional) |
| alter | permission | dataset/resource, userid/groupid, traits |
| check | any | (criteria-specific) |

## Examples

### Extract User Information

```javascript
const { sear, extractUser } = require('./nodejs/sear');

const result = sear(extractUser('FDEGILIO'));
console.log(JSON.stringify(result.result, null, 2));
```

### Search for Users

```javascript
const { sear, searchUsers } = require('./nodejs/sear');

const result = sear(searchUsers({ prefix: 'A' }));
console.log(`Found ${result.result.users?.length || 0} users starting with A`);
```

### Extract Keyring Information

```javascript
const { sear, extractKeyring } = require('./nodejs/sear');

const result = sear(extractKeyring('MYKEYRING', 'KEYRING_OWNER'));
console.log(JSON.stringify(result.result, null, 2));
```

### Extract Certificate from Keyring

```javascript
const { sear, extractCertificate } = require('./nodejs/sear');

const result = sear(extractCertificate('MYKEYRING', 'KEYRING_OWNER', 'CERT_LABEL'));
console.log(JSON.stringify(result.result, null, 2));
```

### Grant Dataset Permission

```javascript
const { sear, alterPermission } = require('./nodejs/sear');

const result = sear(alterPermission({
    operation: 'alter',
    dataset: 'PROD.DATA',
    userid: 'NEWUSER',
    generic: true,
    traits: { 'base:access': 'READ' }
}));

if (result.isSuccess()) {
    console.log('Permission granted');
} else {
    console.error('Failed to grant permission:', result.result.error);
}
```

### Batch Processing with Async

```javascript
const { searAsync, extractUser } = require('./nodejs/sear');

async function batchExtract(userids) {
  const promises = userids.map(id => 
    searAsync(extractUser(id)).catch(err => ({ error: err.message }))
  );
  return Promise.all(promises);
}

const results = await batchExtract(['USER1', 'USER2', 'USER3']);
```

### TypeScript Usage

With TypeScript support (index.d.ts provided):

```typescript
import { sear, extractUser, ValidationError, SearRequest } from './nodejs/sear';

const request: SearRequest = extractUser('MYUSER');
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

- Node.js v14+ on z/OS (os390/s390x)
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
