'use strict';

const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const repoRoot = path.resolve(__dirname, '..', '..');
const artifactsDir = path.join(repoRoot, 'artifacts');
const asmSources = [
    {
        source: path.join(repoRoot, 'sear', 'irrseq00', 'irrseq00.s'),
        includeDirs: [path.join(repoRoot, 'sear', 'irrseq00')],
        output: path.join(artifactsDir, 'irrseq00.o'),
    },
    {
        source: path.join(repoRoot, 'sear', 'irrsim00', 'irrsim00.s'),
        includeDirs: [
            path.join(repoRoot, 'sear', 'irrsim00'),
            path.join(repoRoot, 'sear', 'irrseq00'),
        ],
        output: path.join(artifactsDir, 'irrsim00.o'),
    },
];
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

function assembleRacfBridges() {
    fs.mkdirSync(artifactsDir, { recursive: true });

    for (const asmSource of asmSources) {
        run('as', [
            '-mGOFF',
            ...asmSource.includeDirs.map((includeDir) => `-I${includeDir}`),
            '-o',
            asmSource.output,
            asmSource.source,
        ]);
    }
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

assembleRacfBridges();
generateSchemaHeader();
