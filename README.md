# NadaLisp Project

It's Scheme implementation implemented by Copilot using Claude Sonnet 3.7 Thinking, Gemini 2.5 Pro (Preview) and, to a lesser degree, all of it's collegues. Human did only little hand-holding and debris removal...

Since the language is in flux, have a look at `tests/lisp_tests` for the current language elements. `src/nadalib` will contain a standard library.

### Remarks on model performance (2025-04-12)

_Note: this is of course a snapshot in time_

- The bulk of the work has been done by Claude Sonnet 3.7 Thinking
- Available only later on, Gemini 2.5 Pro (Preview) was the only model that could resolve a very complex memory leak within nested lambda environments. It solved this in (almost) single shot, where all other models failed, even when they tried with many, many different approaches. Gemini 2.5 is currently the king in understanding complex projects.
- GPT model (4o, o1 prev, o3) did not meaningfully contribute after the project reached a certain complexity.

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
