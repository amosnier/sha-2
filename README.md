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
  no effort at all has been put in optimization.

## License

This repository is made available in the public domain. See [LICENSE
FILE](LICENSE).
