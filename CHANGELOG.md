# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.7.0] - 2025-11-15

### Added
- Target triple parsing for cross-compilation support
- Library build mode with lib.mach entrypoint support
- Static and shared library generation for library targets
- Compile-time context initialization from target triples

### Changed
- Library mode now prefers lib.mach as entrypoint before falling back to all source files
- Target OS and architecture now properly set from target triple in project builds

### Fixed
- Cross-compilation now correctly uses target triple for compile-time constants

## [0.6.8] - 2025-11-14

### Added
- Dependency management commands (add, remove, list, update)
- Version specification support for dependencies
- Dependency validation and cleanup features

### Changed
- Migrated std library to standalone mach-std repository
- Enhanced configuration handling for dependencies

## [0.6.7] - 2024-11-14

### Fixed
- Function pointer handling inside records with proper symbol mapping
- Missing TOKEN_EQUAL check for function types in val/var declarations

## [0.6.6] - 2024-11-01

### Changed
- Renamed 'new' command to 'init' for project creation
- Updated LLVM requirement to version 19+

### Fixed
- Windows config path formatting
- Command usage and project initialization

## [0.6.5] - 2024-10-15

### Fixed
- Specialization caching to prevent cycles during field resolution
- ASCII helper functions to use char type consistently

## [0.6.4] - 2024-10-01

### Added
- Unary expression hint support

### Changed
- Refactored time and memory management modules

### Fixed
- Binary expression analysis and method lookup for type aliases
- Windows output path in configuration

## [0.6.3] - 2024-09-15

### Added
- Windows build system with bootstrap compiler targets
- Windows-specific link libraries
- Windows path resolution and library parsing

### Fixed
- Windows console handle mapping and performance variables

## [0.6.2] - 2024-09-01

### Changed
- Reorganized imach structure with stdlib improvements

### Fixed
- Type handling in binary expression analysis
- ASCII utility functions

## [0.6.1] - 2024-08-15

### Added
- Getting started documentation for building from source

### Fixed
- Unwrap method usage and memory handling in filesystem/path functions

## [0.6.0] - 2024-08-01

### Added
- Platform-specific runtime support (Linux, Darwin, Windows)
- Platform-specific console I/O, environment, and memory management
- String character search functions (index_of_char, last_index_of_char)
- Compile-time intrinsics (min, max, iota)
- Parser stage and AST integration
- TOML table header parsing

### Changed
- Unified string type usage across codebase (str instead of string)
- Refactored memory allocation to use Option type
- Enhanced generic method lookup

### Fixed
- Lexer position line offset handling for newlines
- Comptime expression handling
- Memory zeroing in FileArena and FileInfo destructors

## [0.5.0] - 2024-07-01

### Added
- Lexer and token definitions
- JSON and TOML parsing support
- Compilation unit structure and pipeline stages

### Changed
- Enhanced debug information handling with C99 support
- Improved constant evaluation and propagation
- Enhanced varargs code generation and pointer handling

### Fixed
- Project ID and name formatting in mach.toml

## [0.4.2] - 2024-06-15

### Added
- usize type for sizes across standard library

## [0.4.1] - 2024-06-01

### Changed
- Rebuilt documentation into improved structure

## [0.4.0] - 2024-05-15

### Added
- Module alias support for type names
- offset_of and type_of intrinsic functions
- Receiver-first method syntax support

### Changed
- Switched to bracket syntax for generics (from angle brackets)
- Switched to [] for slices (dropped unbounded-array syntax)
- Updated stdlib to use new generic syntax
- Compilation context now initialized with build options

### Fixed
- Error messages to use $if instead of $when

## [0.3.3] - 2024-05-01

### Fixed
- Consolidated function declaration logic to prevent LLVM optimizations

## [0.3.2] - 2024-04-25

### Fixed
- Prevented LLVM from optimizing code into library calls via no-builtins attribute

## [0.3.1] - 2024-04-15

### Changed
- Minor improvements and fixes

## [0.3.0] - 2024-04-01

### Changed
- Refactored str keyword to rec (struct renamed to record)

### Fixed
- Token documentation improvements

## [0.2.0] - 2024-03-01

### Added
- Enhanced build system with artifact path handling
- Project configuration improvements
- Filesystem utilities

### Changed
- Documentation overhaul
- Updated project structure for build artifacts

## [0.1.0] - 2024-02-01

### Added
- Initial repository setup
- Bootstrap compiler implementation
- Basic language features and standard library
