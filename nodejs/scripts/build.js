'use strict';

const { spawnSync } = require('child_process');
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
const wrapperPath = path.join(__dirname, 'compiler-wrapper.js');
const ccCommand = process.env.SEAR_NODE_CC || `"${process.execPath}" "${wrapperPath}" ibm-clang64`;
const cxxCommand = process.env.SEAR_NODE_CXX || `"${process.execPath}" "${wrapperPath}" ibm-clang++64`;

if (!makeCommand) {
    console.error('Unable to find GNU make. Install gmake or set MAKE to a GNU make executable.');
    process.exit(1);
}

const result = spawnSync('node-gyp', ['configure', 'build'], {
    stdio: 'inherit',
    env: {
        ...process.env,
        MAKE: makeCommand,
        CC: ccCommand,
        CXX: cxxCommand,
    },
    shell: process.platform === 'win32',
});

if (result.error) {
    throw result.error;
}

process.exit(result.status ?? 0);
