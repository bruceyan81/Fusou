// ui/gridView.js
(function () {
  'use strict';

  function create(refs) {
    refs = refs || {};

    const gridRoot = refs.gridRoot || null;
    const inputWidth = refs.inputWidth || null;
    const inputHeight = refs.inputHeight || null;

    let cellEls = [];
    let gridW = 0;
    let gridH = 0;
    let paletteMap = new Map();

    function rebuildPaletteMap(paletteConfig) {
      paletteMap = new Map();
      if (!paletteConfig || !Array.isArray(paletteConfig.entries)) {
        return;
      }

      for (let i = 0; i < paletteConfig.entries.length; i++) {
        const e = paletteConfig.entries[i] || {};
        const id = e.id | 0;
        const glyph = String((e.glyph === null || e.glyph === undefined) ? '' : e.glyph);
        const name = String((e.name === null || e.name === undefined) ? '' : e.name);
        paletteMap.set(id, { glyph, name });
      }
    }

    function glyphForId(tileId) {
      const id = tileId | 0;
      if (id === 0) {
        const z = paletteMap.get(0);
        return z ? String(z.glyph || '') : '';
      }
      const t = paletteMap.get(id);
      if (t) {
        return String(t.glyph || '');
      }
      return String(id);
    }

    function applyGridLayout(w) {
      if (!gridRoot) {
        return;
      }
      gridRoot.style.gridTemplateColumns = 'repeat(' + w + ', var(--cell-size, 18px))';
    }

    function buildGridDom(w, h) {
      if (!gridRoot) {
        return;
      }

      gridW = w | 0;
      gridH = h | 0;

      gridRoot.textContent = '';
      cellEls = new Array(gridW * gridH);

      applyGridLayout(gridW);

      const frag = document.createDocumentFragment();
      for (let i = 0; i < gridW * gridH; i++) {
        const el = document.createElement('div');
        el.className = 'cm-cell';
        el.dataset.index = String(i);
        el.textContent = '';
        cellEls[i] = el;
        frag.appendChild(el);
      }
      gridRoot.appendChild(frag);
    }

    function applyCellConsoleColors(cellEl, tileId, getBrushColor) {
      const c16 = window.appConstAsset && window.appConstAsset.CONSOLE16;
      const hex = (c16 && c16.HEX) ? c16.HEX : null;

      if (!hex || hex.length < 16) {
        cellEl.style.color = '';
        cellEl.style.backgroundColor = '';
        return;
      }

      const c = getBrushColor(tileId | 0);
      cellEl.style.color = hex[(c.fg | 0) & 15];
      cellEl.style.backgroundColor = hex[(c.bg | 0) & 15];
    }

    function renderAllCells(state, getBrushColor, renderRulers) {
      const asset = state.getAsset();
      const tiles = asset.tiles;

      if (inputWidth) {
        inputWidth.value = String(asset.width | 0);
      }
      if (inputHeight) {
        inputHeight.value = String(asset.height | 0);
      }

      const shouldRebuild =
        (asset.width | 0) !== gridW ||
        (asset.height | 0) !== gridH ||
        cellEls.length !== tiles.length;

      if (shouldRebuild) {
        buildGridDom(asset.width | 0, asset.height | 0);
        if (typeof renderRulers === 'function') {
          renderRulers(asset.width | 0, asset.height | 0);
        }
      }

      for (let i = 0; i < tiles.length; i++) {
        const id = tiles[i] | 0;
        const el = cellEls[i];
        if (!el) {
          continue;
        }

        el.classList.toggle('is-filled', id !== 0);
        el.textContent = (id === 0) ? '' : glyphForId(id);
        applyCellConsoleColors(el, id, getBrushColor);
      }
    }

    function renderIndices(indices, values, getBrushColor) {
      if (!Array.isArray(indices) || indices.length === 0) {
        return;
      }

      for (let k = 0; k < indices.length; k++) {
        const idx = indices[k] | 0;
        if (idx < 0 || idx >= cellEls.length) {
          continue;
        }

        const el = cellEls[idx];
        if (!el) {
          continue;
        }

        const id = (values && values.length === indices.length) ? (values[k] | 0) : 0;
        el.classList.toggle('is-filled', id !== 0);
        el.textContent = (id === 0) ? '' : glyphForId(id);
        applyCellConsoleColors(el, id, getBrushColor);
      }
    }

    return {
      rebuildPaletteMap,
      glyphForId,
      renderAllCells,
      renderIndices
    };
  }

  window.appUiGridView = {
    create
  };
  window.AppUIGridView = window.appUiGridView;
})();
