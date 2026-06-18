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
const irrseqObjectPath = path.join(__dirname, '..', '..', 'artifacts', 'irrseq00.o');

if (!makeCommand) {
    console.error('Unable to find GNU make. Install gmake or set MAKE to a GNU make executable.');
    process.exit(1);
}

fs.chmodSync(ccWrapperPath, 0o755);
fs.chmodSync(cxxWrapperPath, 0o755);

const existingLdflags = process.env.LDFLAGS ? `${process.env.LDFLAGS} ` : '';
const linkWithIrrseq = `${existingLdflags}${irrseqObjectPath}`;

const realCc = process.env.SEAR_NODE_REAL_CC || 'ibm-clang64';
const realCxx = process.env.SEAR_NODE_REAL_CXX || 'ibm-clang++64';

const configureResult = spawnSync('node-gyp', ['configure'], {
    stdio: 'inherit',
    env: {
        ...process.env,
        MAKE: makeCommand,
        CC: realCc,
        CXX: realCxx,
        LDFLAGS: linkWithIrrseq,
    },
    shell: process.platform === 'win32',
});

if (configureResult.error) {
    throw configureResult.error;
}

if (configureResult.status !== 0) {
    process.exit(configureResult.status ?? 1);
}

const buildResult = spawnSync('node-gyp', ['build'], {
    stdio: 'inherit',
    env: {
        ...process.env,
        MAKE: makeCommand,
        CC: process.env.SEAR_NODE_CC || ccWrapperPath,
        CXX: process.env.SEAR_NODE_CXX || cxxWrapperPath,
        LDFLAGS: linkWithIrrseq,
    },
    shell: process.platform === 'win32',
});

if (buildResult.error) {
    throw buildResult.error;
}

process.exit(buildResult.status ?? 0);
