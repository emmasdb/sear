'use strict';

const fs = require('fs');
const path = require('path');

const addonPath = path.resolve(__dirname, '..', 'build', 'Release', '_sear.node');

if (!fs.existsSync(addonPath)) {
    console.error(`Missing addon binary: ${addonPath}`);
    console.error('Run `npm run build` first.');
    process.exit(1);
}

const addon = require(addonPath);
if (typeof addon.call_sear !== 'function') {
    console.error('Addon loaded, but `call_sear` export is missing.');
    process.exit(1);
}

const wrapper = require('./sear');
if (typeof wrapper.sear !== 'function') {
    console.error('Wrapper export `sear` is missing.');
    process.exit(1);
}
if (typeof wrapper.SecurityResult !== 'function') {
    console.error('Wrapper export `SecurityResult` is missing.');
    process.exit(1);
}

console.log('Smoke test passed: addon and wrapper exports are available.');
