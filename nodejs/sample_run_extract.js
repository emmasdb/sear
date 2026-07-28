'use strict';

const { sear } = require('./sear');

try {
    const request = {
        "operation": "extract",
        "admin_type": "user",
        "userid": "<userid>",
    }
    const response = sear(request, true);
    console.log('\nResult:');
    console.log(JSON.stringify(response.result, null, 2));
} catch (error) {
    console.error('SEAR extract failed:', error.message);
    process.exit(1);
}
