# NadaLisp Project

It's Scheme implementation implemented by Copilot using Claude Sonnet 3.7 Thinking and all of it's collegues. Human did only little hand-holding and debris removal...

Since the language is in flux, have a look at `tests/lisp_tests` for the current language elements. `src/nadalib` will contain a standard library.

## Build

Requires: `cmake`, `ninja`, `readline`

### One time preparation

```bash
git clone https://github.com/domschl/NadaLisp
cd NadaLisp
mkdir build
```

### (Re-)build

In `NadaLisp/build`:

```bash
cmake -G Ninja ..
ninja
```

### Execute

```bash
NadaLisp/build/src/nada
```

## Tests

In `NadaLisp/build`:

```bash
ctest -L LispTests         # Just functional tests
ctest -L MemoryTests       # All memory tests
ctest -L LispMemoryTests   # Memory tests for regular Lisp files only
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.
