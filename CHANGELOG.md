# Changelog

## [1.0.0] - 2026-05-02

Initial public release.

### Added
- Core argument parser with positional, optional, and subcommand support
- 10 action types: STORE, STORE_CONST, STORE_TRUE, STORE_FALSE, APPEND, APPEND_CONST, COUNT, HELP, VERSION, CUSTOM
- Built-in type conversion for string, int, float, bool with `clap_register_type()` for custom types
- Mutually exclusive group support (`clap_add_mutually_exclusive_group`)
- Argument display groups for organizing help output (`clap_add_argument_group`)
- Inter-argument dependencies (`clap_argument_requires`, `clap_argument_conflicts`)
- Subcommand support with flat namespace merging
- Automatic help/usage generation with smart word-wrapping
- Abbreviated option matching (`clap_parser_set_allow_abbrev`)
- Short option bundling (`-abc` → `-a -b -c`)
- nargs specifiers: N, `'?'` (0/1), `'*'` (0+), `'+'` (1+), REMAINDER, PARSER
- Custom memory allocator support via `clap_set_allocator()`
- Arena allocator for internal long-lived structures (trie nodes)
- Trie-based fast option lookup
- Safe string buffer (`clap_buffer_t`) with bounds checking
- Zero-initialized memory wrappers (`clap_malloc`, `clap_calloc`)
- DLL export/import macros with static/shared library support
- pkg-config integration
- CMake `find_package(clap)` support
- Strict compiler warnings enabled for GCC, Clang, and MSVC
- ASan/UBSan/coverage optional build modes

### Changed
- Parser returns `clap_parse_result_t` enum instead of calling `exit(0)`
- Float defaults stored as native double instead of string representation
- Default conversion errors now propagate instead of silently failing
- Custom type handlers resolved from parser registry, not just built-in types
- `clap_argument_mutex_group()` deprecated — use `clap_mutex_group_add_argument()`

### Fixed
- Option value theft when multiple options share the same destination
- Mutex group conflict detection with proper error reporting
- Auto-generated help option (`-h/--help`) correctly excluded from mutex groups
- Subparser name extraction from `prog_name` for help display
- Memory leaks in fuzz testing (const_value buffers, tokenized long option names)
- Windows DLL/static build conflicts with `CLAP_STATIC` symbol handling
- Indentation of display group descriptions in help output

### Documentation
- Comprehensive README with install guide, quick start, and API reference
- Full Doxygen annotations on public API
- Examples: basic, git-style CLI, action type demonstration
- Contributing guidelines with testing requirements

### Testing
- 20 unit test files covering every source module
- 3 integration test suites for real-world scenarios and help output compliance
- libFuzzer harness with 50k-iteration CI runs
- CI matrix across Ubuntu, macOS, Windows with Debug and Release builds
