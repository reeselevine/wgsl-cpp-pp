// Example WGSL compute shader demonstrating preprocessor directives

// Define some configuration variables
#define USE_NORMALIZATION 1
#define DEBUG_MODE 0
#define OPERATION_MODE 2  // 0=multiply, 1=add, 2=transform
#define PI 3.14159

#include "utils.wgsl"

@group(0) @binding(0) var<storage, read> input_data: array<f32>;
@group(0) @binding(1) var<storage, read_write> output_data: array<f32>;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    let index = global_id.x;

    // Read input value
    let value = input_data[index];

    #if OPERATION_MODE == 0
        // Multiply mode
        var result = value * PI;
    #elif OPERATION_MODE == 1
        // Add mode
        var result = value + 10.0;
    #else
        // Transform mode
        var result = applyTransform(value, 1.5);
    #endif

    #if USE_NORMALIZATION == 1
        // Normalize to range [0, 1]
        result = normalizeValue(result, 0.0, 100.0);
    #else
        result = result;
    #endif

    #if DEBUG_MODE == 1
        // Debug mode
        output_data[index] = debugValue(result, index);
    #else
        output_data[index] = result;
    #endif
}
