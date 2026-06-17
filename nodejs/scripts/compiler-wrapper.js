'use strict';

const { spawnSync } = require('child_process');

const [compiler, ...args] = process.argv.slice(2);

if (!compiler) {
    console.error('Missing compiler argument.');
    process.exit(1);
}

const unsupportedArgs = new Set([
    '-q64',
    '-qlonglong',
    '-qenum=int',
    '-qxclang=-fexec-charset=ISO8859-1',
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
