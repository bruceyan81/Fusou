// helpers/paletteBridge.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const paletteHelpers = options.paletteHelpers;
    const brushColorHelpers = options.brushColorHelpers;
    const brushColorRuntime = options.brushColorRuntime;

    if (!paletteHelpers) {
      throw new Error('paletteHelpers is required');
    }
    if (!brushColorHelpers) {
      throw new Error('brushColorHelpers is required');
    }
    if (!brushColorRuntime) {
      throw new Error('brushColorRuntime is required');
    }

    function buildExportPaletteFromUI(paletteConfig) {
      return paletteHelpers.buildExportPaletteFromUI(
        paletteConfig,
        brushColorRuntime.getBrushColors()
      );
    }

    function applyImportedPaletteToBrushColors(importedPalette) {
      brushColorHelpers.applyImportedPaletteToBrushColors(
        importedPalette,
        brushColorRuntime.getBrushColors()
      );
    }

    function resetBrushColorsToBaseForEntries(paletteConfig) {
      brushColorHelpers.resetBrushColorsToBaseForEntries(
        paletteConfig,
        brushColorRuntime.getBrushColors(),
        brushColorRuntime.getBaseBrushColors()
      );
    }

    function reconcileBrushColors(paletteConfig, fillMissing) {
      brushColorHelpers.reconcileBrushColors(
        paletteConfig,
        brushColorRuntime.getBrushColors(),
        fillMissing
      );
    }

    return {
      buildExportPaletteFromUI,
      applyImportedPaletteToBrushColors,
      resetBrushColorsToBaseForEntries,
      reconcileBrushColors
    };
  }

  window.appHelpersPaletteBridge = {
    create
  };
})();
