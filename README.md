# Measurement Kit GeoIP library

[![GitHub license](https://img.shields.io/github/license/measurement-kit/mkgeoip.svg)](https://raw.githubusercontent.com/measurement-kit/mkgeoip/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/mkgeoip.svg)](https://github.com/measurement-kit/mkgeoip/releases) [![Build Status](https://img.shields.io/travis/measurement-kit/mkgeoip/master.svg?label=travis)](https://travis-ci.org/measurement-kit/mkgeoip) [![codecov](https://codecov.io/gh/measurement-kit/mkgeoip/branch/master/graph/badge.svg)](https://codecov.io/gh/measurement-kit/mkgeoip) [![Build status](https://img.shields.io/appveyor/ci/bassosimone/mkgeoip/master.svg?label=appveyor)](https://ci.appveyor.com/project/bassosimone/mkgeoip/branch/master)

Measurement Kit library for performing probe IP, probe ASN, probe CC, and
probe ORG resolution. Since this is a basic building block that aims to
be vendored in Measurement Kit, we don't provide any stable API guarantee
(i.e. we'll never release `v1.0.0`).

## Regenerating build files

Possibly edit `MKBuild.yaml`, then run:

```
go get -v github.com/measurement-kit/mkbuild
mkbuild
```

## Building

```
mkdir build
cd build
cmake -GNinja ..
cmake --build .
ctest -a -j8 --output-on-failure
```

## Testing with docker

```
./docker.sh <build-type>
```
