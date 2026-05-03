# libclap

**Command Line Argument Parser for C**

[![CI](https://github.com/wdl0214/libclap/actions/workflows/test.yml/badge.svg)](https://github.com/wdl0214/libclap/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/wdl0214/libclap/graph/badge.svg)](https://codecov.io/gh/wdl0214/libclap)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Standard](https://img.shields.io/badge/C-99-standard.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()
[![Docs](https://img.shields.io/badge/docs-Doxygen-blue.svg)](https://wdl0214.github.io/libclap/)

libclap is a modern, secure, and extensible command-line argument parsing library for C, inspired by Python's `argparse`. It provides a familiar, feature-rich API while maintaining C99 compatibility, zero external dependencies, and a strong focus on memory safety.

## ✨ Features

- **Full argparse Compatibility** – Positional arguments, optional flags, subcommands, mutually exclusive groups, `nargs` (`*`, `+`, `?`, `N`, `REMAINDER`), and more.
- **Rich Action System** – `store`, `store_true/false`, `store_const`, `append`, `append_const`, `count`, `help`, `version`, and custom actions.
- **Type Conversion** – Built-in support for `int`, `float`, `string`, and `bool`, plus user‑defined converters.
- **Automatic Help Generation** – Beautifully formatted usage and help messages with smart line‑wrapping and alignment.
- **Subcommands** – Full support for git‑style sub‑parsers with flat namespace merging.
- **Memory Safe** – Bounds‑checked string buffers, input validation, and comprehensive leak testing (Valgrind/ASan clean).
- **Cross‑Platform** – Windows, Linux, macOS, and BSD. Works with GCC, Clang, and MSVC.
- **Zero Dependencies** – Only requires a C99 standard library.
- **CMake Integration** – Easy to build and consume with `find_package(clap)` or `pkg-config`.

## 📦 Installation

### From Source

```bash
git clone https://github.com/wdl0214/libclap.git
cd libclap
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CLAP_BUILD_SHARED` | `ON` | Build shared library |
| `CLAP_BUILD_STATIC` | `ON` | Build static library |
| `CLAP_BUILD_TESTS` | `ON` | Build unit tests |
| `CLAP_BUILD_EXAMPLES` | `ON` | Build example programs |
| `CLAP_BUILD_FUZZ` | `OFF` | Build fuzz test (Clang only) |
| `CLAP_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer |
| `CLAP_ENABLE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer |
| `CLAP_ENABLE_COVERAGE` | `OFF` | Enable code coverage |

### Using in Your Project

> **⚠️ Static linking:** If you link libclap **statically**, you **must** define
> `CLAP_STATIC` at compile time (e.g. `-DCLAP_STATIC` or `#define CLAP_STATIC`).
> This is required on all platforms, but most important on Windows where the
> absence of `CLAP_STATIC` causes the compiler to emit `__declspec(dllimport)`
> decorations that are incompatible with a static library.
>
> When linking against the shared library, no additional defines are needed.

#### CMake `find_package`

```cmake
find_package(clap REQUIRED)

add_executable(myapp myapp.c)
target_link_libraries(myapp PRIVATE clap::clap_static)
target_compile_definitions(myapp PRIVATE CLAP_STATIC)
```

Use `clap::clap_shared` instead if you prefer linking against the shared library
(no `CLAP_STATIC` needed).

#### pkg-config

```bash
# Dynamic linking (default)
gcc myapp.c -o myapp $(pkg-config --cflags --libs libclap)

# Static linking — requires -DCLAP_STATIC
gcc myapp.c -o myapp $(pkg-config --cflags --static --libs libclap) -DCLAP_STATIC
```

#### Manual Linking

```bash
# Dynamic linking
gcc myapp.c -o myapp -I/usr/local/include -L/usr/local/lib -lclap

# Static linking — requires -DCLAP_STATIC
gcc myapp.c -o myapp -I/usr/local/include /usr/local/lib/libclap.a -DCLAP_STATIC
```

## 🚀 Quick Start

```c
#include <clap/clap.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    clap_error_t error = {0};
    clap_namespace_t *ns = NULL;
    clap_argument_t *arg;
    clap_parser_t *parser = clap_parser_new("my_program", "A simple example", NULL);
    
    arg = clap_add_argument(parser, "input");
    clap_argument_help(arg, "Input file");
    clap_argument_type(arg, "string");
    clap_argument_required(arg, true);
    
    arg = clap_add_argument(parser, "--output/-o");
    clap_argument_help(arg, "Output file");
    clap_argument_type(arg, "string");
    clap_argument_default(arg, "-");
    
    arg = clap_add_argument(parser, "--verbose/-v");
    clap_argument_help(arg, "Increase verbosity");
    clap_argument_action(arg, CLAP_ACTION_COUNT);
    
    clap_parse_result_t result = clap_parse_args(parser, argc, argv, &ns, &error);
    if (result == CLAP_PARSE_ERROR) {
        fprintf(stderr, "Error: %s\n", error.message);
        clap_parser_free(parser);
        return 1;
    }
    if (result == CLAP_PARSE_HELP || result == CLAP_PARSE_VERSION) {
        clap_parser_free(parser);
        return 0;
    }

    const char *input, *output;
    int verbose = 0;
    clap_namespace_get_string(ns, "input", &input);
    clap_namespace_get_string(ns, "output", &output);
    clap_namespace_get_int(ns, "verbose", &verbose);

    printf("Input: %s, Output: %s, Verbose: %d\n", input, output, verbose);

    clap_namespace_free(ns);
    clap_parser_free(parser);
    return 0;
}
```

Compile and run:

```bash
$ gcc -o myapp myapp.c -lclap
$ ./myapp --help
usage: my_program [-h] [--output OUTPUT] [--verbose] input

A simple example

Positional arguments:
  input                    Input file

Optional arguments:
  --help, -h               Show this help message and exit
  --output, -o OUTPUT      Output file (default: -)
  --verbose, -v            Increase verbosity
```

## 📚 API Overview

### Parser Lifecycle

```c
clap_parser_t *parser = clap_parser_new("prog", "Description", "Epilog");
clap_parser_set_help_width(parser, 100);
clap_parser_set_version(parser, "1.0.0");

clap_namespace_t *ns = NULL;
clap_error_t error = {0};
clap_parse_result_t result = clap_parse_args(parser, argc, argv, &ns, &error);
if (result == CLAP_PARSE_ERROR) {
    fprintf(stderr, "Error: %s\n", error.message);
    clap_parser_free(parser);
    return 1;
}
if (result == CLAP_PARSE_HELP || result == CLAP_PARSE_VERSION) {
    clap_parser_free(parser);
    return 0;
}

clap_namespace_free(ns);
clap_parser_free(parser);
```

### Adding Arguments

```c
clap_argument_t *arg = clap_add_argument(parser, "--output/-o");
clap_argument_help(arg, "Output file path");
clap_argument_default(arg, "-");
clap_argument_required(arg, true);
clap_argument_type(arg, "string");
clap_argument_metavar(arg, "FILE");
```

### Actions

| Action | Description |
|--------|-------------|
| `CLAP_ACTION_STORE` | Store the value (default) |
| `CLAP_ACTION_STORE_TRUE` | Store `true` when flag is present |
| `CLAP_ACTION_STORE_FALSE` | Store `false` when flag is present |
| `CLAP_ACTION_STORE_CONST` | Store a constant value |
| `CLAP_ACTION_APPEND` | Append value to a list |
| `CLAP_ACTION_APPEND_CONST` | Append constant to a list |
| `CLAP_ACTION_COUNT` | Count occurrences of the option |
| `CLAP_ACTION_HELP` | Print help and exit |
| `CLAP_ACTION_VERSION` | Print version and exit |
| `CLAP_ACTION_CUSTOM` | User‑defined handler |

### nargs Specifiers

| Specifier | Description |
|-----------|-------------|
| `1` (default) | Exactly one argument |
| `?` (`CLAP_NARGS_ZERO_OR_ONE`) | Zero or one argument |
| `*` (`CLAP_NARGS_ZERO_OR_MORE`) | Zero or more arguments |
| `+` (`CLAP_NARGS_ONE_OR_MORE`) | One or more arguments |
| `N` | Exactly N arguments |
| `REMAINDER` | Consume all remaining arguments |

### Subcommands

```c
clap_parser_t *subparsers = clap_add_subparsers(parser, "command", "Available commands");

clap_parser_t *commit = clap_subparser_add(subparsers, "commit", "Record changes");
clap_argument_t *msg_arg = clap_add_argument(commit, "-m");
clap_argument_dest(msg_arg, "message");
clap_argument_required(msg_arg, true);

// Parse as usual
const char *cmd;
clap_namespace_get_string(ns, "command", &cmd);
if (strcmp(cmd, "commit") == 0) {
    const char *msg;
    clap_namespace_get_string(ns, "message", &msg);
}
```

### Mutually Exclusive Groups

```c
int group = clap_add_mutually_exclusive_group(parser, false);

clap_argument_t *verbose = clap_add_argument(parser, "--verbose");
clap_argument_action(verbose, CLAP_ACTION_STORE_TRUE);
clap_mutex_group_add_argument(parser, group, verbose);

clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
clap_mutex_group_add_argument(parser, group, quiet);
```

### Argument Groups

Organize related arguments into named sections in the help output:

```c
int net_group = clap_add_argument_group(parser, "Network", "Connection options");

clap_argument_t *host = clap_add_argument(parser, "--host");
clap_argument_help(host, "Hostname to connect to");
clap_argument_group_add_argument(parser, net_group, host);

clap_argument_t *port = clap_add_argument(parser, "--port");
clap_argument_type(port, "int");
clap_argument_help(port, "Port number");
clap_argument_group_add_argument(parser, net_group, port);
```

Help output:

```
usage: prog [-h] [--host HOST] [--port PORT]

Optional arguments:
  -h, --help  Show this help message and exit

Network:
  Connection options

  --host HOST  Hostname to connect to
  --port PORT  Port number
```

Arguments in a display group are excluded from the default "Optional arguments" and "Positional arguments" sections, appearing only in their group section.

### Custom Memory Allocator

```c
void* my_malloc(size_t size) { return pool_alloc(size); }
void my_free(void *ptr) { pool_free(ptr); }

int main() {
    clap_set_allocator(my_malloc, my_free, NULL);
    // ... use libclap ...
}
```

## Frequently Asked Questions

### How do I handle CLAP_PARSE_HELP / CLAP_PARSE_VERSION?

These are not errors.  The library has already printed the help/version
text to stdout.  The caller should clean up and exit successfully:

```c
if (result == CLAP_PARSE_HELP || result == CLAP_PARSE_VERSION) {
    clap_parser_free(parser);
    return 0;
}
```

### Why does my custom type handler return "Unknown type"?

You registered a type name via `clap_argument_type(arg, "ip_addr")`
but forgot to register the converter with `clap_register_type(parser,
"ip_addr", my_handler, sizeof(my_ip_t))`.  The type handler must be
registered on the parser before parsing begins.

### What is CLAP_STATIC and when do I need it?

When linking libclap **statically** (e.g. `libclap.a` on Unix or
`clap.lib` on Windows), you **must** define `CLAP_STATIC` at compile
time (`-DCLAP_STATIC` or `#define CLAP_STATIC`).  This is required
because libclap's headers use `__declspec(dllimport)` on Windows by
default, which is incompatible with a static library.  Shared library
users do **not** need this define.

### My --verbose/-v flag doesn't consume a value, but the next argument is skipped

This is expected.  Flags using `CLAP_ACTION_STORE_TRUE`, `STORE_FALSE`,
`STORE_CONST`, `APPEND_CONST`, or `COUNT` do **not** consume a value.
If you need a flag that takes a value, use `CLAP_ACTION_STORE` (the
default) with `clap_argument_type()`.

### Why is my subcommand's help not showing the right name?

Subcommand names are extracted from the parser's `prog_name`.  When
creating a subparser with `clap_subparser_add(subparsers, "commit", ...)`,
the subparser's `prog_name` is set to `"parent_prog commit"`.  The
display logic extracts the portion after the last space, so names
with spaces (e.g. `"myprog nested sub"`) will only show `"sub"`.

### How do I make a positional argument optional?

Use `clap_argument_nargs(arg, '?')` (zero or one), `'*'` (zero or
more), or set `CLAP_NARGS_REMAINDER`.  By default, positional
arguments are required.

## Testing

libclap includes a comprehensive test suite built with Unity.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
ctest                          # Run all tests
ctest -R usage                 # Run only usage compliance tests
ctest --output-on-failure      # Show output for failed tests
./tests/test_parser            # Run a specific test
```

Memory safety is verified with deterministic runtime checks and sanitizers:

```bash
valgrind --leak-check=full ./tests/test_usage
cmake .. -DCLAP_ENABLE_ASAN=ON && make && ctest
```

The Valgrind command above runs the `test_usage` executable under a leak checker.
It is most useful on Linux; for regular development and CI, ASan is usually faster and easier to integrate.

Code coverage requires `gcovr` to be installed:

```bash
pip install gcovr
cmake .. -DCLAP_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
make && ctest
make coverage
```

Fuzz testing is available with Clang builds:

```bash
mkdir build && cd build
cmake .. -DCLAP_BUILD_FUZZ=ON -DCMAKE_C_COMPILER=clang
make fuzz_clap
./tests/fuzz_clap -max_len=4096 -runs=100000 corpus/
```

Unlike the fixed test suite, fuzzing continuously generates and mutates inputs to find crashes,
unexpected exits, leaks, and other parser edge cases.

## 📖 Examples

| Example | Description |
|---------|-------------|
| `examples/basic.c` | Minimal argument parsing |
| `examples/git_style.c` | Full git‑style CLI with subcommands |
| `examples/action_demo.c` | Demonstrates all 10 action types |

## 🏗️ Project Structure

```
libclap/
├── CMakeLists.txt
├── cmake/                    # CMake modules
├── include/clap/             # Public headers
├── src/                      # Library source
├── tests/                    # Unit and integration tests
│   ├── unit/                 # Per-module unit tests
│   ├── integration/          # End-to-end tests
│   └── fuzz/                 # Fuzz testing
├── examples/                 # Example programs
└── scripts/                  # Test and utility scripts
```

## 🤝 Contributing

Contributions are welcome! Please open an issue or pull request on GitHub. Ensure that:

- Code follows the existing C99 style
- All tests pass (`ctest`)
- No new compiler warnings are introduced
- Valgrind/ASan reports no leaks
- Parser and tokenizer changes are validated with fuzzing when possible

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and release notes.

## License

libclap is released under the **MIT License**. See [LICENSE](LICENSE) for details.

---

**libclap** – *argparse for C, without compromise.*
