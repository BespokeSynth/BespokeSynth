# Bolt's Journal - Critical Learnings Only

## 2025-05-18 - Hoist and Cache Math Functions in Audio Processing Loops
**Learning:** In audio DSP code operating on sample buffers, functions like `powf()`, `expf()`, and `sinf()` inside per-sample inner loops (`for (int j = 0; j < bufferSize; ++j)`) cause heavy CPU overhead when reading global or member state like `gSampleRate` or `mDecayTime`. Because `gSampleRate` is an external global variable, the compiler cannot assume it remains invariant across iterations or function calls, preventing loop-invariant code motion.
**Action:** Always hoist or cache invariant computations like exponential decay coefficients (`powf(0.5f, 1.0f / (decayTime * sampleRate))`) outside the per-sample loop or inside the class state when parameters change.
