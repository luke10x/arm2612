#pragma once

// =============================================================================
// YM2612 Patch Structure (simplified ymfm-compatible format)
// =============================================================================
struct YM2612Operator
{
    int DT;   // Detune
    int MUL;  // Multiplier
    int TL;   // Total Level
    int RS;   // Rate Scaling
    int AR;   // Attack Rate
    int AM;   // AM Enable
    int DR;   // Decay Rate
    int SR;   // Sustain Rate
    int SL;   // Sustain Level
    int RR;   // Release Rate
    int SSG;  // SSG-EG
};

struct YM2612Patch
{
    int ALG;  // Algorithm
    int FB;   // Feedback
    int AMS;  // AM Sensitivity
    int FMS;  // FM Sensitivity
    
    YM2612Operator op[4];
};

// =============================================================================
// Built-in Patches
// =============================================================================

static constexpr YM2612Patch PATCH_SLAP_BASS =
{
    .ALG = 4,
    .FB  = 5,
    .AMS = 2,
    .FMS = 3,
    
    .op =
    {
        {3, 1, 34, 0, 31, 0, 10, 6, 4, 7, 0},
        {0, 2, 18, 1, 25, 0, 12, 5, 5, 6, 0},
        {0, 1,  0, 0, 31, 0,  6, 3, 6, 5, 0},
        {0, 1,  0, 0, 31, 0,  7, 2, 5, 5, 0}
    }
};

static constexpr YM2612Patch PATCH_SYNTH_BASS =
{
    .ALG = 5,
    .FB  = 7,
    .AMS = 0,
    .FMS = 4,
    
    .op =
    {
        {0, 1, 20, 0, 31, 0, 15, 7, 3, 8, 0},
        {0, 1, 15, 0, 28, 0, 12, 6, 4, 7, 0},
        {0, 0,  0, 0, 31, 0,  0, 0, 0, 0, 0},
        {0, 0,  0, 0, 31, 0,  0, 0, 0, 0, 0}
    }
};

static constexpr YM2612Patch PATCH_ELECTRIC_BASS =
{
    .ALG = 4,
    .FB  = 6,
    .AMS = 1,
    .FMS = 2,
    
    .op =
    {
        {2, 1, 28, 0, 31, 0, 12, 5, 4, 6, 0},
        {0, 1, 22, 0, 26, 0, 10, 4, 5, 5, 0},
        {0, 1,  0, 0, 31, 0,  5, 3, 6, 4, 0},
        {0, 0,  0, 0, 31, 0,  0, 0, 0, 0, 0}
    }
};

static constexpr YM2612Patch PATCH_ACOUSTIC_BASS =
{
    .ALG = 2,
    .FB  = 3,
    .AMS = 0,
    .FMS = 1,
    
    .op =
    {
        {1, 1, 24, 0, 30, 0,  8, 4, 3, 5, 0},
        {0, 2, 16, 1, 24, 0, 10, 5, 4, 6, 0},
        {0, 1, 12, 0, 28, 0,  6, 3, 5, 4, 0},
        {0, 0,  0, 0, 31, 0,  0, 0, 0, 0, 0}
    }
};

// Array of all patches with metadata
struct PatchEntry
{
    const char* name;
    const YM2612Patch* patch;
};

static constexpr PatchEntry kBuiltInPatches[] = 
{
    { "Slap Bass",     &PATCH_SLAP_BASS },
    { "Synth Bass",    &PATCH_SYNTH_BASS },
    { "Electric Bass", &PATCH_ELECTRIC_BASS },
    { "Acoustic Bass", &PATCH_ACOUSTIC_BASS },
};

static constexpr int kNumBuiltInPatches = sizeof(kBuiltInPatches) / sizeof(kBuiltInPatches[0]);
