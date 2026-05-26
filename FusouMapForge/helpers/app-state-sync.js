// helpers/appStateSync.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const importInfoEl = options.importInfoEl || null;

    const gridView = options.gridView || null;
    const rulerView = options.rulerView || null;
    const state = options.state || null;
    const getBrushColor = options.getBrushColor || null;

    const paletteView = options.paletteView || null;
    const paletteBridge = options.paletteBridge || null;

    const rebuildBrushOptionsOverride = options.rebuildBrushOptions || null;
    const rebuildPaletteTableOverride = options.rebuildPaletteTable || null;

    const rebuildPaletteMap =
      options.rebuildPaletteMap ||
      (gridView && typeof gridView.rebuildPaletteMap === 'function'
        ? gridView.rebuildPaletteMap
        : null);

    function updateImportInfo() {
      if (!importInfoEl) {
        return;
      }

      importInfoEl.textContent = '';
    }

    function renderAllCells() {
      if (!gridView || !state || typeof getBrushColor !== 'function') {
        return;
      }

      gridView.renderAllCells(
        state,
        getBrushColor,
        rulerView && typeof rulerView.render === 'function'
          ? rulerView.render
          : null
      );
    }

    function renderIndices(indices, values) {
      if (!gridView || typeof getBrushColor !== 'function') {
        return;
      }
      gridView.renderIndices(indices, values, getBrushColor);
    }

    function rebuildBrushOptions(paletteConfig) {
      if (typeof rebuildBrushOptionsOverride === 'function') {
        rebuildBrushOptionsOverride(paletteConfig);
        return;
      }

      if (!paletteView) {
        return;
      }
      paletteView.rebuildBrushOptions(paletteConfig);
    }

    function rebuildPaletteTable(paletteConfig) {
      if (typeof rebuildPaletteTableOverride === 'function') {
        rebuildPaletteTableOverride(paletteConfig);
        return;
      }

      if (!paletteView) {
        return;
      }

      paletteView.rebuildPaletteTable(paletteConfig, {
        reconcileBrushColors:
          paletteBridge &&
          typeof paletteBridge.reconcileBrushColors === 'function'
            ? paletteBridge.reconcileBrushColors
            : null,
        getBrushColor
      });
    }

    function handlePatch(patch) {
      if (!patch || !patch.type) {
        return false;
      }

      if (patch.type === 'resize' || patch.type === 'load' || patch.type === 'clear') {
        renderAllCells();
        return true;
      }

      if (patch.type === 'tile') {
        renderIndices(patch.indices, patch.values);
        return true;
      }

      if (patch.type === 'palette') {
        if (!state || typeof state.getPaletteConfig !== 'function') {
          return true;
        }

        const paletteConfig = state.getPaletteConfig();

        if (typeof rebuildPaletteMap === 'function') {
          rebuildPaletteMap(paletteConfig);
        }

        rebuildBrushOptions(paletteConfig);
        rebuildPaletteTable(paletteConfig);
        renderAllCells();
        return true;
      }

      return false;
    }

    return {
      updateImportInfo,
      renderAllCells,
      renderIndices,
      rebuildBrushOptions,
      rebuildPaletteTable,
      handlePatch
    };
  }

  window.appHelpersAppStateSync = {
    create
  };
  window.AppHelpersAppStateSync = window.appHelpersAppStateSync;
})();
