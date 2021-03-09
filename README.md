# Compiling Lua with Cosmopolitan

This is a fork of the Github repo for [Lua][lua] customized to compile with
[Cosmopolitan][cosmo] libc.  Meant for experimental purposes only. This repo
results in 340kb portable Lua executable, the Cosmopolitan monorepo now compiles
a slimmer executable.

The `include` folder contains dummy headers corresponding to the [C
stdlib][cstdlib]. The `libcosmo` folder contains the required files for
compiling an executable using Cosmopolitan on Linux.

Compiled on:

```bash
# Debian Buster (Linux 4.19.0-12-amd64)
$ gcc --version
gcc (Debian 8.3.0-6) 8.3.0
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
$ clang --version
Debian clang version 11.0.1-++20201103062553+ef4ffcafbb2-1~exp1~20201103053214.125
Target: x86_64-pc-linux-gnu
Thread model: posix
```

`makefile` runs without error (a couple of warnings) to produce `lua.exe`.  The
compiled `lua.exe` runs on Linux and Windows 10. `clang` produces a slightly
larger binary than `gcc`.

The compiled `lua.exe` passes all tests in the `testes` directory. Run `all`
after compilation to check it passes all tests.


## Usage Instructions

1. Compile the latest commit in the `master` branch of `Cosmopolitan`.
2. Copy `cosmopolitan.h` (and `crt.o`, `ape.o`, `ape.lds`, `cosmopolitan.a`) to
   the `libcosmo/` folder.
3. Run `make`
4. Run `lua.exe`

```bash
cd libcosmo/
# use the latest version
wget https://justine.lol/cosmopolitan/cosmopolitan-amalgamation-0.2.zip
unzip cosmopolitan-amalgamation-0.2.zip
# copy the compiled versions from the latest commit in master branch
cd ../
make -j4
./lua.exe
```

[lua]: https://www.lua.org
[cosmo]: https://github.com/jart/cosmopolitan
[cstdlib]: https://en.cppreference.com/w/c/header
