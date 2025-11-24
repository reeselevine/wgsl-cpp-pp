// Import the preprocessor
// Default: Uses the published npm package from CDN
 import { createPreprocessor } from 'https://unpkg.com/pre-wgsl@latest/dist/index.js';
// For local/standalone deployment: uses dist files in this directory. Comment the line above and uncomment this (after building and copying to examples/web):
// import { createPreprocessor } from './dist/index.js';

let preprocessor = null;
let macroCounter = 0;

// Example shaders
const examples = [
    {
        name: "Basic Compute Shader",
        code: `#define WORKGROUP_SIZE 256
#define PI 3.14159

@compute @workgroup_size(WORKGROUP_SIZE, 1, 1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    let angle = f32(global_id.x) * PI / 180.0;
}`,
        macros: []
    },
    {
        name: "Conditional Compilation",
        code: `#define USE_OPTIMIZED_PATH
#define VERSION 2

#ifdef USE_OPTIMIZED_PATH
@compute @workgroup_size(256, 1, 1)
#else
@compute @workgroup_size(64, 1, 1)
#endif
fn compute_shader() {
    #if VERSION >= 2
        // Use version 2 features
        const optimized = true;
    #else
        // Fallback code
        const optimized = false;
    #endif
}`,
        macros: []
    },
    {
        name: "Quality Presets",
        code: `#ifdef QUALITY_HIGH
const SAMPLE_COUNT = 16;
#elif defined(QUALITY_MEDIUM)
const SAMPLE_COUNT = 8;
#else
const SAMPLE_COUNT = 4;
#endif

@fragment
fn main() -> @location(0) vec4<f32> {
    // Use SAMPLE_COUNT samples
    return vec4<f32>(1.0);
}`,
        macros: ['QUALITY_MEDIUM']
    },
    {
        name: "Complex Expressions",
        code: `#define MAX_LIGHTS 8
#define USE_SHADOWS
#define USE_PBR

#if defined(USE_SHADOWS) && defined(USE_PBR)
    #define ADVANCED_LIGHTING
    const BUFFER_SIZE = MAX_LIGHTS * 64;
#else
    const BUFFER_SIZE = MAX_LIGHTS * 32;
#endif

struct Light {
    position: vec3<f32>,
    color: vec3<f32>,
}

@group(0) @binding(0)
var<uniform> lights: array<Light, MAX_LIGHTS>;

@fragment
fn main() -> @location(0) vec4<f32> {
    #ifdef ADVANCED_LIGHTING
        // Advanced lighting code
        var result = vec3<f32>(0.0);
        for (var i = 0u; i < MAX_LIGHTS; i++) {
            result += lights[i].color;
        }
        return vec4<f32>(result, 1.0);
    #else
        // Simple lighting
        return vec4<f32>(1.0);
    #endif
}`,
        macros: []
    }
];

let currentExampleIndex = 0;

// Initialize the app
async function init() {
    try {
    showStatus('Initializing preprocessor...', 'info');
    preprocessor = await createPreprocessor();
    showStatus('Preprocessor ready!', 'success');

    // Hide status after a moment
    setTimeout(() => hideStatus(), 2000);

    // Set inputCode to first example's code
    document.getElementById('inputCode').value = examples[0].code;

    setupEventListeners();
    } catch (error) {
        showError(`Failed to initialize preprocessor: ${error.message}`);
    }
}

function setupEventListeners() {
    document.getElementById('processBtn').addEventListener('click', processShader);
    document.getElementById('clearBtn').addEventListener('click', clearAll);
    document.getElementById('loadExample').addEventListener('click', loadNextExample);
    document.getElementById('addMacro').addEventListener('click', () => addMacro());
    document.getElementById('copyBtn').addEventListener('click', copyOutput);

    // Allow Ctrl/Cmd+Enter to process
    document.getElementById('inputCode').addEventListener('keydown', (e) => {
        if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
            e.preventDefault();
            processShader();
        }
    });
}

function addMacro(value = '', skipIfExists = false) {
    const macrosList = document.getElementById('macrosList');

    // Check if we should skip if there are already macros
    if (skipIfExists && macrosList.children.length > 0) {
        return;
    }

    const macroId = macroCounter++;
    const macroItem = document.createElement('div');
    macroItem.className = 'macro-item';
    macroItem.dataset.macroId = macroId;

    macroItem.innerHTML = `
        <input type="text"
               placeholder="MACRO_NAME=value"
               value="${value}"
               data-macro-id="${macroId}">
        <button class="btn btn-secondary btn-small" onclick="removeMacro(${macroId})">×</button>
    `;

    macrosList.appendChild(macroItem);
}

