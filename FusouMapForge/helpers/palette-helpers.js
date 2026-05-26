// helpers/paletteHelpers.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const defaultImportFg = (typeof options.defaultImportFg === 'number')
      ? (options.defaultImportFg | 0)
      : 0;
    const defaultImportBg = (typeof options.defaultImportBg === 'number')
      ? (options.defaultImportBg | 0)
      : 15;

    function buildExportPaletteFromUI(paletteConfig, brushColors) {
      const entries = (paletteConfig && paletteConfig.entries) ? paletteConfig.entries : [];
      const out = [];

      for (let i = 0; i < entries.length; i++) {
        const tileId = entries[i].id | 0;
        const c = brushColors && brushColors[tileId];

        const fg = (c && c.fg !== null && c.fg !== undefined) ? (c.fg | 0) : defaultImportFg;
        const bg = (c && c.bg !== null && c.bg !== undefined) ? (c.bg | 0) : defaultImportBg;

        out.push({
          tileId,
          fg,
          bg
        });
      }

      return out;
    }

    return {
      buildExportPaletteFromUI
    };
  }

  window.appHelpersPalette = {
    create
  };
  window.AppHelpersPalette = window.appHelpersPalette;
})();
