# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test Commands

```bash
# Configure (from build dir — typically `build/` or CLion's `cmake-build-debug/`)
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake .. -DCLAP_ENABLE_ASAN=ON -DCLAP_BUILD_TESTS=ON           # AddressSanitizer
cmake .. -DCLAP_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug    # Coverage (requires gcovr)

# Build
cmake --build .
cmake --build . --target basic              # Build a single example
cmake --build . --target pot                # Regenerate i18n .pot template

# Test
ctest --output-on-failure                   # Run all tests
ctest -R usage                              # Run tests matching "usage"
ctest -R "^test_"                           # Run unit tests only
./tests/test_parser                         # Run a specific test binary directly
cmake --build . --target coverage && open coverage/index.html  # Coverage report

# Sanitizers (ASan+UBSan)
cmake .. -DCLAP_BUILD_SHARED=OFF -DCLAP_ENABLE_ASAN=ON -DCLAP_ENABLE_UBSAN=ON
cmake --build . && ctest

# Fuzz (Clang only)
cmake .. -DCLAP_BUILD_FUZZ=ON -DCMAKE_C_COMPILER=clang
cmake --build . --target fuzz_clap
./tests/fuzz_clap -max_len=4096 -runs=50000 corpus/

# Memory (Linux)
valgrind --leak-check=full ./tests/test_usage

# Install
sudo cmake --install .
```

## Architecture

**C99 argument parsing library** inspired by Python's argparse. Zero dependencies. MIT license.

### Parse Pipeline (multi-stage)

1. **Tokenizer** (`clap_tokenizer.c`) — splits `argv` into tokens: positional, long option, short option, short bundle (`-abc`), stop (`--`), end.
2. **Pattern analysis** (`clap_pattern.c`) — maps token sequence to registered arguments.
3. **State-machine parser** (`clap_parser_stage.c`) — iterates tokens/pattern, dispatches values to action handlers.
4. **Validation** (`clap_validator.c`) — checks required args, choices, nargs, dependencies, mutex groups.
5. **Defaults** — fills missing values from `clap_argument_default()`.

### Internal Structure (`src/clap_parser_internal.h`)

- `clap_parser_t` owns: argument list, mutex groups, display groups, dependencies, type registry, subparsers
- `clap_argument_t` holds: flags, dest, action, nargs, type name, choices, default, handler, mutex group ID, display group ID
- `clap_namespace_t` — flat key-value store holding parsed results; typed accessors (`get_string`, `get_int`, etc.)
- `clap_buffer_t` — dynamic string buffer (flexible array member) used for safe string building

### Source Layout

| Path | Purpose |
|------|---------|
| `include/clap/` | 8 public headers (umbrella `clap.h` + types, error, version, color, i18n, export) |
| `src/clap.c` | Glue / top-level API exports |
| `src/clap_parser.c` | Parser lifecycle, argument registration |
| `src/clap_argument.c` | Argument setter chain methods |
| `src/clap_namespace.c` | Namespace value storage |
| `src/clap_tokenizer.c` | argv tokenizer |
| `src/clap_pattern.c` | Token-argument pattern matching |
| `src/clap_parser_stage.c` | Parse state machine |
| `src/clap_validator.c` | Post-parse validation |
| `src/clap_actions.c` | 10 built-in action handlers |
| `src/clap_action_executor.c` | Action dispatch |
| `src/clap_mutex.c` | Mutex group tracking |
| `src/clap_display_group.c` | Display group tracking |
| `src/clap_dependency.c` | Requires/conflicts |
| `src/clap_convert.c` | Type converters (string/int/float/bool) |
| `src/clap_formatter.c` | Help/usage text generation |
| `src/clap_subparser.c` | Subcommand dispatch |
| `src/clap_find.c` | Option lookup (exact + abbrev) |
| `src/clap_buffer.c` | Dynamic string buffer |
| `src/clap_color.c` | ANSI color init/detect |
| `src/clap_allocator.c` | Custom allocator support |
| `src/clap_i18n.c` | Translation dispatch |
| `src/clap_parser_internal.h` | Internal structs, limits, function prototypes |
| `tests/unit/` | 22 per-module unit tests (Unity framework) |
| `tests/integration/` | 3 end-to-end tests |
| `tests/fuzz/` | libFuzzer-based fuzz target |
| `examples/` | 4 examples: basic, git_style, action_demo, custom_type |
| `cmake/` | CMake helper modules (CompilerWarnings, Sanitizers, package config) |
| `po/` | i18n .pot translation template (regenerate via `cmake --build . --target pot`) |
| `.github/workflows/` | 5 CI workflows: ci, coverage, sanitizers, fuzz, docs |

## API Style

- **Object-oriented C**: `clap_parser_t*`, `clap_argument_t*`, `clap_namespace_t*` with `_new()`/`_free()` lifecycle.
- **Fluent setters**: `clap_argument_*()` return `clap_argument_t*` for chaining:
  ```c
  clap_argument_help(clap_argument_type(arg, "string"), "help text");
  ```
- **Return value pattern**: Most functions return `bool`. `clap_parse_args()` returns `clap_parse_result_t`. Errors go through `clap_error_t` (code + 512-char message buffer).
- **All public functions** are marked `CLAP_EXPORT` for symbol visibility.
- **Naming** mirrors Python argparse: `store`, `append`, `nargs`, `dest`, `metavar`, `choices`.

## Important Gotchas

- **CLAP_STATIC**: Must `#define CLAP_STATIC` at compile time when linking statically. Without it, Windows emits `__declspec(dllimport)` decorations incompatible with static libraries.
- **One-shot globals**: `clap_set_allocator()` and `clap_set_translator()` must be called before `clap_parser_new()`. Subsequent calls are silently ignored.
- **Argument spec syntax**: `"--output/-o"` (split on `/`) creates both `--output` and `-o`. Positional args use bare names like `"filename"`. The `dest` key is auto-derived from the longest flag (dashes → underscores).
- **Internal limits**: `CLAP_MAX_ARGUMENTS=256`, `CLAP_MAX_ARG_LENGTH=4096`, `CLAP_MAX_CHOICES=1000`, `CLAP_MAX_DEPENDENCIES=16` — defined in `clap_parser_internal.h`.
- **Tests are white-box**: Test files include `clap_parser_internal.h` and directly access internal structures.
- **i18n single-call**: `clap_set_translator()` is one-shot; the `test_i18n` test must not have tests reordered.
- **Custom type handlers** validate input only — raw string is stored in namespace regardless. Use `CLAP_ACTION_CUSTOM` to persist typed results.
- **Color detection** respects `NO_COLOR`/`FORCE_COLOR`/`CLICOLOR`/`CLICOLOR_FORCE` env vars per the [no-color.org](https://no-color.org) convention.
