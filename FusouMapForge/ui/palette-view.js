// ui/palette-view.js
(function () {
  'use strict';

  /**
   * @param {Object} refs - DOM references used by palette UI.
   * @returns {Object} Palette view API.
   */
  function create(refs) {
    refs = refs || {};

    const selectRenderBrush = refs.selectRenderBrush || null;
    const paletteTbody = refs.paletteTbody || null;

    function rebuildBrushOptions(paletteConfig) {
      if (!selectRenderBrush) {
        return;
      }

      const previousValue = selectRenderBrush.value;
      selectRenderBrush.textContent = '';

      const entries = (paletteConfig && paletteConfig.entries) ? paletteConfig.entries.slice() : [];

      for (let i = 0; i < entries.length; i++) {
        const e = entries[i] || {};
        const id = e.id | 0;
        const name = String((e.name === null || e.name === undefined) ? '' : e.name);
        const glyph = String((e.glyph === null || e.glyph === undefined) ? '' : e.glyph);

        const opt = document.createElement('option');
        opt.value = String(id);
        opt.textContent = `[${id}] ${name || `Tile${id}`} : ${glyph || ''}`;
        selectRenderBrush.appendChild(opt);
      }

      if (selectRenderBrush.options.length > 0) {
        let has1 = false;
        let hasPrevious = false;
        for (let j = 0; j < selectRenderBrush.options.length; j++) {
          if ((selectRenderBrush.options[j].value | 0) === 1) {
            has1 = true;
          }
          if (selectRenderBrush.options[j].value === previousValue) {
            hasPrevious = true;
          }
        }
        if (hasPrevious) {
          selectRenderBrush.value = previousValue;
        } else {
          selectRenderBrush.value = has1 ? '1' : selectRenderBrush.options[0].value;
        }
      }
    }

    function makeConsole16Select(initialIndex) {
      const s = document.createElement('select');
      s.className = 'cm-colorSel';

      const c16 = window.appConstAsset && window.appConstAsset.CONSOLE16;
      const names = (c16 && c16.NAMES) ? c16.NAMES : [];

      for (let i = 0; i < 16; i++) {
        const opt = document.createElement('option');
        const nm = names[i] || `Color${i}`;
        opt.value = String(i);
        opt.textContent = `${i < 10 ? ` ${i}` : String(i)} ${nm}`;
        s.appendChild(opt);
      }

      s.value = String((initialIndex | 0) & 15);
      return s;
    }

    function applySampleStyle(el, fgIdx, bgIdx, text) {
      const c16 = window.appConstAsset && window.appConstAsset.CONSOLE16;
      const hex = (c16 && c16.HEX) ? c16.HEX : null;

      if (hex && hex.length >= 16) {
        el.style.color = hex[(fgIdx | 0) & 15];
        el.style.backgroundColor = hex[(bgIdx | 0) & 15];
      } else {
        el.style.color = '';
        el.style.backgroundColor = '';
      }

      el.textContent = (text === null || text === undefined || text === '') ? 'Aa' : String(text);
    }

    function createPaletteRow(entry, deps) {
      const { getBrushColor, onColorChange, onEntryChange } = deps;
      const e = entry || {};
      const id = e.id | 0;

      const tr = document.createElement('tr');

      const tdId = document.createElement('td');
      tdId.textContent = String(id);

      const tdName = document.createElement('td');
      const nameInput = document.createElement('input');
      nameInput.type = 'text';
      nameInput.value = String((e.name === null || e.name === undefined) ? '' : e.name);
      nameInput.style.width = '10em';
      tdName.appendChild(nameInput);

      const tdGlyph = document.createElement('td');
      const glyphInput = document.createElement('input');
      glyphInput.type = 'text';
      glyphInput.value = String((e.glyph === null || e.glyph === undefined) ? '' : e.glyph);
      glyphInput.removeAttribute('maxlength');
      glyphInput.className = 'glyphInput';
      tdGlyph.appendChild(glyphInput);

      const curColor = (typeof getBrushColor === 'function')
        ? getBrushColor(id)
        : { fg: 0, bg: 15 };

      const tdFg = document.createElement('td');
      const fgSel = makeConsole16Select(curColor.fg);
      tdFg.appendChild(fgSel);

      const tdBg = document.createElement('td');
      const bgSel = makeConsole16Select(curColor.bg);
      tdBg.appendChild(bgSel);

      const tdPrev = document.createElement('td');
      const sample = document.createElement('span');
      sample.className = 'cm-colorSample';
      tdPrev.appendChild(sample);

      const refreshPreview = () => {
        applySampleStyle(sample, (fgSel.value | 0), (bgSel.value | 0), glyphInput.value);
      };
      refreshPreview();

      fgSel.addEventListener('change', () => {
        if (typeof onColorChange === 'function') {
          onColorChange(id, (fgSel.value | 0), (bgSel.value | 0), glyphInput.value);
        }
        refreshPreview();
      });

      bgSel.addEventListener('change', () => {
        if (typeof onColorChange === 'function') {
          onColorChange(id, (fgSel.value | 0), (bgSel.value | 0), glyphInput.value);
        }
        refreshPreview();
      });

      glyphInput.addEventListener('input', refreshPreview);

      const applyChange = () => {
        if (typeof onEntryChange === 'function') {
          onEntryChange(id, glyphInput.value, nameInput.value);
        }
      };

      nameInput.addEventListener('change', applyChange);
      glyphInput.addEventListener('change', applyChange);

      tr.appendChild(tdId);
      tr.appendChild(tdName);
      tr.appendChild(tdGlyph);
      tr.appendChild(tdFg);
      tr.appendChild(tdBg);
      tr.appendChild(tdPrev);

      return tr;
    }

    function rebuildPaletteTable(paletteConfig, deps) {
      if (!paletteTbody) {
        return;
      }

      deps = deps || {};

      const reconcileBrushColors = deps.reconcileBrushColors;
      const getBrushColor = deps.getBrushColor;
      const onColorChange = deps.onColorChange;
      const onEntryChange = deps.onEntryChange;

      if (typeof reconcileBrushColors === 'function') {
        reconcileBrushColors(paletteConfig, false);
      }

      paletteTbody.textContent = '';

      const entries = (paletteConfig && paletteConfig.entries) ? paletteConfig.entries.slice() : [];

      for (let i = 0; i < entries.length; i++) {
        paletteTbody.appendChild(createPaletteRow(entries[i], {
          getBrushColor,
          onColorChange,
          onEntryChange
        }));
      }
    }

    return {
      rebuildBrushOptions,
      rebuildPaletteTable
    };
  }

  window.appUiPaletteView = {
    create
  };
})();
