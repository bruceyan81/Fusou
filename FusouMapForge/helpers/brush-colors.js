// helpers/brushColors.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const console16 = options.console16 || null;

    function clampConsole16Index(v) {
      let n = Number(v);
      if (!Number.isFinite(n)) {
        return 0;
      }
      n = n | 0;
      if (n < 0) {
        n = 0;
      }
      if (n > 15) {
        n = 15;
      }
      return n;
    }

    function defaultBrushColor() {
      const fg = (console16 && typeof console16.DEFAULT_FG === 'number')
        ? console16.DEFAULT_FG
        : 0;
      const bg = (console16 && typeof console16.DEFAULT_BG === 'number')
        ? console16.DEFAULT_BG
        : 15;
      return { fg: clampConsole16Index(fg), bg: clampConsole16Index(bg) };
    }

    function cloneBrushColorMap(src) {
      const out = {};
      if (!src || typeof src !== 'object') {
        return out;
      }

      for (const k in src) {
        if (!Object.prototype.hasOwnProperty.call(src, k)) {
          continue;
        }

        let id = Number(k);
        if (!Number.isFinite(id)) {
          continue;
        }
        id = id | 0;
        if (id < 0) {
          continue;
        }

        const v = src[k] || {};
        out[id] = {
          fg: clampConsole16Index(v.fg),
          bg: clampConsole16Index(v.bg)
        };
      }
      return out;
    }

    function applyImportedPaletteToBrushColors(importedPalette, brushColors) {
      if (!Array.isArray(importedPalette) || !brushColors) {
        return;
      }

      for (let i = 0; i < importedPalette.length; i++) {
        const e = importedPalette[i];
        if (!e) {
          continue;
        }

        const tileId = e.tileId | 0;
        brushColors[tileId] = {
          fg: clampConsole16Index(e.fg),
          bg: clampConsole16Index(e.bg)
        };
      }
    }

    function resetBrushColorsToBaseForEntries(paletteConfig, brushColors, baseBrushColors) {
      const entries = (paletteConfig && paletteConfig.entries) ? paletteConfig.entries : [];

      for (let i = 0; i < entries.length; i++) {
        const id = entries[i].id;
        const b = baseBrushColors && baseBrushColors[id] ? baseBrushColors[id] : null;

        brushColors[id] = b
          ? { fg: clampConsole16Index(b.fg), bg: clampConsole16Index(b.bg) }
          : defaultBrushColor();
      }
    }

    function reconcileBrushColors(paletteConfig, brushColors, fillMissing) {
      if (fillMissing === undefined) {
        fillMissing = true;
      }

      const entries = (paletteConfig && paletteConfig.entries) ? paletteConfig.entries : [];
      const valid = Object.create(null);

      for (let i = 0; i < entries.length; i++) {
        valid[entries[i].id] = true;
      }

      Object.keys(brushColors).forEach((k) => {
        const id = parseInt(k, 10);
        if (!valid[id]) {
          delete brushColors[k];
        }
      });

      if (fillMissing) {
        for (let j = 0; j < entries.length; j++) {
          const id2 = entries[j].id;
          if (!brushColors[id2]) {
            brushColors[id2] = defaultBrushColor();
          } else {
            brushColors[id2].fg = clampConsole16Index(brushColors[id2].fg);
            brushColors[id2].bg = clampConsole16Index(brushColors[id2].bg);
          }
        }
      } else {
        for (let j2 = 0; j2 < entries.length; j2++) {
          const id3 = entries[j2].id;
          if (brushColors[id3]) {
            brushColors[id3].fg = clampConsole16Index(brushColors[id3].fg);
            brushColors[id3].bg = clampConsole16Index(brushColors[id3].bg);
          }
        }
      }
    }

    function getBrushColor(brushColors, id) {
      id = id | 0;
      const v = brushColors[id];
      if (!v) {
        return defaultBrushColor();
      }

      return {
        fg: clampConsole16Index(v.fg),
        bg: clampConsole16Index(v.bg)
      };
    }

    return {
      clampConsole16Index,
      defaultBrushColor,
      cloneBrushColorMap,
      applyImportedPaletteToBrushColors,
      resetBrushColorsToBaseForEntries,
      reconcileBrushColors,
      getBrushColor
    };
  }

  window.appHelpersBrushColors = {
    create
  };
  window.AppHelpersBrushColors = window.appHelpersBrushColors;
})();
