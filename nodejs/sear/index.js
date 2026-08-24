'use strict';

const path = require('path');
const { spawn, spawnSync } = require('child_process');
const {
    SearError,
    ValidationError,
    NativeError,
} = require('./errors');

let nativeModulePath;
let childModulePath;
try {
    nativeModulePath = require.resolve('../../build/Release/_sear.node');
    childModulePath = path.join(__dirname, 'sear.child.js');
} catch (error) {
    throw new NativeError(
        'Failed to load native SEAR binding. Ensure the addon is built: npm run build',
        { originalError: error.message }
    );
}

// ============================================================================
// Constants
// ============================================================================

const VALID_OPERATIONS = ['extract', 'search', 'alter', 'add', 'delete', 'remove', 'auth'];
const VALID_ADMIN_TYPES = [
    'user',
    'group',
    'dataset',
    'group-connection',
    'permission',
    'keyring',
    'certificate',
    'resource',
    'racf-rrsf',
    'racf-options',
];
const CHILD_OUTPUT_MAX_BYTES = 1024 * 1024 * 16;
const DUPLICATE_ADD_RESULT_TYPES = ['user', 'group', 'dataset', 'resource'];

// ============================================================================
// SecurityResult Class
// ============================================================================

/**
 * Represents the result of a SEAR operation
 * @class SecurityResult
 * @property {Object} request - The original request object
 * @property {Buffer} raw_request - The raw request buffer sent to RACF
 * @property {Buffer} raw_result - The raw result buffer from RACF
 * @property {Object} result - Parsed result object
 */
class SecurityResult {
    constructor({ request, raw_request, raw_result, result }) {
        this.request = request;
        this.raw_request = raw_request;  // Buffer
        this.raw_result = raw_result;    // Buffer
        this.result = result;            // parsed object
    }

    /**
     * Get the result as JSON string
     * @returns {string}
     */
    toJSON() {
        return JSON.stringify({
            request: this.request,
            result: this.result,
        });
    }

    /**
     * Check if the operation was successful
     * @returns {boolean}
     */
    isSuccess() {
        return this.result && !this.result.error;
    }
}

// ============================================================================
// Input Validation
// ============================================================================

/**
 * Validate a request object
 * @private
 * @param {Object} request - The request to validate
 * @throws {ValidationError} if request is invalid
 */
function validateRequest(request) {
    const errors = [];

    if (!request || typeof request !== 'object') {
        throw new ValidationError('Request must be a non-null object');
    }

    if (!request.operation) {
        errors.push('operation is required');
    } else if (!VALID_OPERATIONS.includes(request.operation)) {
        errors.push(`operation must be one of: ${VALID_OPERATIONS.join(', ')}`);
    }

    if (!request.admin_type) {
        errors.push('admin_type is required');
    } else if (!VALID_ADMIN_TYPES.includes(request.admin_type)) {
        errors.push(`admin_type must be one of: ${VALID_ADMIN_TYPES.join(', ')}`);
    }

    if (request.admin_type === 'racf-options' &&
        request.operation &&
        !['extract', 'alter'].includes(request.operation)) {
        errors.push('racf-options only supports extract and alter operations');
    }

    if (request.admin_type === 'permission' && request.operation === 'extract') {
        errors.push('permission extraction is not supported');
    }

    if (request.operation === 'auth' && !['dataset', 'resource'].includes(request.admin_type)) {
        errors.push('auth only supports dataset and resource admin types');
    }

    if (Object.prototype.hasOwnProperty.call(request, 'resource_class')) {
        errors.push('class_name must be used instead of resource_class');
    }

    if (Object.prototype.hasOwnProperty.call(request, 'class')) {
        errors.push('class_name must be used instead of class');
    }

    if (request.admin_type === 'resource' && ['add', 'alter', 'delete', 'auth'].includes(request.operation)) {
        if (!request.resource) {
            errors.push(`resource is required for resource ${request.operation}`);
        }
        if (!request.class_name) {
            errors.push(`class_name is required for resource ${request.operation}`);
        }
        if (request.operation === 'alter' && !request.traits) {
            errors.push('traits is required for resource alteration');
        }
    }

    if (request.operation === 'auth') {
        if (request.admin_type === 'dataset' && !request.dataset) {
            errors.push('dataset is required for dataset auth');
        }
        if (!request.access) {
            errors.push('access is required for auth');
        } else if (!['READ', 'read', 'UPDATE', 'update', 'CONTROL', 'control', 'ALTER', 'alter'].includes(request.access)) {
            errors.push('access must be one of: READ, UPDATE, CONTROL, ALTER');
        }
    }

    // Operation-specific validation
    if (request.operation === 'extract') {
        if (request.admin_type === 'user' && !request.userid) {
            errors.push('userid is required for user extraction');
        }
        if (request.admin_type === 'group' && !request.group) {
            errors.push('group is required for group extraction');
        }
        if (request.admin_type === 'dataset' && !request.dataset) {
            errors.push('dataset is required for dataset extraction');
        }
        if (request.admin_type === 'resource' && !request.resource) {
            errors.push('resource is required for resource extraction');
        }
        if (request.admin_type === 'resource' && !request.class_name) {
            errors.push('class_name is required for resource extraction');
        }
        if (request.admin_type === 'keyring' && !request.keyring) {
            errors.push('keyring is required for keyring extraction');
        }
        if (request.admin_type === 'keyring' && !request.owner) {
            errors.push('owner is required for keyring extraction');
        }
        if (request.admin_type === 'certificate') {
            errors.push('certificate extraction is not supported');
        }
    } else if (['alter', 'delete'].includes(request.operation) && request.admin_type === 'permission') {
        if (!request.dataset && !request.resource) {
            errors.push(`either dataset or resource is required for permission ${request.operation}`);
        }
        if (request.resource && !request.class_name) {
            errors.push(`class_name is required for resource permission ${request.operation}`);
        }
        if (!request.userid && !request.group) {
            errors.push(`either userid or group is required for permission ${request.operation}`);
        }
        if (request.operation === 'alter' && !request.traits) {
            errors.push('traits is required for permission alteration');
        }
    } else if (['add', 'delete', 'remove'].includes(request.operation) && request.admin_type === 'certificate') {
        if (!request.owner) {
            errors.push(`owner is required for certificate ${request.operation}`);
        }
        if (!request.keyring) {
            errors.push(`keyring is required for certificate ${request.operation}`);
        }
        if (!request.keyring_owner) {
            errors.push(`keyring_owner is required for certificate ${request.operation}`);
        }
        if (!request.label) {
            errors.push(`label is required for certificate ${request.operation}`);
        }
        if (request.operation === 'add' && !request.usage) {
            errors.push('usage is required for certificate add');
        }
        if (request.operation === 'add' && !request.status) {
            errors.push('status is required for certificate add');
        }
    }

    if (errors.length > 0) {
        throw new ValidationError(
            `Invalid request: ${errors.join('; ')}`,
            { errors }
        );
    }
}

