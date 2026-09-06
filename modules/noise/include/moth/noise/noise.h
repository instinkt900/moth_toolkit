#pragma once

/// @file
/// @brief Umbrella header for the moth::noise module.
///
/// Robust, node-graph-driven noise built on FastNoise2. For a quick
/// dependency-free Perlin or Simplex sample, use @c moth/core/noise.h instead —
/// this module is for applications that need the full generator graph, and it
/// carries a third-party dependency to provide it.

#include "moth/noise/node_tree.h"
