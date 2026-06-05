// helpers/appInit.js
(function () {
  'use strict';

  function shouldSeedDefaultPalette(curPc) {
    if (!curPc || !Array.isArray(curPc.entries)) {
      return true;
    }

    return curPc.entries.length === 0;
  }

  function initApp(options) {
    options = options || {};

    const runtime = options.runtime || {};
    const status = options.status || {};
    const ui = options.ui || {};
    const config = options.config || {};

    const state = runtime.state || null;
    const paletteBridge = runtime.paletteBridge || null;

    const clearStatus = status.clearStatus || (() => {});
    const setStatus = status.setStatus || (() => {});

    const rebuildPaletteMap = ui.rebuildPaletteMap || (() => {});
    const rebuildBrushOptions = ui.rebuildBrushOptions || (() => {});
    const rebuildPaletteTable = ui.rebuildPaletteTable || (() => {});
    const renderAllCells = ui.renderAllCells || (() => {});
    const updateImportInfo = ui.updateImportInfo || (() => {});
    const renderRulers = ui.renderRulers || (() => {});

    const appDefaults = config.appDefaults || null;
    clearStatus();

    const builtInPc = appDefaults && appDefaults.DEFAULT_PALETTE_CONFIG
      ? appDefaults.DEFAULT_PALETTE_CONFIG
      : null;

    const curPc0 = state.getPaletteConfig();
    if (builtInPc && shouldSeedDefaultPalette(curPc0)) {
      state.replacePaletteConfig(builtInPc);
    }

    // Reconcile brush colors against the final palette shape without inventing new colors.
    paletteBridge.reconcileBrushColors(state.getPaletteConfig(), false);

    // Rebuild palette-driven UI from the reconciled palette config.
    const pc = state.getPaletteConfig();
    rebuildPaletteMap(pc);
    rebuildBrushOptions(pc);
    rebuildPaletteTable(pc);

    renderAllCells();

    const a0 = state.getAsset();
    renderRulers(a0.width | 0, a0.height | 0);

    setStatus('Ready.', 'ok');
    updateImportInfo();
  }

  window.appHelpersAppInit = {
    shouldSeedDefaultPalette,
    initApp
  };
})();
