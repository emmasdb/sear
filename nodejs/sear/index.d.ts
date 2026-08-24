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
 * Optional RACROUTE REQUEST=AUTH operands exposed by SEAR.
 */
export interface RACRouteAuthOptions {
    /**
     * RACROUTE AUTH status option. ACCESS returns the caller's current access in the result.
     */
    status?: string;
}

/**
 * SEAR request object
 */
export interface SearRequest {
    /**
    * Operation to perform: 'extract', 'search', 'alter', 'add', 'delete', 'remove', 'auth'
     */
    operation: 'extract' | 'search' | 'alter' | 'add' | 'delete' | 'remove' | 'auth';

    /**
    * Type of resource: 'user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf', or 'racf-options'.
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
        | 'racf-options';

    /**
    * User ID (required for extract admin_type='user'; not supported for auth)
     */
    userid?: string;

    /**
    * Group name/ID (canonical field for group operations; not supported for auth)
     */
    group?: string;

    /**
     * Dataset name (required for extract admin_type='dataset')
     */
    dataset?: string;

    /**
    * Keyring name (required for keyring and certificate operations)
     */
    keyring?: string;

    /**
    * Owner user ID (required for keyring and certificate operations)
     */
    owner?: string;

    /**
    * Certificate keyring owner user ID (required for admin_type='certificate')
    */
    keyring_owner?: string;

    /**
    * Certificate label (required for admin_type='certificate')
     */
    label?: string;

    /**
    * Certificate usage (required for certificate add)
    */
    usage?: 'PERSONAL' | 'personal' | 'SITE' | 'site' | 'CERTAUTH' | 'certauth';

    /**
    * Certificate trust status (required for certificate add)
    */
    status?: 'TRUST' | 'trust' | 'HIGHTRUST' | 'hightrust' | 'NOTRUST' | 'notrust';

    /**
    * Whether the certificate is default for the keyring (optional for certificate add)
    */
    default?: 'yes' | 'no';

    /**
    * Certificate file path (optional for certificate add)
    */
    certificate_file?: string;

    /**
    * Private key file path (optional for certificate add)
    */
    private_key_file?: string;

    /**
     * Resource name (required for extract admin_type='resource')
     */
    resource?: string;

    /**
     * RACF resource class (required for admin_type='resource')
     */
    class_name?: string;

    /**
     * Resource profile type (optional for extract admin_type='resource')
     */
    profile_type?: string;

    /**
     * RACROUTE access level for operation='auth'
     */
    access?: string;

    /**
     * Optional RACROUTE REQUEST=AUTH operands.
     */
    racroute_options?: RACRouteAuthOptions;

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
    raw_request: Uint8Array;

    /**
     * Raw result buffer from RACF
     */
    raw_result: Uint8Array;

    /**
     * Parsed result object
     */
    result: SearResultObject;

    constructor(options: {
        request: SearRequest;
        raw_request: Uint8Array;
        raw_result: Uint8Array;
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
    name: string;
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
 * Execute a SEAR operation asynchronously using child-process isolation
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
export const VALID_OPERATIONS: readonly ['extract', 'search', 'alter', 'add', 'delete', 'remove', 'auth'];

/**
 * Valid admin types
 */
export const VALID_ADMIN_TYPES: readonly ['user', 'group', 'dataset', 'group-connection', 'permission', 'keyring', 'certificate', 'resource', 'racf-rrsf', 'racf-options'];
