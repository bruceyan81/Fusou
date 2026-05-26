// helpers/coordTip.js
(function () {
  'use strict';

  function create(options) {
    const coordTipEl = options.coordTipEl || null;
    const state = options.state;
    const windowRef = options.windowRef || window;
    const coordTipOffsetX =
      (options.coordTipOffsetX === null || options.coordTipOffsetX === undefined)
        ? 12
        : (options.coordTipOffsetX | 0);
    const coordTipOffsetY =
      (options.coordTipOffsetY === null || options.coordTipOffsetY === undefined)
        ? 14
        : (options.coordTipOffsetY | 0);

    let hoverTipRaf = 0;
    let hoverRuntimeProvider = null;

    function setHoverRuntimeProvider(fn) {
      hoverRuntimeProvider = (typeof fn === 'function') ? fn : null;
    }

    function hide() {
      if (!coordTipEl) {
        return;
      }
      coordTipEl.style.display = 'none';
      coordTipEl.setAttribute('aria-hidden', 'true');
    }

    function show() {
      if (!coordTipEl) {
        return;
      }
      if (coordTipEl.style.display !== 'block') {
        coordTipEl.style.display = 'block';
      }
      coordTipEl.setAttribute('aria-hidden', 'false');
    }

    function setText(x, y) {
      if (!coordTipEl) {
        return;
      }
      coordTipEl.textContent = '{ ' + (x | 0) + ', ' + (y | 0) + ' }';
    }

    function indexToXY(idx, width) {
      idx = idx | 0;
      width = width | 0;
      if (width <= 0) {
        return { x: 0, y: 0 };
      }
      return { x: (idx % width) | 0, y: ((idx / width) | 0) };
    }

    function clamp(n, lo, hi) {
      n = +n;
      if (n < lo) {
        return lo;
      }
      if (n > hi) {
        return hi;
      }
      return n;
    }

    function flush() {
      hoverTipRaf = 0;

      if (!coordTipEl || !hoverRuntimeProvider) {
        return;
      }

      const rt = hoverRuntimeProvider();
      if (!rt || rt.index === null || rt.index === undefined) {
        hide();
        return;
      }

      const w = state.getWidth
        ? state.getWidth()
        : (state.getAsset ? (state.getAsset().width | 0) : 0);
      const xy = indexToXY(rt.index | 0, w | 0);

      setText(xy.x, xy.y);
      show();

      const tipW = coordTipEl.offsetWidth | 0;
      const tipH = coordTipEl.offsetHeight | 0;

      let x = (rt.clientX | 0) + coordTipOffsetX;
      let y = (rt.clientY | 0) + coordTipOffsetY;

      const maxX = (windowRef.innerWidth | 0) - tipW - 2;
      const maxY = (windowRef.innerHeight | 0) - tipH - 2;

      x = clamp(x, 2, maxX);
      y = clamp(y, 2, maxY);

      coordTipEl.style.left = x + 'px';
      coordTipEl.style.top = y + 'px';
    }

    function requestUpdate() {
      if (hoverTipRaf) {
        return;
      }
      hoverTipRaf = windowRef.requestAnimationFrame(flush);
    }

    return {
      setHoverRuntimeProvider,
      hide,
      requestUpdate,
      flush
    };
  }

  window.appHelpersCoordTip = {
    create
  };
  window.AppHelpersCoordTip = window.appHelpersCoordTip;
})();
