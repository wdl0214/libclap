# libclap

**Command Line Argument Parser for C**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-99-standard.svg)](https://en.wikipedia.org/wiki/C99)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

libclap is a modern, secure, and extensible command-line argument parsing library for C, inspired by Python's `argparse`. It provides a familiar, feature-rich API while maintaining C99 compatibility, zero external dependencies, and a strong focus on memory safety.

## ✨ Features

- **Full argparse Compatibility** – Positional arguments, optional flags, subcommands, mutually exclusive groups, `nargs` (`*`, `+`, `?`, `N`, `REMAINDER`), and more.
- **Rich Action System** – `store`, `store_true/false`, `store_const`, `append`, `append_const`, `count`, `help`, `version`, and custom actions.
- **Type Conversion** – Built-in support for `int`, `float`, `string`, and `bool`, plus user‑defined converters.
- **Automatic Help Generation** – Beautifully formatted usage and help messages with smart line‑wrapping and alignment.
- **Subcommands** – Full support for git‑style sub‑parsers with flat namespace merging.
- **Memory Safe** – Arena allocator, bounds‑checked string buffers, input validation, and comprehensive leak testing (Valgrind/ASan clean).
- **Cross‑Platform** – Windows, Linux, macOS, and BSD. Works with GCC, Clang, and MSVC.
- **Zero Dependencies** – Only requires a C99 standard library.
- **CMake Integration** – Easy to build and consume with `find_package(clap)` or `pkg-config`.

## 📦 Installation

### From Source

```bash
git clone https://github.com/yourname/libclap.git
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

#### CMake `find_package`

```cmake
find_package(clap REQUIRED)

add_executable(myapp myapp.c)
target_link_libraries(myapp PRIVATE clap::clap)
```

#### pkg-config

```bash
gcc myapp.c -o myapp $(pkg-config --cflags --libs libclap)
```

#### Manual Linking

```bash
gcc myapp.c -o myapp -I/usr/local/include -L/usr/local/lib -lclap
```

## 🚀 Quick Start

```c
#include <clap/clap.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    clap_argument_t *arg;
    clap_parser_t *parser = clap_parser_new("my_program", "A simple example", NULL);
    
    arg = clap_add_argument(parser, "input");
    clap_argument_help(arg, "Input file");
    clap_argument_required(arg, true);
    
    arg = clap_add_argument(parser, "--output/-o");
    clap_argument_help(arg, "Output file");
    clap_argument_default(arg, "-");
    
    arg = clap_add_argument(parser, "--verbose/-v");
    clap_argument_help(arg, "Increase verbosity");
    clap_argument_action(arg, CLAP_ACTION_COUNT);

    clap_namespace_t *ns;
    clap_error_t error = {0};
    
    if (!clap_parse_args(parser, argc, argv, &ns, &error)) {
        fprintf(stderr, "Error: %s\n", error.message);
        return 1;
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

clap_namespace_t *ns;
clap_error_t error;
bool ok = clap_parse_args(parser, argc, argv, &ns, &error);

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
clap_add_argument(commit, "-m")
clap_argument_required(commit, true);

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
clap_argument_group(verbose, group);
clap_mutex_group_add_argument(parser, group, verbose);

clap_argument_t *quiet = clap_add_argument(parser, "--quiet");
clap_argument_action(quiet, CLAP_ACTION_STORE_TRUE);
clap_argument_group(quiet, group);
clap_mutex_group_add_argument(parser, group, quiet);
```

### Custom Memory Allocator

```c
void* my_malloc(size_t size) { return pool_alloc(size); }
void my_free(void *ptr) { pool_free(ptr); }

int main() {
    clap_set_allocator(my_malloc, my_free, NULL);
    // ... use libclap ...
}
```

## 🧪 Testing

libclap includes a comprehensive test suite built with Unity.

```bash
cd build
ctest                          # Run all tests
ctest -R usage                 # Run only usage compliance tests
ctest --output-on-failure      # Show output for failed tests
./tests/test_parser            # Run a specific test
```

Memory safety is verified with Valgrind and AddressSanitizer:

```bash
valgrind --leak-check=full ./tests/test_usage
cmake .. -DCLAP_ENABLE_ASAN=ON && make && ctest
```

Code coverage:

```bash
cmake .. -DCLAP_ENABLE_COVERAGE=ON
make && ctest
make coverage
```

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
└── docs/                     # Documentation
```

## 🤝 Contributing

Contributions are welcome! Please open an issue or pull request on GitHub. Ensure that:

- Code follows the existing C99 style
- All tests pass (`ctest`)
- No new compiler warnings are introduced
- Valgrind/ASan reports no leaks

## 📄 License

libclap is released under the **MIT License**. See [LICENSE](LICENSE) for details.

---

**libclap** – *argparse for C, without compromise.*
