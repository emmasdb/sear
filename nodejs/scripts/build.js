'use strict';

const { spawnSync } = require('child_process');

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

if (!makeCommand) {
    console.error('Unable to find GNU make. Install gmake or set MAKE to a GNU make executable.');
    process.exit(1);
}

const result = spawnSync('node-gyp', ['configure', 'build'], {
    stdio: 'inherit',
    env: {
        ...process.env,
        MAKE: makeCommand,
    },
    shell: process.platform === 'win32',
});

if (result.error) {
    throw result.error;
}

process.exit(result.status ?? 0);
