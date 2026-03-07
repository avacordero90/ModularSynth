# Modular Synth Agent Guidelines

## Build Commands
- `make` or `make all` - Compile the complete modular synth application
- `make debug` - Build with debug flags enabled
- `make clean` - Remove all compiled files
- `make run` - Compile and run the application
- `make rebuild` - Clean and rebuild the entire project

## Linting & Testing
This is a C++ project using Makefile build system. Currently no automated linting or testing commands defined.
- No specific test runner mentioned in documentation
- Manual testing via Makefile `run` target

## Code Style Guidelines

### Language & Standards
- C++17 compliant code
- Use of modern C++ features where appropriate
- All code must compile with g++ using CXXFLAGS set in Makefile

### Naming Conventions
- Class names: PascalCase (e.g., `AudioPipeline`, `WavetableManager`)
- Function names: camelCase (e.g., `initializeModules`, `processAudio`)
- Variable names: camelCase (e.g., `sampleRate`, `waveformType`)
- Constants: UPPER_CASE (e.g., `MAX_BUFFER_SIZE`, `DEFAULT_SAMPLE_RATE`)

### Formatting
- Use 4 spaces for indentation (no tabs)
- Braces on same line for function definitions and classes
- One statement per line where practical
- Space around operators: `int x = y + z;`
- No trailing whitespace
- Include necessary headers in appropriate order:
  1. Standard library headers
  2. External library headers  
  3. Project-specific headers

### Code Organization
- Follow existing modular structure in src/ directory:
  - Core audio components in `src/core/` 
  - Individual modules in `src/modules/`
  - Plugin interface in `src/plugin/`
- Each class should have clear separation of concerns
- Audio processing functions should be efficient and minimize memory allocations
- Use header files with include guards or #pragma once

### Error Handling
- All audio processing should be robust and handle edge cases
- Return appropriate error codes from functions where possible
- Use exceptions sparingly for truly exceptional conditions
- Validate input parameters at function boundaries

### Documentation
- Document all public APIs with Doxygen-style comments
- Include class descriptions, parameter explanations, and return values
- Add inline comments for complex algorithmic sections
- Maintain consistent documentation style with existing codebase

## Special Considerations
- Audio processing requires careful attention to performance
- Memory management should be efficient due to real-time constraints
- Thread safety considerations for multi-threaded audio processing
- Ensure no memory leaks in long-running application instances
- Follow the existing folder structure and naming conventions throughout

## Plugin Design Reference
Refer to `PLUGIN_DESIGN.md` for detailed plugin architecture and parameter definitions.