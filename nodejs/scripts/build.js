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
const nodeGypNodeDir = path.join(__dirname, '..', '..', 'build', 'node-gyp-node-dir');

function hasNodeGypMetadata(nodeDir) {
    return Boolean(nodeDir) && fs.existsSync(path.join(nodeDir, 'common.gypi'));
}

function hasNodeHeaders(nodeDir) {
    return Boolean(nodeDir) && fs.existsSync(path.join(nodeDir, 'include', 'node', 'node.h'));
}

function hasIncludeNodeGypMetadata(nodeDir) {
    return Boolean(nodeDir) && fs.existsSync(path.join(nodeDir, 'include', 'node', 'common.gypi'));
}

function isUsableNodeGypDir(nodeDir) {
    return hasNodeGypMetadata(nodeDir) && hasNodeHeaders(nodeDir);
}

function findCurrentNodeRoot() {
    return path.dirname(path.dirname(process.execPath));
}

function removePath(targetPath) {
    fs.rmSync(targetPath, { force: true, recursive: true });
}

function linkOrCopyDirectory(source, target) {
    removePath(target);

    try {
        fs.symlinkSync(source, target, 'dir');
    } catch (error) {
        fs.cpSync(source, target, { recursive: true });
    }
}

function linkOrCopyFile(source, target) {
    removePath(target);

    try {
        fs.symlinkSync(source, target, 'file');
    } catch (error) {
        fs.copyFileSync(source, target);
    }
}

function createNodeGypNodeDir(nodeRoot) {
    const includeNodeDir = path.join(nodeRoot, 'include', 'node');
    const includeCommonGypi = path.join(includeNodeDir, 'common.gypi');

    if (!hasIncludeNodeGypMetadata(nodeRoot) || !hasNodeHeaders(nodeRoot)) {
        return null;
    }

    removePath(nodeGypNodeDir);
    fs.mkdirSync(path.join(nodeGypNodeDir, 'include'), { recursive: true });
    linkOrCopyFile(includeCommonGypi, path.join(nodeGypNodeDir, 'common.gypi'));
    linkOrCopyDirectory(includeNodeDir, path.join(nodeGypNodeDir, 'include', 'node'));

    console.warn(`Using local Node headers from ${includeNodeDir}`);

    return nodeGypNodeDir;
}

function findCurrentNodeDir() {
    const explicitNodeDir = process.env.SEAR_NODE_NODEDIR;
    if (isUsableNodeGypDir(explicitNodeDir)) {
        return explicitNodeDir;
    }

    const generatedExplicitNodeDir = createNodeGypNodeDir(explicitNodeDir);
    if (generatedExplicitNodeDir) {
        return generatedExplicitNodeDir;
    }

    const executableNodeDir = findCurrentNodeRoot();
    if (isUsableNodeGypDir(executableNodeDir)) {
        return executableNodeDir;
    }

    const generatedNodeDir = createNodeGypNodeDir(executableNodeDir);
    if (generatedNodeDir) {
        return generatedNodeDir;
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

    if (process.env.SEAR_NODE_NODEDIR && isUsableNodeGypDir(process.env.SEAR_NODE_NODEDIR)) {
        env.npm_config_nodedir = process.env.SEAR_NODE_NODEDIR;
    } else if (process.env.SEAR_NODE_NODEDIR && currentNodeDir) {
        console.warn(
            `Ignoring unusable SEAR_NODE_NODEDIR (${process.env.SEAR_NODE_NODEDIR}); using ${currentNodeDir}`
        );
        env.npm_config_nodedir = currentNodeDir;
    } else if (process.env.SEAR_NODE_NODEDIR) {
        console.warn(`Ignoring unusable SEAR_NODE_NODEDIR (${process.env.SEAR_NODE_NODEDIR})`);
        delete env.npm_config_nodedir;
    } else if (configuredNodeDir && !isUsableNodeGypDir(configuredNodeDir) && currentNodeDir) {
        console.warn(
            `Ignoring stale npm_config_nodedir (${configuredNodeDir}); using ${currentNodeDir}`
        );
        env.npm_config_nodedir = currentNodeDir;
    } else if (configuredNodeDir && !isUsableNodeGypDir(configuredNodeDir)) {
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
