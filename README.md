# sha-2

## Contents

SHA-2 algorithm implementations.

At the moment, only SHA-256 is implemented.

## Design criteria

- Easy to test, include in any project, compile and link.

- ANSI C with as little specific C99 as possible (e.g. extended
  integer types are used, but not bool).

- Portable. Makes no assumptions on the target system's endianess or
  word size.

- The SHA-256 implementation is a straightforward implementation of
  the algorithm specified on
  [Wikipedia](https://en.wikipedia.org/wiki/SHA-2). At the moment,
  no effort at all has been put on optimization.

## Notes

The Makefile is as minimal as possible. No effort was put into making
it general. Its purpose is mainly to ease testing for the developer's
host machine. The actual implementation is however extremely easy to
include in any project, may it use GNU make or any other build tool.

## License

This repository is made available in the public domain. See [LICENSE
FILE](LICENSE).
