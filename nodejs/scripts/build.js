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

function hasNodeGypMetadata(nodeDir) {
    return Boolean(nodeDir) && fs.existsSync(path.join(nodeDir, 'common.gypi'));
}

function findCurrentNodeDir() {
    const explicitNodeDir = process.env.SEAR_NODE_NODEDIR;
    if (hasNodeGypMetadata(explicitNodeDir)) {
        return explicitNodeDir;
    }

    const executableNodeDir = path.dirname(path.dirname(process.execPath));
    if (hasNodeGypMetadata(executableNodeDir)) {
        return executableNodeDir;
    }

    return null;
}

function buildEnv(overrides) {
    const env = {
        ...process.env,
        ...overrides,
    };

    const configuredNodeDir = env.npm_config_nodedir;
    const currentNodeDir = findCurrentNodeDir();

    if (process.env.SEAR_NODE_NODEDIR) {
        env.npm_config_nodedir = process.env.SEAR_NODE_NODEDIR;
    } else if (configuredNodeDir && !hasNodeGypMetadata(configuredNodeDir) && currentNodeDir) {
        console.warn(
            `Ignoring stale npm_config_nodedir (${configuredNodeDir}); using ${currentNodeDir}`
        );
        env.npm_config_nodedir = currentNodeDir;
    } else if (configuredNodeDir && !hasNodeGypMetadata(configuredNodeDir)) {
        console.warn(`Ignoring stale npm_config_nodedir (${configuredNodeDir})`);
        delete env.npm_config_nodedir;
    } else if (!configuredNodeDir && currentNodeDir) {
        env.npm_config_nodedir = currentNodeDir;
    }

    return env;
}

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
    env: buildEnv({
        MAKE: makeCommand,
        CC: realCc,
        CXX: realCxx,
        LDFLAGS: linkWithIrrseq,
    }),
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
    env: buildEnv({
        MAKE: makeCommand,
        CC: process.env.SEAR_NODE_CC || ccWrapperPath,
        CXX: process.env.SEAR_NODE_CXX || cxxWrapperPath,
        LDFLAGS: linkWithIrrseq,
    }),
    shell: process.platform === 'win32',
});

if (buildResult.error) {
    throw buildResult.error;
}

process.exit(buildResult.status ?? 0);
