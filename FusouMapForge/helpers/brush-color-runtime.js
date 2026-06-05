// helpers/brushColorRuntime.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const brushColorHelpers = options.brushColorHelpers;
    if (!brushColorHelpers) {
      throw new Error('brushColorHelpers is required');
    }

    const builtIn = options.defaultBrushColors || null;

    function cloneMap(src) {
      return brushColorHelpers.cloneBrushColorMap(src);
    }

    const brushColors = cloneMap(builtIn);
    const baseBrushColors = cloneMap(brushColors);

    function getBrushColors() {
      return brushColors;
    }

    function getBaseBrushColors() {
      return baseBrushColors;
    }

    function cloneBrushColorMap(src) {
      return cloneMap(src);
    }

    function getBrushColor(id) {
      return brushColorHelpers.getBrushColor(brushColors, id);
    }

    return {
      getBrushColors,
      getBaseBrushColors,
      cloneBrushColorMap,
      getBrushColor
    };
  }

  window.appHelpersBrushColorRuntime = {
    create
  };
})();
