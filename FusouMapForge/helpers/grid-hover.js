// helpers/gridHover.js
(function () {
  'use strict';

  function create(options) {
    const gridRoot = options.gridRoot || null;
    const documentRef = options.documentRef || document;
    const coordTip = options.coordTip || null;

    let hoverTipClientX = 0;
    let hoverTipClientY = 0;
    let hoverTipIndex = null;

    function hitIndexFromEventTarget(target) {
      if (!target) {
        return null;
      }
      const cell = (target.closest) ? target.closest('.cm-cell') : null;
      if (!cell || !cell.dataset) {
        return null;
      }
      const raw = cell.dataset.index;
      if (raw === null || raw === undefined) {
        return null;
      }
      const n = parseInt(raw, 10);
      return Number.isFinite(n) ? (n | 0) : null;
    }

    function hitIndexFromTarget(target) {
      let node = target;
      while (node && node !== gridRoot) {
        if (node.dataset && node.dataset.index !== null && node.dataset.index !== undefined) {
          const n = parseInt(node.dataset.index, 10);
          return Number.isFinite(n) ? n : null;
        }
        node = node.parentNode;
      }
      return null;
    }

    function hitIndexFromPoint(clientX, clientY) {
      const el = documentRef.elementFromPoint(clientX, clientY);
      if (!el) {
        return null;
      }
      return hitIndexFromTarget(el);
    }

    function setHoverRuntime(rt) {
      hoverTipClientX = rt.clientX | 0;
      hoverTipClientY = rt.clientY | 0;
      hoverTipIndex = (rt.index === null || rt.index === undefined) ? null : (rt.index | 0);
    }

    function getHoverRuntime() {
      return {
        clientX: hoverTipClientX | 0,
        clientY: hoverTipClientY | 0,
        index: hoverTipIndex
      };
    }

    function hideCoordTip() {
      if (coordTip && coordTip.hide) {
        coordTip.hide();
      }
    }

    function requestHoverTipUpdate() {
      if (coordTip && coordTip.requestUpdate) {
        coordTip.requestUpdate();
      }
    }

    return {
      hideCoordTip,
      hitIndexFromEventTarget,
      hitIndexFromTarget,
      hitIndexFromPoint,
      setHoverRuntime,
      getHoverRuntime,
      requestHoverTipUpdate
    };
  }

  window.appHelpersGridHover = {
    create
  };
})();