// Make removeMacro available globally
window.removeMacro = function(macroId) {
    const macroItem = document.querySelector(`[data-macro-id="${macroId}"]`);
    if (macroItem) {
        macroItem.remove();
    }
}

function getMacros() {
    const macroInputs = document.querySelectorAll('.macro-item input');
    const macros = [];

    macroInputs.forEach(input => {
        const value = input.value.trim();
        if (value) {
            macros.push(value);
        }
    });

    return macros;
}

function processShader() {
    if (!preprocessor) {
        showError('Preprocessor not initialized');
        return;
    }

    const inputCode = document.getElementById('inputCode').value;
    const macros = getMacros();
    const output = document.getElementById('output');
    const errorDiv = document.getElementById('error');

    // Clear previous output
    errorDiv.classList.add('hidden');
    output.classList.remove('success');

    try {
        // Only pass macros if there are any, otherwise pass undefined
        const result = macros.length > 0
            ? preprocessor.preprocess(inputCode, macros)
            : preprocessor.preprocess(inputCode);
        output.textContent = result;
        output.classList.add('success');

        // Remove placeholder class if present
        const placeholder = output.querySelector('.placeholder');
        if (placeholder) {
            placeholder.remove();
        }

        showStatus('Shader processed successfully!', 'success');
        setTimeout(() => hideStatus(), 2000);
    } catch (error) {
        showError(`Preprocessing failed: ${error.message}`);
        output.textContent = '';
        output.innerHTML = '<div class="placeholder">Fix the error and try again...</div>';
    }
}

function clearAll() {
    document.getElementById('inputCode').value = '';
    document.getElementById('macrosList').innerHTML = '';
    document.getElementById('output').innerHTML = '<div class="placeholder">Processed shader will appear here...</div>';
    document.getElementById('error').classList.add('hidden');
    addMacro('', true); // Add one empty macro input
}

function loadNextExample() {
    const example = examples[currentExampleIndex];

    document.getElementById('inputCode').value = example.code;

    // Clear and add example macros
    document.getElementById('macrosList').innerHTML = '';
    if (example.macros.length > 0) {
        example.macros.forEach(macro => addMacro(macro));
    } else {
        addMacro('', true);
    }

    // Move to next example
    currentExampleIndex = (currentExampleIndex + 1) % examples.length;
}

function copyOutput() {
    const output = document.getElementById('output');
    const text = output.textContent;

    if (!text || output.querySelector('.placeholder')) {
        showStatus('No output to copy', 'error');
        return;
    }

    navigator.clipboard.writeText(text).then(() => {
        const btn = document.getElementById('copyBtn');
        const originalText = btn.textContent;
        btn.textContent = '✓ Copied!';
        btn.style.background = 'var(--success)';

        setTimeout(() => {
            btn.textContent = originalText;
            btn.style.background = '';
        }, 2000);
    }).catch(err => {
        showError('Failed to copy to clipboard');
    });
}

function showError(message) {
    const errorDiv = document.getElementById('error');
    errorDiv.textContent = message;
    errorDiv.classList.remove('hidden');
}

function showStatus(message, type) {
    // For now, just use the error div for all status messages
    const errorDiv = document.getElementById('error');
    errorDiv.textContent = message;
    errorDiv.style.backgroundColor = type === 'success' ? 'rgba(16, 185, 129, 0.1)' :
                                     type === 'info' ? 'rgba(99, 102, 241, 0.1)' :
                                     'rgba(239, 68, 68, 0.1)';
    errorDiv.style.borderColor = type === 'success' ? 'var(--success)' :
                                  type === 'info' ? 'var(--primary-color)' :
                                  'var(--error)';
    errorDiv.style.color = type === 'success' ? 'var(--success)' :
                           type === 'info' ? 'var(--primary-color)' :
                           'var(--error)';
    errorDiv.classList.remove('hidden');
}

function hideStatus() {
    const errorDiv = document.getElementById('error');
    errorDiv.classList.add('hidden');
    errorDiv.style.backgroundColor = '';
    errorDiv.style.borderColor = '';
    errorDiv.style.color = '';
}

// Start the app
init();
