(function () {
  'use strict';

  function wireApp(options) {
    options = options || {};

    const views = options.views || {};
    const features = options.features || {};
    const controls = options.controls || {};

    const rulerView = views.rulerView || null;

    const paintingFeature = features.paintingFeature || null;
    const resizeFeature = features.resizeFeature || null;
    const importExportFeature = features.importExportFeature || null;
    const paletteEditingFeature = features.paletteEditingFeature || null;

    const formEl = controls.formEl || null;
    const btnClear = controls.btnClear || null;
    const btnImportJson = controls.btnImportJson || null;
    const btnExportJson = controls.btnExportJson || null;
    const selectRenderBrush = controls.selectRenderBrush || null;

    // Bind views first so feature callbacks can assume the base DOM wiring already exists.
    if (rulerView && rulerView.bind) {
      rulerView.bind();
    }

    if (paintingFeature && paintingFeature.bind) {
      paintingFeature.bind();
    }

    if (resizeFeature && resizeFeature.bind) {
      resizeFeature.bind({
        formEl,
        btnClear
      });
    }

    if (importExportFeature && importExportFeature.bind) {
      importExportFeature.bind({
        btnImportJson,
        btnExportJson
      });
    }

    if (paletteEditingFeature && paletteEditingFeature.bindBrushSelect) {
      paletteEditingFeature.bindBrushSelect(selectRenderBrush);
    }

    return {};
  }

  window.appHelpersAppWiring = {
    wireApp
  };
  window.AppHelpersAppWiring = window.appHelpersAppWiring;
})();
