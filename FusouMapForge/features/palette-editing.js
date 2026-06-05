// features/palette-editing.js
(function () {
  'use strict';

  /**
   * @param {Object} deps - Feature dependencies.
   * @param {Object} deps.runtime - State, brush color, and active brush collaborators.
   * @param {Object} deps.status - Status output callbacks.
   * @param {Object} deps.ui - Palette and grid refresh callbacks.
   * @returns {Object} Palette editing feature API.
   */
  function create(deps) {
    deps = deps || {};

    const runtime = deps.runtime || null;
    const status = deps.status || null;
    const ui = deps.ui || null;

    const state = (runtime && runtime.state) || deps.state || null;

    const clearStatus = (status && status.clearStatus) || deps.clearStatus || (() => {});
    const setStatus = (status && status.setStatus) || deps.setStatus || (() => {});

    const reconcileBrushColors =
      (runtime && runtime.reconcileBrushColors) || deps.reconcileBrushColors || (() => {});

    const getBrushColors =
      (runtime && runtime.getBrushColors) || deps.getBrushColors || (() => {
        return [];
      });

    const getBrushId =
      (runtime && runtime.getBrushId) || deps.getBrushId || (() => {
        return 0;
      });

    const setPaintId =
      (runtime && runtime.setPaintId) || deps.setPaintId || (() => {});

    const rebuildPaletteMap =
      (ui && ui.rebuildPaletteMap) || deps.rebuildPaletteMap || (() => {});

    const rebuildBrushOptions =
      (ui && ui.rebuildBrushOptions) || deps.rebuildBrushOptions || (() => {});

    const rebuildPaletteTable =
      (ui && ui.rebuildPaletteTable) || deps.rebuildPaletteTable || (() => {});

    const renderAllCells =
      (ui && ui.renderAllCells) || deps.renderAllCells || (() => {});

    // Palette editing follows painting semantics for active brush ids.
    function normalizeBrushId(value) {
      const n = Number(value);
      if (!Number.isFinite(n)) {
        return 0;
      }
      let v = Math.trunc(n);
      if (v < 0) {
        v = 0;
      }
      return v | 0;
    }

    function getPaletteConfigSafe() {
      if (!state || typeof state.getPaletteConfig !== 'function') {
        return null;
      }
      const pc = state.getPaletteConfig();
      if (!pc || !Array.isArray(pc.entries)) {
        return null;
      }
      return pc;
    }

    function refreshPaletteUi() {
      const paletteConfig = getPaletteConfigSafe();
      if (!paletteConfig) {
        return;
      }

      rebuildPaletteMap(paletteConfig);
      rebuildPaletteTable(paletteConfig);
      renderAllCells();
    }

    function updateEntryAtId(id, glyph, name) {
      if (!state || typeof state.replacePaletteConfig !== 'function') {
        return false;
      }

      const paletteConfig = getPaletteConfigSafe();
      if (!paletteConfig) {
        return false;
      }

      const paletteId = id | 0;
      const entries = paletteConfig.entries.slice();
      let found = false;

      for (let i = 0; i < entries.length; i++) {
        const entry = entries[i] || {};
        if ((entry.id | 0) !== paletteId) {
          continue;
        }

        entries[i] = {
          id: paletteId,
          glyph: String(glyph !== null && glyph !== undefined ? glyph : ''),
          name: String(name !== null && name !== undefined ? name : '')
        };
        found = true;
        break;
      }

      if (!found) {
        return false;
      }

      return !!state.replacePaletteConfig({
        version: (paletteConfig.version | 0) || 1,
        entries
      });
    }

    function applyBrushColorAtId(id, fg, bg) {
      const brushColors = getBrushColors();
      if (!brushColors || typeof brushColors !== 'object') {
        return false;
      }

      const paletteId = id | 0;
      if (paletteId < 0) {
        return false;
      }

      brushColors[paletteId] = {
        fg: (fg | 0) & 15,
        bg: (bg | 0) & 15
      };
      return true;
    }

    function onColorChange(id, fg, bg, glyphValue) {
      clearStatus();

      const paletteId = id | 0;
      if (paletteId < 0) {
        setStatus('Palette color update failed.', 'error');
        return false;
      }

      const okColor = applyBrushColorAtId(paletteId, fg, bg);
      if (!okColor) {
        setStatus('Palette color update failed.', 'error');
        return false;
      }

      const paletteConfig = getPaletteConfigSafe();
      if (paletteConfig) {
        reconcileBrushColors(paletteConfig, true);
      }

      refreshPaletteUi();
      setStatus('Updated palette color.', 'ok');
      return true;
    }

    function onEntryChange(id, glyphValue, nameValue) {
      clearStatus();

      const paletteId = id | 0;
      if (paletteId < 0) {
        setStatus('Palette entry update failed.', 'error');
        return false;
      }

      const ok = updateEntryAtId(paletteId, glyphValue, nameValue);
      if (!ok) {
        setStatus('Palette entry update failed.', 'error');
        return false;
      }

      const paletteConfig = getPaletteConfigSafe();
      if (paletteConfig) {
        reconcileBrushColors(paletteConfig, true);
      }

      setStatus('Updated palette entry.', 'ok');
      return true;
    }

    function bindBrushSelect(selectEl) {
      if (!selectEl) {
        return;
      }

      selectEl.addEventListener('change', () => {
        const brushId = normalizeBrushId(selectEl.value);
        setPaintId(brushId);
      });

      const initialBrushId = normalizeBrushId(
        (typeof getBrushId === 'function') ? getBrushId() : selectEl.value
      );
      setPaintId(initialBrushId);
    }

    return {
      bindBrushSelect,
      onColorChange,
      onEntryChange
    };
  }

  window.appFeaturePaletteEditing = {
    create
  };
})();
