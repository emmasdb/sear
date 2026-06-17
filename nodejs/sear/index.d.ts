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
     * Operation to perform: 'extract', 'search', 'list', 'check', 'alter', 'add', 'delete'
     */
    operation: 'extract' | 'search' | 'alter' | 'add' | 'delete';

    /**
     * Type of resource: 'user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf'
     */
    admin_type: 'user' | 'group' | 'dataset' | 'group-connection' | 'permission' | 'keyring' | 'certificate' | 'resource' | 'racf-rrsf';

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
 * Build an extract request for a user
 * @param userid - The user ID to extract
 * @returns Complete request object
 *
 * @example
 * ```typescript
 * const result = sear(extractUser('MYUSER'));
 * ```
 */
export function extractUser(userid: string): SearRequest;

/**
 * Build an extract request for a group
 * @param groupid - The group ID to extract
 * @returns Complete request object
 */
export function extractGroup(groupid: string): SearRequest;

/**
 * Build an extract request for a dataset
 * @param dataset - The dataset name to extract
 * @returns Complete request object
 */
export function extractDataset(dataset: string): SearRequest;

/**
 * Build an extract request for a keyring
 * @param keyring - The keyring name to extract
 * @param owner - The keyring owner (user ID)
 * @returns Complete request object
 *
 * @example
 * ```typescript
 * const result = sear(extractKeyring('MYKEYRING', 'KEYRING_OWNER'));
 * ```
 */
export function extractKeyring(keyring: string, owner: string): SearRequest;

/**
 * Build an extract request for a certificate within a keyring
 * @param keyring - The keyring name
 * @param owner - The keyring owner (user ID)
 * @param label - Optional certificate label
 * @returns Complete request object
 */
export function extractCertificate(keyring: string, owner: string, label?: string): SearRequest;

/**
 * Build an extract request for RACF RRSF (Resource Set, Function-based)
 * @returns Complete request object
 *
 * @example
 * ```typescript
 * const result = sear(extractRRSF());
 * ```
 */
export function extractRRSF(): SearRequest;

/**
 * Build an extract request for a resource
 * @param resource - The resource name to extract
 * @param profile_type - Optional resource profile type
 * @returns Complete request object
 */
export function extractResource(resource: string, profile_type?: string): SearRequest;

/**
 * Build a search request for users matching a filter
 * @param filter - Filter criteria (e.g., { prefix: 'J' })
 * @returns Complete request object
 */
export function searchUsers(filter?: Record<string, any>): SearRequest;

/**
 * Build a search request for groups matching a filter
 * @param filter - Filter criteria
 * @returns Complete request object
 */
export function searchGroups(filter?: Record<string, any>): SearRequest;

/**
 * Build a list request for a resource type
 * @param admin_type - Type of resource to list
 * @returns Complete request object
 */
export function listResources(admin_type: string): SearRequest;

/**
 * Build a check request to verify permissions
 * @param criteria - Check criteria
 * @returns Complete request object
 */
export function checkPermission(criteria: Record<string, any>): SearRequest;

/**
 * Build a request for a group connection
 * @param criteria - Connection criteria (userid and groupid)
 * @returns Complete request object
 *
 * @example
 * ```typescript
 * const result = sear(groupConnection({ userid: 'USER1', groupid: 'GROUP1' }));
 * ```
 */
export function groupConnection(criteria: Record<string, any>): SearRequest;

/**
 * Build a permission alteration request (grant/revoke access)
 * @param criteria - Permission criteria with operation, dataset/resource, userid/groupid, and traits
 * @returns Complete request object
 *
 * @example
 * ```typescript
 * const result = sear(alterPermission({
 *   operation: 'alter',
 *   dataset: 'PROD.DATA',
 *   userid: 'USER1',
 *   generic: true,
 *   traits: { 'base:access': 'READ' }
 * }));
 * ```
 */
export function alterPermission(criteria: Record<string, any>): SearRequest;

/**
 * Valid operation types
 */
export const VALID_OPERATIONS: readonly ['extract', 'search', 'list', 'check', 'alter', 'add', 'delete'];

/**
 * Valid admin types
 */
export const VALID_ADMIN_TYPES: readonly ['user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf'];
