/**
 * SEAR (Security Administration Request) - z/OS RACF Interface
 * TypeScript type definitions
 */

/**
 * Base result object from a SEAR operation
 */
export interface SearResultObject {
    [key: string]: any;
}

/**
 * SEAR request object
 */
export interface SearRequest {
    /**
     * Operation to perform: 'extract', 'search', 'alter', 'add', 'delete'
     */
    operation: 'extract' | 'search' | 'alter' | 'add' | 'delete';

    /**
     * Type of resource: canonical values are 'user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf'.
     * Legacy aliases 'connect' and 'permit' are also accepted for compatibility.
     */
    admin_type:
        | 'user'
        | 'group'
        | 'dataset'
        | 'group-connection'
        | 'permission'
        | 'keyring'
        | 'certificate'
        | 'resource'
        | 'racf-rrsf'
        | 'connect'
        | 'permit';

    /**
     * User ID (required for extract admin_type='user')
     */
    userid?: string;

    /**
     * Group ID (required for extract admin_type='group')
     */
    groupid?: string;

    /**
     * Dataset name (required for extract admin_type='dataset')
     */
    dataset?: string;

    /**
     * Keyring name (required for extract admin_type='keyring' or 'certificate')
     */
    keyring?: string;

    /**
     * Keyring owner user ID (required for extract admin_type='keyring' or 'certificate')
     */
    owner?: string;

    /**
     * Certificate label (optional for extract admin_type='certificate')
     */
    label?: string;

    /**
     * Resource name (required for extract admin_type='resource')
     */
    resource?: string;

    /**
     * Resource profile type (optional for extract admin_type='resource')
     */
    profile_type?: string;

    /**
     * Additional filter/criteria fields
     */
    [key: string]: any;
}

/**
 * Result from a SEAR operation
 */
export class SecurityResult {
    /**
     * The original request object
     */
    request: SearRequest;

    /**
     * Raw request buffer sent to RACF
     */
    raw_request: Buffer;

    /**
     * Raw result buffer from RACF
     */
    raw_result: Buffer;

    /**
     * Parsed result object
     */
    result: SearResultObject;

    constructor(options: {
        request: SearRequest;
        raw_request: Buffer;
        raw_result: Buffer;
        result: SearResultObject;
    });

    /**
     * Get the result as JSON string
     */
    toJSON(): string;

    /**
     * Check if the operation was successful
     */
    isSuccess(): boolean;
}

/**
 * Base error class for SEAR-related errors
 */
export class SearError extends Error {
    name: 'SearError';
    code: string;
    details?: Record<string, any>;
    constructor(message: string, code?: string);
}

/**
 * Validation error for invalid request objects
 */
export class ValidationError extends SearError {
    name: 'ValidationError';
    details: Record<string, any>;
    constructor(message: string, details?: Record<string, any>);
}

/**
 * Request error for operation failures
 */
export class RequestError extends SearError {
    name: 'RequestError';
    details: Record<string, any>;
    constructor(message: string, details?: Record<string, any>);
}

/**
 * Native binding error
 */
export class NativeError extends SearError {
    name: 'NativeError';
    details: Record<string, any>;
    constructor(message: string, details?: Record<string, any>);
}

/**
 * Execute a SEAR operation synchronously
 * @param request - The SEAR request object
 * @param debug - Enable debug output in native layer
 * @returns The operation result
 * @throws {ValidationError} if request is invalid
 * @throws {SearError} if operation fails
 *
 * @example
 * ```typescript
 * const result = sear({
 *   operation: 'extract',
 *   admin_type: 'user',
 *   userid: 'MYUSERID'
 * });
 * console.log(result.result);
 * ```
 */
export function sear(request: SearRequest, debug?: boolean): SecurityResult;

/**
 * Execute a SEAR operation asynchronously using worker thread
 * Prevents blocking the Node.js event loop for long-running operations
 * @param request - The SEAR request object
 * @param debug - Enable debug output
 * @returns Promise resolving to the operation result
 * @throws {ValidationError} if request is invalid
 * @throws {SearError} if operation fails
 *
 * @example
 * ```typescript
 * const result = await searAsync({
 *   operation: 'extract',
 *   admin_type: 'user',
 *   userid: 'MYUSERID'
 * });
 * ```
 */
export function searAsync(request: SearRequest, debug?: boolean): Promise<SecurityResult>;

/**
 * Valid operation types
 */
export const VALID_OPERATIONS: readonly ['extract', 'search', 'list', 'check', 'alter', 'add', 'delete'];

/**
 * Valid admin types
 */
export const VALID_ADMIN_TYPES: readonly ['user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf'];
