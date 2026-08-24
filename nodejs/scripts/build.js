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
const irrsimObjectPath = path.join(__dirname, '..', '..', 'artifacts', 'irrsim00.o');
const nodeGypNodeDir = path.join(__dirname, '..', '..', 'build', 'node-gyp-node-dir');
const buildReleaseDir = path.join(__dirname, '..', '..', 'build', 'Release');

function hasNodeGypMetadata(nodeDir) {
    return typeof nodeDir === 'string' && fs.existsSync(path.join(nodeDir, 'common.gypi'));
}

function hasNodeHeaders(nodeDir) {
    return typeof nodeDir === 'string' && fs.existsSync(path.join(nodeDir, 'include', 'node', 'node.h'));
}

function hasIncludeNodeGypMetadata(nodeDir) {
    return typeof nodeDir === 'string' && fs.existsSync(path.join(nodeDir, 'include', 'node', 'common.gypi'));
}

function isNodeInstallDir(nodeDir) {
    return hasIncludeNodeGypMetadata(nodeDir) && hasNodeHeaders(nodeDir);
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

function listFiles(directory) {
    if (!fs.existsSync(directory)) {
        return [];
    }

    return fs.readdirSync(directory, { withFileTypes: true }).flatMap((entry) => {
        const entryPath = path.join(directory, entry.name);
        return entry.isDirectory() ? listFiles(entryPath) : [entryPath];
    });
}

function reportExceptionFlags() {
    const buildDir = path.join(__dirname, '..', '..', 'build');
    const filesWithExceptions = [];
    const filesWithNoExceptions = [];

    for (const filePath of listFiles(buildDir)) {
        if (!filePath.endsWith('.mk') && !filePath.endsWith('.gypi')) {
            continue;
        }

        const content = fs.readFileSync(filePath, 'utf8');
        const relativePath = path.relative(path.join(__dirname, '..', '..'), filePath);
        if (content.includes('-fexceptions')) {
            filesWithExceptions.push(relativePath);
        }
        if (content.includes('-fno-exceptions')) {
            filesWithNoExceptions.push(relativePath);
        }
    }

    if (filesWithExceptions.length === 0 && filesWithNoExceptions.length === 0) {
        console.warn('No generated exception flags found in node-gyp build files');
        return;
    }

    if (filesWithExceptions.length > 0) {
        console.warn(`Generated -fexceptions found in: ${filesWithExceptions.join(', ')}`);
    }
    if (filesWithNoExceptions.length > 0) {
        console.warn(`Generated -fno-exceptions found in: ${filesWithNoExceptions.join(', ')}`);
    }
}

function reportCharModeFlags() {
    const buildDir = path.join(__dirname, '..', '..', 'build');
    const matches = [];

    for (const filePath of listFiles(buildDir)) {
        if (!filePath.endsWith('.mk') && !filePath.endsWith('.gypi')) {
            continue;
        }

        const content = fs.readFileSync(filePath, 'utf8');
        if (content.includes('-fzos-le-char-mode')) {
            matches.push(path.relative(path.join(__dirname, '..', '..'), filePath));
        }
    }

    if (matches.length === 0) {
        console.warn('No generated z/OS char-mode flags found in node-gyp build files');
        return;
    }

    console.warn(`Generated z/OS char-mode flags found in: ${matches.join(', ')}`);
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
    if (typeof nodeRoot !== 'string') {
        return null;
    }

    const includeNodeDir = path.join(nodeRoot, 'include', 'node');
    const includeCommonGypi = path.join(includeNodeDir, 'common.gypi');

    if (!isNodeInstallDir(nodeRoot)) {
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
    } else if (process.env.SEAR_NODE_NODEDIR && isNodeInstallDir(process.env.SEAR_NODE_NODEDIR) && currentNodeDir) {
        console.warn(
            `Using Node headers from SEAR_NODE_NODEDIR (${process.env.SEAR_NODE_NODEDIR}); generated ${currentNodeDir}`
        );
        env.npm_config_nodedir = currentNodeDir;
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
removePath(buildReleaseDir);

const existingLdflags = process.env.LDFLAGS ? `${process.env.LDFLAGS} ` : '';
const linkWithRacfBridges = `${existingLdflags}${irrseqObjectPath} ${irrsimObjectPath}`;

const realCc = process.env.SEAR_NODE_REAL_CC || 'ibm-clang64';
const realCxx = process.env.SEAR_NODE_REAL_CXX || 'ibm-clang++64';

const configureResult = spawnSync('node-gyp', ['configure'], {
    stdio: 'inherit',
    env: buildEnv({
        MAKE: makeCommand,
        CC: realCc,
        CXX: realCxx,
        LDFLAGS: linkWithRacfBridges,
    }),
    shell: process.platform === 'win32',
});

if (configureResult.error) {
    throw configureResult.error;
}

if (configureResult.status !== 0) {
    process.exit(configureResult.status ?? 1);
}

reportExceptionFlags();
reportCharModeFlags();

const buildResult = spawnSync('node-gyp', ['build'], {
    stdio: 'inherit',
    env: buildEnv({
        MAKE: makeCommand,
        CC: process.env.SEAR_NODE_CC || ccWrapperPath,
        CXX: process.env.SEAR_NODE_CXX || cxxWrapperPath,
        LDFLAGS: linkWithRacfBridges,
    }),
    shell: process.platform === 'win32',
});

if (buildResult.error) {
    throw buildResult.error;
}

process.exit(buildResult.status ?? 0);
