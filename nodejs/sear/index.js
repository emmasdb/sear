'use strict';

const { Worker } = require('worker_threads');
const {
    SearError,
    ValidationError,
    RequestError,
    NativeError,
} = require('./errors');

let _C;
try {
    _C = require('../../build/Release/_sear.node');
} catch (error) {
    throw new NativeError(
        'Failed to load native SEAR binding. Ensure the addon is built: npm run build',
        { originalError: error.message }
    );
}

// ============================================================================
// Constants
// ============================================================================

const VALID_OPERATIONS = ['extract', 'search', 'list', 'check', 'alter', 'add', 'delete'];
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
];

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

    // Operation-specific validation
    if (request.operation === 'extract') {
        if (request.admin_type === 'user' && !request.userid) {
            errors.push('userid is required for user extraction');
        }
        if (request.admin_type === 'group' && !request.groupid) {
            errors.push('groupid is required for group extraction');
        }
        if (request.admin_type === 'dataset' && !request.dataset) {
            errors.push('dataset is required for dataset extraction');
        }
        if (request.admin_type === 'keyring' && !request.keyring) {
            errors.push('keyring is required for keyring extraction');
        }
        if (request.admin_type === 'keyring' && !request.owner) {
            errors.push('owner is required for keyring extraction');
        }
        if (request.admin_type === 'certificate' && !request.keyring) {
            errors.push('keyring is required for certificate extraction');
        }
        if (request.admin_type === 'certificate' && !request.owner) {
            errors.push('owner is required for certificate extraction');
        }
    } else if (request.operation === 'alter' && request.admin_type === 'permission') {
        if (!request.dataset && !request.resource) {
            errors.push('either dataset or resource is required for permission alteration');
        }
        if (!request.userid && !request.groupid) {
            errors.push('either userid or groupid is required for permission alteration');
        }
    }

    if (errors.length > 0) {
        throw new ValidationError(
            `Invalid request: ${errors.join('; ')}`,
            { errors, request }
        );
    }
}

// ============================================================================
// Core Functions
// ============================================================================

/**
 * Execute a SEAR operation synchronously
 * @param {Object} request - The SEAR request object
 * @param {string} request.operation - Operation type: 'extract', 'search', 'list', 'check'
 * @param {string} request.admin_type - Admin type: 'user', 'group', 'dataset', 'connect', 'permit'
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
    try {
        validateRequest(request);
    } catch (error) {
        if (error instanceof ValidationError) {
            throw error;
        }
        throw new ValidationError('Request validation failed', { error: error.message });
    }

    try {
        const response = _C.call_sear(JSON.stringify(request), debug);

        if (!response || typeof response !== 'object') {
            throw new NativeError('Invalid response from native binding');
        }

        const result = new SecurityResult({
            request,
            raw_request: response.raw_request,
            raw_result: response.raw_result,
            result: response.result_json ? JSON.parse(response.result_json) : {},
        });

        return result;
    } catch (error) {
        if (error instanceof SearError) {
            throw error;
        }
        throw new NativeError(
            `Failed to execute SEAR operation: ${error.message}`,
            { operation: request.operation, admin_type: request.admin_type }
        );
    }
}

/**
 * Execute a SEAR operation asynchronously using worker thread
 * Prevents blocking the Node.js event loop for long-running operations
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
    validateRequest(request);

    return new Promise((resolve, reject) => {
        const workerCode = `
            const { parentPort } = require('worker_threads');
            const _C = require('../../build/Release/_sear.node');

            parentPort.on('message', (message) => {
                try {
                    const response = _C.call_sear(message.request, message.debug);
                    parentPort.postMessage({ success: true, response });
                } catch (error) {
                    parentPort.postMessage({ 
                        success: false, 
                        error: error.message 
                    });
                }
            });
        `;

        try {
            const worker = new Worker(workerCode, { eval: true });

            const timeout = setTimeout(() => {
                worker.terminate();
                reject(new SearError('SEAR operation timeout'));
            }, 60000); // 60 second timeout

            worker.on('message', (message) => {
                clearTimeout(timeout);
                worker.terminate();

                if (message.success) {
                    try {
                        const result = new SecurityResult({
                            request,
                            raw_request: message.response.raw_request,
                            raw_result: message.response.raw_result,
                            result: message.response.result_json
                                ? JSON.parse(message.response.result_json)
                                : {},
                        });
                        resolve(result);
                    } catch (error) {
                        reject(new NativeError('Failed to parse native response', {
                            error: error.message,
                        }));
                    }
                } else {
                    reject(new NativeError(
                        `Worker operation failed: ${message.error}`,
                        { operation: request.operation }
                    ));
                }
            });

            worker.on('error', (error) => {
                clearTimeout(timeout);
                reject(new NativeError(
                    `Worker error: ${error.message}`,
                    { operation: request.operation }
                ));
            });

            worker.postMessage({
                request: JSON.stringify(request),
                debug,
            });
        } catch (error) {
            reject(new NativeError(
                `Failed to create worker: ${error.message}`,
                { operation: request.operation }
            ));
        }
    });
}

// ============================================================================
// Request Builders - Factory Functions for Common Operations
// ============================================================================

/**
 * Build an extract request for a user
 * @param {string} userid - The user ID to extract
 * @returns {Object} Complete request object
 * @example
 * const result = sear(extractUser('MYUSER'));
 */