function prepareRequest(request) {
    validateRequest(request);

    const nativeRequest = { ...request };
    if (nativeRequest.class_name) {
        nativeRequest.class = nativeRequest.class_name;
    }
    delete nativeRequest.class_name;

    return {
        request,
        requestJson: JSON.stringify(nativeRequest),
    };
}

function buildSecurityResult(request, response) {
    if (!response || typeof response !== 'object') {
        throw new NativeError('Invalid response from native binding');
    }

    return new SecurityResult({
        request,
        raw_request: response.raw_request,
        raw_result: response.raw_result,
        result: response.result_json ? JSON.parse(response.result_json) : {},
    });
}

function isDuplicateAddRequest(request) {
    return request.operation === 'add' &&
    DUPLICATE_ADD_RESULT_TYPES.includes(request.admin_type);
}

function buildDuplicateAddResult(request) {
    let profileName;
    let errorMessage;

    if (request.admin_type === 'user') {
        profileName = request.userid;
    } else if (request.admin_type === 'group') {
        profileName = request.group;
    } else if (request.admin_type === 'dataset') {
        profileName = request.dataset;
    } else if (request.admin_type === 'resource') {
        profileName = request.resource;
        const className = request.class_name;
        errorMessage = `sear: unable to add '${profileName}' in the ` +
            `'${className}' class because a '${request.admin_type}' ` +
            `profile already exists in the '${className}' class with ` +
            'that name';
    }

    if (!errorMessage) {
        errorMessage = `sear: unable to add '${profileName}' because a ` +
            `'${request.admin_type}' profile already exists with that name`;
    }

    return {
        raw_request: Buffer.alloc(0),
        raw_result: Buffer.alloc(0),
        result_json: JSON.stringify({
            errors: [errorMessage],
            return_codes: {
                saf_return_code: null,
                racf_return_code: null,
                racf_reason_code: null,
                sear_return_code: 4,
            },
        }),
    };
}

function callSearInChild(preparedRequest, debug) {
    const child = spawnSync(process.execPath, [
        childModulePath,
        nativeModulePath,
        String(debug),
    ], {
        encoding: 'utf8',
        input: preparedRequest.requestJson,
        maxBuffer: CHILD_OUTPUT_MAX_BYTES,
        stdio: ['pipe', 'inherit', 'inherit', 'pipe'],
    });

    if (child.status === 0 && child.output[3]) {
        const response = JSON.parse(child.output[3]);
        return {
            raw_request: Buffer.from(response.raw_request, 'base64'),
            raw_result: Buffer.from(response.raw_result, 'base64'),
            result_json: response.result_json,
        };
    }

    if (child.signal) {
        if (debug) {
            console.error(`SEAR child process exited with signal ${child.signal}`);
        }
        if (isDuplicateAddRequest(preparedRequest.request)) {
            return buildDuplicateAddResult(preparedRequest.request);
        }
    }

    throw new NativeError(
        `SEAR child process failed${
            child.signal ? ` with signal ${child.signal}` : ''
        }`,
        { operation: preparedRequest.request.operation, status: child.status }
    );
}

