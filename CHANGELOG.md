# Changelog

All notable changes to this project are documented in this file.

## [1.1.0] - 2026-05-10

### Added
- Added callback-based i18n system: `clap_set_translator()` for translating all user-facing strings (help, errors, usage, warnings, version).
- Added `CLAP_TR()` macro wrapping all user-facing strings across all source files.
- Added `make pot` CMake target for regenerating `po/libclap.pot` via xgettext with `--from-code=UTF-8`.
- Added `.pot` template installation to `${prefix}/share/libclap/libclap.pot`.
- Added `clap_i18n.h` public header with Doxygen-compatible API documentation and usage examples.
- Added `clap_print_usage()` and `clap_print_help_on_error()` for programmatic help output.
- Added `clap_parser_set_help()` to disable auto-generated `--help` (`add_help=False`).
- Added `clap_parser_set_usage()` for custom usage strings.
- Added ANSI color support with `clap_parser_set_color()`, auto-detecting TTY and respecting `NO_COLOR`/`FORCE_COLOR`.
- Added argument deprecation support with `clap_argument_deprecated()` and `clap_argument_deprecated_with_reason()`.

### Changed
- Refactored `clap_action_t` and `clap_set_allocator()` into core headers for better organization.
- Removed duplicated version macro definitions.
- Updated version output format to match argparse convention.

### Fixed
- Fixed pkg-config file generation and import library installation on MinGW.
- Wired missing `clap_validate_dependencies()` and `clap_apply_defaults()` into parse pipeline.

### Documentation
- Added i18n how-to section to README with translation table and gettext integration examples.
- Updated README to use `cmake --build` instead of bare `make`.
- Added Doxygen documentation for color types and related APIs.

### Testing
- Added unit tests for i18n translation via strerror, help, usage, and version output.
- Added null-guard, edge-case, and error-path coverage tests.
- Added integration coverage for nargs constants.

### Maintenance
- Split CI workflow into separate per-concern workflows.
- Removed obsolete test runner scripts.

## [1.0.0] - 2026-05-06

First stable release of clap, consolidating all work completed since the project was created.

### Added
- Added the core parser with positional arguments, options, subcommands, and namespace-based parse results.
- Added built-in actions for storing values, storing constants, booleans, counters, append-style accumulation, help, version, and custom handlers.
- Added built-in type conversion for strings, integers, floats, and booleans, plus custom type registration.
- Added mutually exclusive groups, argument dependencies, display groups, and user data attachment for richer argument modeling.
- Added abbreviated long-option matching, bundled short options, and flexible `nargs` support including optional, repeated, remainder, and parser forms.
- Added automatic usage and help formatting with grouping, wrapping, and alignment support.
- Added `clap_parse_result_t` return handling, structured error accessors, and helper APIs such as `clap_namespace_has()` and `clap_argument_data()`.
- Added custom allocator support, internal safe buffer utilities, symbol visibility controls, CMake package export, pkg-config metadata, and Windows DLL/static build support.
- Added comprehensive unit, integration, tokenizer, short-option bundle, fuzz, and abbreviation coverage in the test suite.
- Added CI support for coverage, sanitizer, and multi-platform validation workflows.

### Changed
- Reworked parsing internals into tokenizer, pattern, and state-machine stages to improve maintainability.
- Changed parse flow to return `clap_parse_result_t` instead of calling `exit(0)`.
- Stored float defaults as native numeric values instead of string representations.
- Propagated default conversion failures instead of silently ignoring them.
- Resolved custom type handlers from the parser registry and made the registry the single source of truth for built-in handlers.
- Reduced public API exposure by hiding internal structs and trimming implementation details from installed headers and generated docs.
- Cleaned up mutex-group API usage and display-group naming to make the public interface more consistent.
- Standardized help output formatting, including the capitalized `Usage` heading.

### Fixed
- Fixed parsing for fixed-arity positional arguments and optional arguments with fixed `nargs`.
- Fixed option value theft when multiple options shared the same destination.
- Fixed handling of negative numbers so they can be consumed as option values when appropriate.
- Fixed float merging in namespace merge paths.
- Fixed mutex-group conflict reporting and excluded auto-generated help arguments from mutex groups.
- Fixed subparser program-name extraction and display-group indentation in generated help output.
- Fixed Windows build and installation issues around `CLAP_STATIC`, symbol export/import, and packaging.
- Fixed missing POSIX feature test macros in test targets.
- Fixed a non-existent `clap_argument_depends_on` API reference.
- Fixed fuzz and sanitizer findings, including `const_value` buffer leaks, tokenized long-option leaks, and related harness cleanup.
- Added defensive NULL checks after type resolution and addressed MSVC static-analysis warnings including C4200, C6011, and C6031-related issues.
- Fixed Doxygen warnings and missing struct documentation.

### Documentation
- Reworked the README for accuracy, install guidance, examples, badges, and API usage notes.
- Added comprehensive Doxygen documentation and CI generation for API docs.
- Expanded error-handling examples and completed the `custom_type` example description.
- Added and refined documentation around display groups, Windows static linking, and general usage behavior.

### Testing and CI
- Added coverage XML generation and Codecov integration updates.
- Hardened fuzzing workflows, preserved libFuzzer exit codes, and created artifact output handling.
- Added ASan + UBSan CI coverage and fixed sanitizer job link issues.
- Added Windows-specific CI safeguards for timeout-sensitive tests.
- Removed stale coverage cache handling that caused `.gcno` CI failures.

### Maintenance
- Improved formatter implementation readability and maintainability.
- Removed unused trie and arena modules.
- Removed dead code discovered during cleanup and sanitizer work.
- Applied general build, docs, and repository hygiene fixes made during early project setup.
