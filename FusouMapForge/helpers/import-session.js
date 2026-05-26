// helpers/importSession.js
(function () {
  'use strict';

  function normalizeFileName(v) {
    return String(v || '');
  }

  function create() {
    let lastImportedAssetFileName = '';
    let lastImportedPaletteFileName = '';

    function getAssetFileName() {
      return lastImportedAssetFileName;
    }

    function setAssetFileName(v) {
      lastImportedAssetFileName = normalizeFileName(v);
    }

    function getPaletteFileName() {
      return lastImportedPaletteFileName;
    }

    function setPaletteFileName(v) {
      lastImportedPaletteFileName = normalizeFileName(v);
    }

    function getNames() {
      return {
        asset: lastImportedAssetFileName,
        palette: lastImportedPaletteFileName
      };
    }

    function reset() {
      lastImportedAssetFileName = '';
      lastImportedPaletteFileName = '';
    }

    return {
      getAssetFileName,
      setAssetFileName,
      getPaletteFileName,
      setPaletteFileName,
      getNames,
      reset
    };
  }

  window.appHelpersImportSession = {
    create
  };
  window.AppHelpersImportSession = window.appHelpersImportSession;
})();