function callSearInChildAsync(preparedRequest, debug) {
    return new Promise((resolve, reject) => {
        const child = spawn(process.execPath, [
            childModulePath,
            nativeModulePath,
            String(debug),
        ], {
            stdio: ['pipe', 'inherit', 'inherit', 'pipe'],
        });

        const responseChunks = [];
        let responseLength = 0;
        let isSettled = false;

        function settle(settleFn, value) {
            if (isSettled) {
                return;
            }
            isSettled = true;
            settleFn(value);
        }

        child.stdio[3].on('data', (chunk) => {
            responseLength += chunk.length;
            if (responseLength > CHILD_OUTPUT_MAX_BYTES) {
                child.kill();
                settle(reject, new NativeError('SEAR child process response exceeded maximum size', {
                    operation: preparedRequest.request.operation,
                }));
                return;
            }
            responseChunks.push(chunk);
        });

        child.stdin.on('error', () => {
            // The child exit handler reports the actual failure.
        });

        child.stdin.end(preparedRequest.requestJson);

        child.on('error', (error) => {
            settle(reject, new NativeError(`Failed to start SEAR child process: ${error.message}`, {
                operation: preparedRequest.request.operation,
            }));
        });

        child.on('close', (status, signal) => {
            if (status === 0 && responseChunks.length > 0) {
                try {
                    const response = JSON.parse(Buffer.concat(responseChunks).toString('utf8'));
                    settle(resolve, {
                        raw_request: Buffer.from(response.raw_request, 'base64'),
                        raw_result: Buffer.from(response.raw_result, 'base64'),
                        result_json: response.result_json,
                    });
                } catch (error) {
                    settle(reject, new NativeError(`Failed to parse SEAR child process response: ${error.message}`, {
                        operation: preparedRequest.request.operation,
                    }));
                }
                return;
            }

            if (signal) {
                if (debug) {
                    console.error(`SEAR child process exited with signal ${signal}`);
                }
                if (isDuplicateAddRequest(preparedRequest.request)) {
                    settle(resolve, buildDuplicateAddResult(preparedRequest.request));
                    return;
                }
            }

            settle(reject, new NativeError(
                `SEAR child process failed${signal ? ` with signal ${signal}` : ''}`,
                { operation: preparedRequest.request.operation, status }
            ));
        });
    });
}

// ============================================================================
// Core Functions
// ============================================================================

/**
 * Execute a SEAR operation synchronously
 * @param {Object} request - The SEAR request object
 * @param {string} request.operation - Operation type: 'extract', 'search', 'alter', 'add', 'delete', or 'remove'
 * @param {string} request.admin_type - Admin type: 'user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf', or 'racf-options'
 * @param {boolean} [debug=false] - Enable debug output in native layer
 * @returns {SecurityResult} The operation result
 * @throws {ValidationError} if request is invalid
 * @throws {SearError} if operation fails
 * @example
 * const result = sear({
 *   operation: 'extract',
 *   admin_type: 'user',
 *   userid: 'MYUSERID'
 * });
 * console.log(result.result);
 */
function sear(request, debug = false) {
    const preparedRequest = prepareRequest(request);

    try {
        const response = callSearInChild(preparedRequest, debug);
        return buildSecurityResult(preparedRequest.request, response);
    } catch (error) {
        if (error instanceof SearError) {
            throw error;
        }
        throw new NativeError(
            `Failed to execute SEAR operation: ${error.message}`,
            { operation: preparedRequest.request.operation, admin_type: preparedRequest.request.admin_type }
        );
    }
}

/**
 * Execute a SEAR operation asynchronously
 * @param {Object} request - The SEAR request object (see sear() for properties)
 * @param {boolean} [debug=false] - Enable debug output
 * @returns {Promise<SecurityResult>} Promise resolving to the operation result
 * @throws {ValidationError} if request is invalid
 * @throws {SearError} if operation fails
 * @example
 * const result = await searAsync({
 *   operation: 'extract',
 *   admin_type: 'user',
 *   userid: 'MYUSERID'
 * });
 */
async function searAsync(request, debug = false) {
    const preparedRequest = prepareRequest(request);

    return buildSecurityResult(
        preparedRequest.request,
        await callSearInChildAsync(preparedRequest, debug)
    );
}

// ============================================================================
// Exports
// ============================================================================

module.exports = {
    // Core API
    sear,
    searAsync,
    SecurityResult,

    // Error classes
    SearError,
    ValidationError,
    NativeError,

    // Constants
    VALID_OPERATIONS,
    VALID_ADMIN_TYPES,
};
