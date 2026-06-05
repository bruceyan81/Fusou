// constants.js
// Constants used by the Asset-based editor.

(function (global) {
  'use strict';

  // Keep these conservative to avoid accidental huge allocations.
  const LIMITS = {
    MIN_W: 1,
    MIN_H: 1,
    MAX_W: 500,
    MAX_H: 500,
  };

  // These are used by CSS/layout (cell size etc.) in your existing HTML/CSS.
  const UI = {
    CELL_SIZE: 18,
  };

  // Asset format version (C++ loader reads it but does not branch on it yet).
  const DEFAULTS = {
    ASSET_VERSION: 2,

    // Default map size
    WIDTH: 40,
    HEIGHT: 25,

    // Resize policy values used by app.js: 'clear' | 'keepTopLeft'.
    RESIZE_POLICY: 'clear',
  };

  global.appConstAsset = {
    LIMITS,
    UI,
    DEFAULTS,
  };

})(window);
