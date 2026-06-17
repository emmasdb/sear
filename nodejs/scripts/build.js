'use strict';

const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

function findGnuMake() {
    for (const command of ['gmake', 'make']) {
        const result = spawnSync(command, ['--version'], {
            stdio: 'pipe',
            encoding: 'utf8',
        });

        if (result.status === 0 && /gnu make/i.test(`${result.stdout}${result.stderr}`)) {
            return command;
        }
    }

    return null;
}

const makeCommand = process.env.MAKE || findGnuMake();
const ccWrapperPath = path.join(__dirname, 'cc-wrapper.js');
const cxxWrapperPath = path.join(__dirname, 'cxx-wrapper.js');

if (!makeCommand) {
    console.error('Unable to find GNU make. Install gmake or set MAKE to a GNU make executable.');
    process.exit(1);
}

fs.chmodSync(ccWrapperPath, 0o755);
fs.chmodSync(cxxWrapperPath, 0o755);

const result = spawnSync('node-gyp', ['configure', 'build'], {
    stdio: 'inherit',
    env: {
        ...process.env,
        MAKE: makeCommand,
        CC: process.env.SEAR_NODE_CC || ccWrapperPath,
        CXX: process.env.SEAR_NODE_CXX || cxxWrapperPath,
    },
    shell: process.platform === 'win32',
});

if (result.error) {
    throw result.error;
}

process.exit(result.status ?? 0);
