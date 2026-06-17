'use strict';
const _C = require('../build/Release/_sear.node');

class SecurityResult {
    constructor({ request, raw_request, raw_result, result }) {
        this.request = request;
        this.raw_request = raw_request;  // Buffer
        this.raw_result = raw_result;    // Buffer
        this.result = result;            // parsed object
    }
}

function sear(request, debug = false) {
    const response = _C.call_sear(JSON.stringify(request), debug);
    return new SecurityResult({
        request,
        raw_request: response.raw_request,
        raw_result:  response.raw_result,
        result:      JSON.parse(response.result_json),
    });
}

module.exports = { sear, SecurityResult };
