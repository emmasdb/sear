#!/usr/bin/env node
'use strict';

const { spawnSync } = require('child_process');

const compiler = process.env.SEAR_NODE_REAL_CC || 'ibm-clang64';
const args = process.argv.slice(2);
const unsupportedArgs = new Set([
    '-q64',
    '-qlonglong',
    '-qenum=int',
    '-qxclang=-fexec-charset=ISO8859-1',
    '-Wc,DLL',
    '-qmakedep=gcc',
]);

const filteredArgs = args.filter((arg) => !unsupportedArgs.has(arg));

const result = spawnSync(compiler, filteredArgs, {
    stdio: 'inherit',
});

if (result.error) {
    throw result.error;
}

process.exit(result.status ?? 0);
