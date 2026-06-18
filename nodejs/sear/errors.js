'use strict';

/**
 * Base error class for SEAR-related errors
 */
class SearError extends Error {
    constructor(message, code = 'SEAR_ERROR') {
        super(message);
        this.name = 'SearError';
        this.code = code;
    }
}

/**
 * Validation error for invalid request objects
 */
class ValidationError extends SearError {
    constructor(message, details = {}) {
        super(message, 'VALIDATION_ERROR');
        this.name = 'ValidationError';
        this.details = details;
    }
}

/**
 * Native binding error
 */
class NativeError extends SearError {
    constructor(message, details = {}) {
        super(message, 'NATIVE_ERROR');
        this.name = 'NativeError';
        this.details = details;
    }
}

module.exports = {
    SearError,
    ValidationError,
    NativeError,
};
