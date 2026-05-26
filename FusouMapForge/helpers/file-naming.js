// helpers/fileNaming.js
(function () {
  'use strict';

  function getExportFileName(kind, names, fallbackNameFn) {
    names = names || {};

    let name = '';
    if (kind === 'asset') {
      name = names.asset || '';
    } else if (kind === 'palette') {
      name = names.palette || '';
    }

    if (name && typeof name === 'string') {
      return name;
    }
    if (typeof fallbackNameFn === 'function') {
      return String(fallbackNameFn() || '');
    }
    return '';
  }

  window.appHelpersFileNaming = {
    getExportFileName
  };
  window.AppHelpersFileNaming = window.appHelpersFileNaming;
})();
