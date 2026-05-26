// helpers/painting-session.js
(function () {
  'use strict';

  function create(options) {
    options = options || {};

    const selectRenderBrush = options.selectRenderBrush || null;

    let isPointerDown = false;
    let lastPaintIndex = -1;
    let paintRaf = 0;
    const pendingPaint = new Map(); // idx -> id
    let currentBrushId = 1;

    // Session state preserves the previous brush for non-numeric input.
    function normalizeBrushId(value, fallback) {
      const base = (fallback === null || fallback === undefined) ? currentBrushId : fallback;
      const n = Number(value);
      if (!Number.isFinite(n)) {
        return (base | 0);
      }

      const v = Math.trunc(n);
      if (v < 0) {
        v = 0;
      }
      return v | 0;
    }

    function syncBrushIdFromInput() {
      if (!selectRenderBrush) {
        return currentBrushId | 0;
      }
      currentBrushId = normalizeBrushId(selectRenderBrush.value, currentBrushId);
      return currentBrushId | 0;
    }

    function getBrushId() {
      if (selectRenderBrush) {
        return syncBrushIdFromInput();
      }
      return currentBrushId | 0;
    }

    function setBrushId(v) {
      currentBrushId = normalizeBrushId(v, currentBrushId);

      if (selectRenderBrush) {
        selectRenderBrush.value = String(currentBrushId | 0);
      }
    }

    function setIsPointerDown(v) {
      isPointerDown = !!v;
    }

    function getIsPointerDown() {
      return !!isPointerDown;
    }

    function setLastPaintIndex(v) {
      lastPaintIndex = v | 0;
    }

    function getLastPaintIndex() {
      return lastPaintIndex | 0;
    }

    function getPaintRaf() {
      return paintRaf | 0;
    }

    function setPaintRaf(v) {
      paintRaf = v | 0;
    }

    function getPendingPaint() {
      return pendingPaint;
    }

    return {
      syncBrushIdFromInput,
      getBrushId,
      setBrushId,

      setIsPointerDown,
      getIsPointerDown,
      setLastPaintIndex,
      getLastPaintIndex,
      getPaintRaf,
      setPaintRaf,
      getPendingPaint
    };
  }

  window.appHelpersPaintingSession = {
    create
  };
  window.AppHelpersPaintingSession = window.appHelpersPaintingSession;
})();
