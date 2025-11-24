# PreWGSL - Universal preprocessor for WGSL Shaders

This library provides a way to preprocess WGSL shader code with features like file inclusion (native only), macro definitions, and conditional compilation. It is inspired by the C/C++ preprocessor but tailored for WGSL. It is written in C++ and can be used in native applications as well as in web applications via WebAssembly.


## Features

- Support for:
  - `#include` - Include other shader files, currently native only
  - `#ifdef` / `#ifndef` - Conditional compilation
  - `#if` / `#elif` / `#else` - Expression-based conditions
    - Expressions can use boolean logic and integer arithmetic, as well a a special `defined(MACRO_NAME)` operator
  - `#define` - Define macros with/without values
    - Macro expansion in code
      - Note: WGSL uses syntax like `4u` for type suffixes, but this preprocessor will not expand a macro like `MACRO_NAMEu`.
    - Pass macros globally or per-shader process call

## Native

### Installation

Include `include/pre-wgsl.hpp` as a header-only library in your C++ project. Just copy the file or see `examples/cli` for CMake integration.

### Usage

```cpp
#include "pre_wgsl.hpp"

Preprocessor preprocessor;
std::vector<std::string> macros = {"MY_MACRO=42"};
std::string shaderCode = R"(
  @compute @workgroup_size(1)
  fn main() {
    let value = {{MY_MACRO}}u;
  }
)";
std::string processed = preprocessor.preprocess(shaderCode, macros);
```

For a full demo see `examples/cli`.

## Browser / Node.js

### Installation

Via NPM:

```bash
npm install pre-wgsl
```

### Usage

```javascript
import { createPreprocessor } from 'pre-wgsl';
const preprocessor = await createPreprocessor({
  macros: ['MY_MACRO=42']
});
const source = `
  @compute @workgroup_size(1)
  fn main() {
    let value = {{MY_MACRO}}u;
  }
`;
const processed = preprocessor.preprocess(source);
```

For a full demo see `examples/web`.

## Why another WGSL preprocessor?

WGSL made the conscious decision to not include a preprocessor in the core language specification (at least for now): https://github.com/gpuweb/gpuweb/issues/568. Without getting into the pros and cons of this decision, this has led to the development of several open-source third-party preprocessors. Likely, many projects implement custom preprocessing solutions within their codebases as well.

To the best of my knowledge, here are the existing open-source WGSL preprocessors, and extensions to the WGSL language:

- [wgsl-preprocessor](https://github.com/toji/wgsl-preprocessor): A JavaScript-based preprocessor with basic conditional preprocessing, uses template literals for macro expansion.
- [wgsl-template](https://github.com/fs-eire/wgsl-template): Another JavaScript-based preprocessor which supports different syntax for macros, and also can generate C++ code to embed processed WGSL shaders.
- [WESL](https://github.com/wgsl-tooling-wg/wesl-spec): An extended version of WGSL with support for imports, conditional translation by extending WGSL's "@" attributes. Also supports packaging shader libraries for reuse.
- [wgsl_preprocessor](https://github.com/elyshaffir/wgsl_preprocessor): A Rust-based preprocessor with support for includes and macros.

However, none of these worked for my use case, which was to be able to define many variants of WGSL shaders for my work on [llama.cpp](https://github.com/reeselevine/llama.cpp), which is a C++ project. And I did not want to have to depend on a JavaScript or Rust toolchain to preprocess my shaders.

The other benefit to a C++ preprocessor is that the same code can be compiled to WebAssembly and used in web applications too. Of course, this could be done with Rust, but since llama.cpp and Dawn are C++ projects, I wanted a C++ solution. I think the competition between dawn vs. wgpu, llama.cpp vs. [candle](https://github.com/huggingface/candle)/[burn](https://github.com/tracel-ai/burn) is good, and may the best framework win :).

## Building

### Prerequisites

- CMake 3.17+

For building the WebAssembly module:
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- Node.js 16+

### Native

```bash
# Build the native library
mkdir build && cd build
cmake ..
# Run tests
tests/pre_wgsl_tests
# Or with ctest
ctest
```

### WebAssembly

```bash
# Build the WASM module and TypeScript
npm run build
```

## Detailed Functionality

### `#include "filename"`

Include another WGSL file:

```wgsl
#include "common.wgsl"
#include "utils.wgsl"
```

### `#define NAME [value]`

Define a macro:

```wgsl
#define PI 3.14159
#define WORKGROUP_SIZE 256
#define ENABLED
```

### `#ifdef` / `#ifndef`

Conditional compilation based on whether a macro is defined:

```wgsl
#define DEBUG

#ifdef DEBUG
    // This code is included
#endif

#ifndef RELEASE
    // This code is included
#endif
```

### `#if` / `#elif` / `#else`

Expression-based conditions:

```wgsl
#define VERSION 2

#if VERSION == 1
    // Version 1 code
#elif VERSION == 2
    // Version 2 code
#else
    // Other versions
#endif
```

Supported operators: `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `+`, `-`, `*`, `/`, `%`, `!`, `<<`, `>>`

### `defined(NAME)`

Check if a macro is defined in expressions:

```wgsl
#if defined(FEATURE_A) && defined(FEATURE_B)
    // Both features enabled
#endif
```

## License

MIT

## Contributing

Contributions are welcome! Feel free to open an issue/PR.
