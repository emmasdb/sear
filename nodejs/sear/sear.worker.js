'use strict';

const { parentPort, workerData } = require('worker_threads');

const _C = require(workerData.nativeModulePath);

parentPort.on('message', (message) => {
    try {
        const response = _C.call_sear(message.request, message.debug);
        parentPort.postMessage({ success: true, response });
    } catch (error) {
        parentPort.postMessage({
            success: false,
            error: error.message,
        });
    }
});
