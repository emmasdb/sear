'use strict';

const path = require('path');
const { Worker } = require('worker_threads');
const {
    SearError,
    ValidationError,
    NativeError,
} = require('./errors');

let _C;
let nativeModulePath;
let workerModulePath;
try {
    nativeModulePath = require.resolve('../../build/Release/_sear.node');
    workerModulePath = path.join(__dirname, 'sear.worker.js');
    _C = require(nativeModulePath);
} catch (error) {
    throw new NativeError(
        'Failed to load native SEAR binding. Ensure the addon is built: npm run build',
        { originalError: error.message }
    );
}

// ============================================================================
// Constants
// ============================================================================

const VALID_OPERATIONS = ['extract', 'search', 'alter', 'add', 'delete'];
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
const DEFAULT_ASYNC_TIMEOUT_MS = 60000;

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
        if (request.admin_type === 'group' && !request.group) {
            errors.push('group is required for group extraction');
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
        if (!request.userid && !request.group) {
            errors.push('either userid or group is required for permission alteration');
        }
    }

    if (errors.length > 0) {
        throw new ValidationError(
            `Invalid request: ${errors.join('; ')}`,
            { errors, request }
        );
    }
}

function prepareRequest(request) {
    validateRequest(request);

    return {
        request,
        requestJson: JSON.stringify(request),
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

// ============================================================================
// Core Functions
// ============================================================================

/**
 * Execute a SEAR operation synchronously
 * @param {Object} request - The SEAR request object
 * @param {string} request.operation - Operation type: 'extract', 'search', 'alter', 'add', or 'delete'
 * @param {string} request.admin_type - Admin type: 'user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', or 'racf-rrsf'
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
        const response = _C.call_sear(preparedRequest.requestJson, debug);
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
    const preparedRequest = prepareRequest(request);

    return new Promise((resolve, reject) => {
        let isSettled = false;

        function settle(settleFn, value) {
            if (isSettled) {
                return;
            }
            isSettled = true;
            settleFn(value);
        }

        try {
            const worker = new Worker(workerModulePath, {
                workerData: { nativeModulePath },
            });

            const timeout = setTimeout(() => {
                worker.terminate();
                settle(reject, new SearError('SEAR operation timeout'));
            }, DEFAULT_ASYNC_TIMEOUT_MS);

            worker.on('message', (message) => {
                clearTimeout(timeout);
                worker.terminate();

                if (message.success) {
                    try {
                        settle(resolve, buildSecurityResult(preparedRequest.request, message.response));
                    } catch (error) {
                        settle(reject, new NativeError('Failed to parse native response', {
                            error: error.message,
                        }));
                    }
                } else {
                    settle(reject, new NativeError(
                        `Worker operation failed: ${message.error}`,
                        { operation: preparedRequest.request.operation }
                    ));
                }
            });

            worker.on('error', (error) => {
                clearTimeout(timeout);
                settle(reject, new NativeError(
                    `Worker error: ${error.message}`,
                    { operation: preparedRequest.request.operation }
                ));
            });

            worker.on('exit', (code) => {
                if (isSettled || code === 0) {
                    return;
                }

                clearTimeout(timeout);
                settle(reject, new NativeError(
                    `Worker exited with code ${code}`,
                    { operation: preparedRequest.request.operation }
                ));
            });

            worker.postMessage({
                request: preparedRequest.requestJson,
                debug,
            });
        } catch (error) {
            settle(reject, new NativeError(
                `Failed to create worker: ${error.message}`,
                { operation: preparedRequest.request.operation }
            ));
        }
    });
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
