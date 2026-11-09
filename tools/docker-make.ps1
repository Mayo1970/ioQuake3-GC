$ErrorActionPreference = 'Stop'

$Image = 'devkitpro/devkitppc:20261102'
$RepoRoot = Split-Path -Parent $PSScriptRoot

$DockerArgs = @(
    'run', '--rm',
    '-v', "${RepoRoot}:/src",
    '-w', '/src',
    $Image,
    'bash', '-c',
    'export DEVKITPRO=/opt/devkitpro DEVKITPPC=/opt/devkitpro/devkitPPC PATH=/opt/devkitpro/devkitPPC/bin:/opt/devkitpro/tools/bin:$PATH; make "$@"',
    '--'
)

& docker @DockerArgs @args
exit $LASTEXITCODE
