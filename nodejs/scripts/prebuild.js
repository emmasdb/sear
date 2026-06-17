'use strict';

const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const repoRoot = path.resolve(__dirname, '..', '..');
const artifactsDir = path.join(repoRoot, 'artifacts');
const asmSource = path.join(repoRoot, 'sear', 'irrseq00', 'irrseq00.s');
const asmIncludeDir = path.join(repoRoot, 'sear', 'irrseq00');
const asmOutput = path.join(artifactsDir, 'irrseq00.o');
const schemaPath = path.join(repoRoot, 'schema.json');
const schemaHeaderPath = path.join(repoRoot, 'sear', 'sear_schema.hpp');

function run(command, args) {
    const result = spawnSync(command, args, {
        cwd: repoRoot,
        stdio: 'inherit',
    });

    if (result.error) {
        throw result.error;
    }

    if (result.status !== 0) {
        process.exit(result.status ?? 1);
    }
}

function assembleIrrseq00() {
    fs.mkdirSync(artifactsDir, { recursive: true });

    run('as', [
        '-mGOFF',
        `-I${asmIncludeDir}`,
        '-o',
        asmOutput,
        asmSource,
    ]);
}

function generateSchemaHeader() {
    const schema = JSON.stringify(JSON.parse(fs.readFileSync(schemaPath, 'utf8')));
    const header = [
        '#ifndef __SEAR_SCHEMA_H_',
        '#define __SEAR_SCHEMA_H_',
        '',
        `#define SEAR_SCHEMA R"(${schema})"_json`,
        '',
        '#endif',
        '',
    ].join('\n');

    fs.writeFileSync(schemaHeaderPath, header);
}

assembleIrrseq00();
generateSchemaHeader();