function extractUser(userid) {
    return {
        operation: 'extract',
        admin_type: 'user',
        userid,
    };
}

/**
 * Build an extract request for a group
 * @param {string} groupid - The group ID to extract
 * @returns {Object} Complete request object
 */
function extractGroup(groupid) {
    return {
        operation: 'extract',
        admin_type: 'group',
        groupid,
    };
}

/**
 * Build an extract request for a dataset
 * @param {string} dataset - The dataset name to extract
 * @returns {Object} Complete request object
 */
function extractDataset(dataset) {
    return {
        operation: 'extract',
        admin_type: 'dataset',
        dataset,
    };
}

/**
 * Build a search request for users matching a filter
 * @param {Object} filter - Filter criteria (e.g., { prefix: 'J' })
 * @returns {Object} Complete request object
 */
function searchUsers(filter = {}) {
    return {
        operation: 'search',
        admin_type: 'user',
        ...filter,
    };
}

/**
 * Build a search request for groups matching a filter
 * @param {Object} filter - Filter criteria
 * @returns {Object} Complete request object
 */
function searchGroups(filter = {}) {
    return {
        operation: 'search',
        admin_type: 'group',
        ...filter,
    };
}

/**
 * Build a list request for a resource type
 * @param {string} admin_type - Type of resource to list
 * @returns {Object} Complete request object
 */
function listResources(admin_type) {
    return {
        operation: 'list',
        admin_type,
    };
}

/**
 * Build a check request to verify permissions
 * @param {Object} criteria - Check criteria
 * @returns {Object} Complete request object
 */
function checkPermission(criteria) {
    return {
        operation: 'check',
        ...criteria,
    };
}

/**
 * Build an extract request for a keyring
 * @param {string} keyring - The keyring name to extract
 * @param {string} owner - The keyring owner (user ID)
 * @returns {Object} Complete request object
 * @example
 * const result = sear(extractKeyring('MYKEYRING', 'KEYRING_OWNER'));
 */
function extractKeyring(keyring, owner) {
    return {
        operation: 'extract',
        admin_type: 'keyring',
        keyring,
        owner,
    };
}

/**
 * Build an extract request for a certificate within a keyring
 * @param {string} keyring - The keyring name
 * @param {string} owner - The keyring owner (user ID)
 * @param {string} [label] - Optional certificate label
 * @returns {Object} Complete request object
 */
function extractCertificate(keyring, owner, label) {
    const req = {
        operation: 'extract',
        admin_type: 'certificate',
        keyring,
        owner,
    };
    if (label) {
        req.label = label;
    }
    return req;
}

/**
 * Build an extract request for RACF RRSF (Resource Set, Function-based)
 * @returns {Object} Complete request object
 * @example
 * const result = sear(extractRRSF());
 */
function extractRRSF() {
    return {
        operation: 'extract',
        admin_type: 'racf-rrsf',
    };
}

/**
 * Build an extract request for a resource
 * @param {string} resource - The resource name to extract
 * @param {string} [profile_type] - Optional resource profile type
 * @returns {Object} Complete request object
 */
function extractResource(resource, profile_type) {
    const req = {
        operation: 'extract',
        admin_type: 'resource',
        resource,
    };
    if (profile_type) {
        req.profile_type = profile_type;
    }
    return req;
}

/**
 * Build a request for a group connection
 * @param {Object} criteria - Connection criteria
 * @returns {Object} Complete request object
 * @example
 * const result = sear(groupConnection({ userid: 'USER1', groupid: 'GROUP1' }));
 */
function groupConnection(criteria) {
    return {
        ...criteria,
        admin_type: 'group-connection',
    };
}

/**
 * Build a permission alteration request (grant/revoke access)
 * @param {Object} criteria - Permission criteria with operation, dataset/resource, userid/groupid, and traits
 * @returns {Object} Complete request object
 * @example
 * const result = sear(alterPermission({
 *   operation: 'alter',
 *   dataset: 'PROD.DATA',
 *   userid: 'USER1',
 *   generic: true,
 *   traits: { 'base:access': 'READ' }
 * }));
 */
function alterPermission(criteria) {
    return {
        ...criteria,
        admin_type: 'permission',
    };
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
    RequestError,
    NativeError,

    // Request builders - Extract operations
    extractUser,
    extractGroup,
    extractDataset,
    extractKeyring,
    extractCertificate,
    extractRRSF,
    extractResource,

    // Request builders - Search/List operations
    searchUsers,
    searchGroups,
    listResources,

    // Request builders - Permission/Connection operations
    groupConnection,
    alterPermission,
    checkPermission,

    // Constants
    VALID_OPERATIONS,
    VALID_ADMIN_TYPES,
};
