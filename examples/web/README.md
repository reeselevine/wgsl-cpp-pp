# PreWGSL Web Example

This is an interactive web application that demonstrates the PreWGSL preprocessor. It allows you to test WGSL shader preprocessing with different inputs and macro definitions in real-time.

## Setup for Local Development or Deployment

By default the web example uses the PreWGSL package from a CDN. However, you can also set it up to use a locally built version of the PreWGSL WebAssembly module. See `app.js` (top of file) for details on how to switch between the two modes.

### Step 1: Build the Project

First, build the WebAssembly module from the project root:

```bash
# From the wasm/ directory
npm install
npm run build
```

### Step 2: Setup Standalone Web Example

Run the setup script to copy the built files into the web example directory:

```bash
# From the project root
cd examples/web
bash setup-standalone.sh
```

This creates a `dist/` folder inside `examples/web/` with all necessary files.

### Step 3: Test Locally

```bash
# From examples/web directory
# Assuming you have http-server installed (npm install -g http-server)
http-server -p 8080
```

Then open `http://localhost:8080/` in your browser.

## Deploying as a Static Site

After running the setup script, the `examples/web/` directory contains everything needed for deployment:

- `index.html`
- `app.js`
- `styles.css`
- `dist/` (with WASM and JS files)
- `README.md`

### Deployment Options

#### Option 1: GitHub Pages

1. Create a new repository or use an existing one
2. Copy the contents of `examples/web/` to your repository
3. Enable GitHub Pages in repository settings
4. Select the branch and root directory
5. Access at `https://yourusername.github.io/repo-name/`

## Examples Included

1. **Basic Compute Shader**: Simple macro substitution
2. **Conditional Compilation**: Using `#ifdef` and `#if` directives
3. **Quality Presets**: Different rendering quality levels using macros
4. **Complex Expressions**: Advanced usage with multiple features
