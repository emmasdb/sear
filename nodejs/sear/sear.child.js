'use strict';

const [nativeModulePath, requestJson, debugValue] = process.argv.slice(2);
const _C = require(nativeModulePath);

const response = _C.call_sear(requestJson, debugValue === 'true');

process.stdout.write(JSON.stringify({
    raw_request: response.raw_request.toString('base64'),
    raw_result: response.raw_result.toString('base64'),
    result_json: response.result_json,
}));
