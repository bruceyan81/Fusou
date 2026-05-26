// helpers/appBoot.js
(function () {
  'use strict';

  function requireGlobal(obj, key, message) {
    if (!obj || !obj[key]) {
      throw new Error(message || ('Missing global: ' + key));
    }
    return obj[key];
  }

  function requireFn(obj, key, message) {
    const fn = requireGlobal(obj, key, message);
    if (typeof fn !== 'function') {
      throw new Error(message || ('Global is not a function: ' + key));
    }
    return fn;
  }

  function assertBootGlobals(win) {
    const missingViews = 'Required view globals missing: status/ruler/grid/palette view';
    const missingHelpers =
      'Required helper globals missing: importSession/brushColors/brushColorRuntime/' +
      'palette/paletteBridge/fileNaming helpers';
    const missingCore =
      'Required core globals missing: AssetState / InternalAssetSchema / PaletteConfigSchema';
    const missingFeatures =
      'Required feature globals missing: importExport/resize/painting/paletteEditing features';

    // UI primitives must exist before any helper or feature composes them.
    requireGlobal(win, 'appUiDom', 'Required global missing: appUiDom');
    requireFn(win.appUiDom, 'getRefs', 'Required global missing: appUiDom.getRefs');

    requireGlobal(win, 'appUiStatusView', missingViews);
    requireGlobal(win, 'appUiRulerView', missingViews);
    requireGlobal(win, 'appUiGridView', missingViews);
    requireGlobal(win, 'appUiPaletteView', missingViews);

    requireGlobal(
      win,
      'appHelpersImportSession',
      missingHelpers
    );
    requireGlobal(
      win,
      'appHelpersBrushColors',
      missingHelpers
    );
    requireGlobal(
      win,
      'appHelpersBrushColorRuntime',
      missingHelpers
    );
    requireGlobal(
      win,
      'appHelpersPalette',
      missingHelpers
    );
    requireGlobal(
      win,
      'appHelpersPaletteBridge',
      missingHelpers
    );
    requireGlobal(
      win,
      'appHelpersFileNaming',
      missingHelpers
    );

    // Core state, schema, and IO modules are required before runtime features attach.
    requireGlobal(win, 'assetState', missingCore);
    requireGlobal(win, 'internalAssetSchema', missingCore);
    requireGlobal(win, 'paletteConfigSchema', missingCore);

    requireGlobal(win, 'ioAsset', 'Required IO globals missing: ioAsset / ioPalette');
    requireGlobal(win, 'ioPalette', 'Required IO globals missing: ioAsset / ioPalette');

    requireGlobal(
      win,
      'appFeatureImportExport',
      missingFeatures
    );
    requireGlobal(
      win,
      'appFeatureResize',
      missingFeatures
    );
    requireGlobal(
      win,
      'appFeaturePainting',
      missingFeatures
    );
    requireGlobal(
      win,
      'appFeaturePaletteEditing',
      missingFeatures
    );
  }

  /**
   * @param {Object} appConstAsset - Constants namespace that may contain LIMITS.
   * @returns {Object} Normalized limit object used by form controls and features.
   */
  function getLimits(appConstAsset) {
    const limitsSource = (appConstAsset && appConstAsset.LIMITS) || null;

    const MIN_W = (
      limitsSource &&
      limitsSource.MIN_W !== null &&
      limitsSource.MIN_W !== undefined
    )
      ? (limitsSource.MIN_W | 0)
      : 1;
    const MIN_H = (
      limitsSource &&
      limitsSource.MIN_H !== null &&
      limitsSource.MIN_H !== undefined
    )
      ? (limitsSource.MIN_H | 0)
      : 1;
    const MAX_W = (
      limitsSource &&
      limitsSource.MAX_W !== null &&
      limitsSource.MAX_W !== undefined
    )
      ? (limitsSource.MAX_W | 0)
      : 500;
    const MAX_H = (
      limitsSource &&
      limitsSource.MAX_H !== null &&
      limitsSource.MAX_H !== undefined
    )
      ? (limitsSource.MAX_H | 0)
      : 500;
    const MAX_CELLS = (
      limitsSource &&
      limitsSource.MAX_CELLS !== null &&
      limitsSource.MAX_CELLS !== undefined
    )
      ? (limitsSource.MAX_CELLS | 0)
      : (MAX_W * MAX_H);

    return {
      MIN_W,
      MIN_H,
      MAX_W,
      MAX_H,
      MAX_CELLS
    };
  }

  function applyNumberInputLimits(inputWidth, inputHeight, limits) {
    if (inputWidth) {
      inputWidth.type = 'number';
      inputWidth.min = '' + limits.MIN_W;
      inputWidth.max = '' + limits.MAX_W;
    }

    if (inputHeight) {
      inputHeight.type = 'number';
      inputHeight.min = '' + limits.MIN_H;
      inputHeight.max = '' + limits.MAX_H;
    }
  }

  function assertCoreRefs(refs) {
    if (!refs || !refs.gridRoot) {
      throw new Error('gridRoot missing');
    }
    if (!refs || !refs.gridWrap) {
      throw new Error('gridWrap missing');
    }
  }

  window.appHelpersAppBoot = {
    assertBootGlobals,
    getLimits,
    applyNumberInputLimits,
    assertCoreRefs
  };
  window.AppHelpersAppBoot = window.appHelpersAppBoot;
})();
