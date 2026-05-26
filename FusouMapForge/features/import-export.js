// features/import-export.js
(function () {
  'use strict';

  /**
   * @param {Object} deps - Feature dependencies.
   * @param {Object} deps.runtime - State, IO, and palette color runtime collaborators.
   * @param {Object} deps.status - Status and diagnostic output callbacks.
   * @param {Object} deps.ui - UI refresh callbacks used after imports.
   * @param {Object} deps.session - Last imported file name storage and display sync.
   * @param {Object} deps.browser - Browser APIs used for generated downloads.
   * @param {Object} deps.controls - DOM controls owned by import/export actions.
   * @returns {Object} Import/export feature API.
   */
  function create(deps) {
    deps = deps || {};

    const runtime = deps.runtime || null;
    const status = deps.status || null;
    const ui = deps.ui || null;
    const session = deps.session || null;
    const controls = deps.controls || null;

    // State and IO perform the work; UI and session helpers react to outcomes.
    const state = (runtime && runtime.state) || deps.state;
    const ioAsset = (runtime && runtime.ioAsset) || deps.ioAsset || window.ioAsset;

    const clearStatus = (status && status.clearStatus) || deps.clearStatus;
    const setStatus = (status && status.setStatus) || deps.setStatus;
    const setDebug = (status && status.setDebug) || deps.setDebug;

    const rebuildPaletteTable = (ui && ui.rebuildPaletteTable) || deps.rebuildPaletteTable;
    const renderAllCells = (ui && ui.renderAllCells) || deps.renderAllCells;

    const buildExportPaletteFromUI =
      (runtime && runtime.buildExportPaletteFromUI) || deps.buildExportPaletteFromUI;
    const resetBrushColorsToBaseForEntries =
      (runtime && runtime.resetBrushColorsToBaseForEntries) ||
      deps.resetBrushColorsToBaseForEntries;
    const applyImportedPaletteToBrushColors =
      (runtime && runtime.applyImportedPaletteToBrushColors) ||
      deps.applyImportedPaletteToBrushColors;
    const reconcileBrushColors =
      (runtime && runtime.reconcileBrushColors) || deps.reconcileBrushColors;

    const getBrushColors = (runtime && runtime.getBrushColors) || deps.getBrushColors;
    const getBaseBrushColors = (runtime && runtime.getBaseBrushColors) || deps.getBaseBrushColors;

    const inputImportJsonFile =
      (controls && controls.inputImportJsonFile) || deps.inputImportJsonFile;

    const getLastImportedAssetFileName =
      (session && session.getLastImportedAssetFileName) || deps.getLastImportedAssetFileName;
    const setLastImportedAssetFileName =
      (session && session.setLastImportedAssetFileName) || deps.setLastImportedAssetFileName;
    const updateImportInfo =
      (session && session.updateImportInfo) || deps.updateImportInfo;

    function doExportAsset() {
      clearStatus();
      try {
        const asset = state.getAsset ? state.getAsset() : state.asset;
        // Export the palette as the UI currently presents it, including runtime brush color edits.
        const palette = buildExportPaletteFromUI(state.getPaletteConfig(), getBrushColors());

        ioAsset.exportAssetAsJson(asset, {
          mapName: state.mapName,
          filename: getLastImportedAssetFileName() ? getLastImportedAssetFileName() : null,
          palette
        });

        setStatus('Exported JSON.', 'ok');
      } catch (e) {
        setStatus('Export JSON failed.', 'error');
        setDebug(String(e && e.message ? e.message : e));
      }
    }

    function tryImportText(text, sourceFileName) {
      const srcName = (sourceFileName && typeof sourceFileName === 'string')
        ? sourceFileName
        : '';

      // Try asset import first because asset files may also carry palette data in newer formats.
      const r = ioAsset.parseAssetJsonText(text);
      if (r && r.ok) {
        const ok = state.loadAsset(r.value);
        if (!ok) {
          setStatus('Import asset failed (state).', 'error');
          return { ok: false, kind: 'asset' };
        }

        setLastImportedAssetFileName(srcName);

        if (r.detectedVersion === 2 && Array.isArray(r.importedPalette)) {
          // Reset first so removed entries do not leak old colors.
          resetBrushColorsToBaseForEntries(
            state.getPaletteConfig(),
            getBrushColors(),
            getBaseBrushColors()
          );
          applyImportedPaletteToBrushColors(r.importedPalette, getBrushColors());
        }

        const isV2 = (r.detectedVersion === 2 && Array.isArray(r.importedPalette));
        const pcNow = state.getPaletteConfig();
        reconcileBrushColors(pcNow, isV2);

        if (isV2) {
          // Version 2 asset imports can change palette presentation as well as tile data.
          rebuildPaletteTable(pcNow);
          renderAllCells();
        }

        updateImportInfo();
        setStatus('Imported JSON.', 'ok');
        return { ok: true, kind: 'asset' };
      }

      setStatus('Import failed: not a supported JSON asset.', 'error');
      let errs = [];
      if (r && r.errors) {
        errs = errs.concat(r.errors);
      }
      setDebug(errs.join('\n'));
      return { ok: false, kind: 'unknown' };
    }

    function openImportDialog() {
      if (!inputImportJsonFile) {
        return;
      }
      inputImportJsonFile.value = '';
      inputImportJsonFile.click();
    }

    function bind(buttons) {
      buttons = buttons || {};

      if (buttons.btnImportJson) {
        buttons.btnImportJson.addEventListener('click', () => {
          clearStatus();
          openImportDialog();
        });
      }

      if (inputImportJsonFile) {
        inputImportJsonFile.addEventListener('change', () => {
          const file = inputImportJsonFile.files && inputImportJsonFile.files[0];
          if (!file) {
            return;
          }

          ioAsset.readTextFromFile(file).then((text) => {
            // Clear first so selecting the same file again still triggers change.
            clearStatus();
            inputImportJsonFile.value = '';
            tryImportText(text, file.name);
          }).catch((e) => {
            clearStatus();
            inputImportJsonFile.value = '';
            setStatus('Import failed: could not read file.', 'error');
            setDebug(String(e && e.message ? e.message : e));
          });
        });
      }

      if (buttons.btnExportJson) {
        buttons.btnExportJson.textContent = 'Export JSON';
        buttons.btnExportJson.addEventListener('click', doExportAsset);
      }
    }

    return {
      doExportAsset,
      tryImportText,
      openImportDialog,
      bind
    };
  }

  window.appFeatureImportExport = {
    create
  };
  window.AppFeatureImportExport = window.appFeatureImportExport;
})();
