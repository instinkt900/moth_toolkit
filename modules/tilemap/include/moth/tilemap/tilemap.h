#pragma once

// moth::tilemap — grid-based maps loaded from Tiled JSON (.tmj), rendered as
// culled, layered tiles drawn from a tileset atlas (Decisions — see roadmap 6.6).
//
// Data model (tile_map.h) and TMJ import (tile_map_loader.h) are graphics-free;
// rendering (tile_map_renderer.h) draws through moth::gfx's IGraphics.

#include "moth/tilemap/tile.h"
#include "moth/tilemap/tile_map.h"
#include "moth/tilemap/tile_map_loader.h"
#include "moth/tilemap/tile_map_renderer.h"
