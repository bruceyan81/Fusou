// ui/dom.js
(function () {
  'use strict';

  function getById(id) {
    return document.getElementById(id);
  }

  function getRefs() {
    return {
      // Header / status
      formEl: getById('configBar'),
      statusEl: getById('status'),
      debugEl: getById('debug'),

      // Grid / rulers / tooltip
      gridRoot: getById('gridRoot'),
      gridWrap: getById('gridWrap'),
      coordTipEl: getById('cmCoordTip'),
      rulerTopInner: getById('rulerTopInner'),
      rulerLeftInner: getById('rulerLeftInner'),

      // Resize controls
      inputWidth: getById('inputWidth'),
      inputHeight: getById('inputHeight'),
      selectResizePolicy: getById('selectResizePolicy'),
      btnApplyResize: getById('btnApplyResize'),
      btnClear: getById('btnClear'),

      // Import / export
      btnImportJson: getById('btnImportJson'),
      inputImportJsonFile: getById('inputImportJsonFile'),
      importInfo: getById('importInfo'),
      btnExportJson: getById('btnExportJson'),

      // Tool / palette panel
      toolsPanel: getById('toolsPanel'),
      selectRenderBrush: getById('selectRenderBrush'),
      paletteTbody: getById('paletteTbody'),
      paletteTable: getById('paletteTable')
    };
  }

  window.appUiDom = {
    getRefs
  };
  window.AppUIDom = window.appUiDom;
})();
