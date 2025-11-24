import createPreWGSLModule from './pre-wgsl.mjs';

export interface PreprocessorOptions {
  macros?: string[];
}

export interface PreprocessResult {
  output: string;
}

class PreWGSLWrapper {
  private module: any;
  private preprocessor: any;

  constructor(module: any, options: PreprocessorOptions = {}) {
    this.module = module;

    // Create a VectorString for macros
    const macrosVector = new module.VectorString();
    if (options.macros && options.macros.length > 0) {
      for (const macro of options.macros) {
        macrosVector.push_back(macro);
      }
    }

    const cppOptions: any = {
      // Note includePath is not currently supported in browser environment
      includePath: '.',
      macros: macrosVector
    };

    this.preprocessor = new module.PreWGSL(cppOptions);
  }

  /**
   * Preprocess WGSL shader source code
   * @param source The WGSL source code to preprocess
   * @param additionalMacros Optional macros for this specific preprocessing operation
   * @returns The preprocessed source code
   */
  preprocess(source: string, additionalMacros?: string[]): string {
    try {
      const macrosVector = new this.module.VectorString();
      if (additionalMacros && additionalMacros.length > 0) {
        for (const macro of additionalMacros) {
          macrosVector.push_back(macro);
        }
      }

      return this.preprocessor.preprocess(source, macrosVector);
    } catch (error) {
      throw new Error(`Preprocessing failed: ${error}`);
    }
  }

  destroy(): void {
    if (this.preprocessor) {
      this.preprocessor.delete();
      this.preprocessor = null;
    }
  }
}

let moduleInstance: any = null;

/**
 * Initialize the WGSL preprocessor module
 * This must be called before using any preprocessor functions
 */
export async function init(): Promise<void> {
  if (!moduleInstance) {
    moduleInstance = await createPreWGSLModule();
  }
}

/**
 * Create a new preprocessor instance with optional configuration
 * @param options Configuration options
 * @returns A preprocessor instance
 */
export async function createPreprocessor(
  options: PreprocessorOptions = {}
): Promise<PreWGSLWrapper> {
  await init();
  return new PreWGSLWrapper(moduleInstance, options);
}

export { PreWGSLWrapper as Preprocessor };
