// features/painting.js
(function () {
  'use strict';

  /**
   * @param {Object} deps - Feature dependencies.
   * @param {Object} deps.runtime - State, brush, hover, and paint-session collaborators.
   * @param {Object} deps.controls - DOM controls used by pointer painting.
   * @returns {Object} Painting feature API.
   */
  function create(deps) {
    deps = deps || {};

    const runtime = deps.runtime || null;
    const controls = deps.controls || null;

    // Resolve DOM and state first; remaining collaborators describe pointer state.
    const gridRoot = (controls && controls.gridRoot) || deps.gridRoot || null;
    const state = (runtime && runtime.state) || deps.state || null;

    const getBrushId =
      (runtime && runtime.getBrushId) || deps.getBrushId || (() => { return 0; });
    const hideCoordTip = (runtime && runtime.hideCoordTip) || deps.hideCoordTip || (() => {});
    const requestHoverTipUpdate =
      (runtime && runtime.requestHoverTipUpdate) || deps.requestHoverTipUpdate || (() => {});

    const setHoverRuntime =
      (runtime && runtime.setHoverRuntime) || deps.setHoverRuntime || (() => {});
    const getHoverRuntime =
      (runtime && runtime.getHoverRuntime) || deps.getHoverRuntime || (() => {
        return { clientX: 0, clientY: 0, index: null };
      });

    const hitIndexFromEventTarget =
      (runtime && runtime.hitIndexFromEventTarget) || deps.hitIndexFromEventTarget || (() => {
        return null;
      });

    let hitIndexFromPoint =
      (runtime && runtime.hitIndexFromPoint) || deps.hitIndexFromPoint || (() => {
        return null;
      });

    const setIsPointerDown =
      (runtime && runtime.setIsPointerDown) || deps.setIsPointerDown || (() => {});

    const getIsPointerDown =
      (runtime && runtime.getIsPointerDown) || deps.getIsPointerDown || (() => {
        return false;
      });

    const setLastPaintIndex =
      (runtime && runtime.setLastPaintIndex) || deps.setLastPaintIndex || (() => {});

    const getLastPaintIndex =
      (runtime && runtime.getLastPaintIndex) || deps.getLastPaintIndex || (() => {
        return -1;
      });

    const getPaintRaf =
      (runtime && runtime.getPaintRaf) || deps.getPaintRaf || (() => {
        return 0;
      });

    const setPaintRaf =
      (runtime && runtime.setPaintRaf) || deps.setPaintRaf || (() => {});

    const getPendingPaint =
      (runtime && runtime.getPendingPaint) || deps.getPendingPaint || (() => {
        return new Map();
      });

    let activePaintId = null;

    // Pointer painting treats invalid or negative brush ids as tile id 0.
    function normalizeBrushId(value) {
      const n = Number(value);
      if (!Number.isFinite(n)) {
        return 0;
      }
      let v = Math.trunc(n);
      if (v < 0) {
        v = 0;
      }
      return v | 0;
    }

    function updateHoverRuntime(clientX, clientY, index) {
      setHoverRuntime({
        clientX: clientX | 0,
        clientY: clientY | 0,
        index: (index === null || index === undefined) ? null : (index | 0)
      });
      requestHoverTipUpdate();
    }

    function queuePaint(index, brushId) {
      if (!state || typeof state.setTilesAtIndices !== 'function') {
        return false;
      }
      if (index === null || index === undefined || index < 0) {
        return false;
      }

      const pending = getPendingPaint();
      if (!pending || typeof pending.set !== 'function') {
        // Fallback to immediate writes if the shared batch map is unavailable.
        state.setTileAtIndex(index | 0, brushId | 0);
        return true;
      }

      // Buffer writes until the next frame so drag painting can batch contiguous updates.
      pending.set(index | 0, brushId | 0);

      if (getPaintRaf()) {
        return true;
      }

      const rafId = window.requestAnimationFrame(() => {
        setPaintRaf(0);

        const indices = [];
        const values = [];

        pending.forEach((value, key) => {
          indices.push(key | 0);
          values.push(value | 0);
        });

        pending.clear();

        if (indices.length > 0) {
          state.setTilesAtIndices(indices, values);
        }
      });

      setPaintRaf(rafId | 0);
      return true;
    }

    function paintIndex(index) {
      if (index === null || index === undefined || index < 0) {
        return false;
      }
      if (!state) {
        return false;
      }

      const last = getLastPaintIndex();
      // Skip duplicate writes while the pointer stays on the same cell.
      if ((last | 0) === (index | 0)) {
        return false;
      }

      const brushId = (activePaintId === null || activePaintId === undefined)
        ? normalizeBrushId(getBrushId())
        : normalizeBrushId(activePaintId);
      queuePaint(index | 0, brushId | 0);
      setLastPaintIndex(index | 0);
      return true;
    }

    function onPointerDown(e) {
      if (e.button !== 0 && e.button !== 2) {
        return;
      }

      const index = hitIndexFromEventTarget(e.target);
      if (index === null || index === undefined) {
        return;
      }

      setIsPointerDown(true);
      setLastPaintIndex(-1);
      activePaintId = (e.button === 2) ? 0 : normalizeBrushId(getBrushId());

      if (
        gridRoot &&
        gridRoot.setPointerCapture &&
        e.pointerId !== null &&
        e.pointerId !== undefined
      ) {
        try {
          gridRoot.setPointerCapture(e.pointerId);
        } catch (_) {}
      }

      updateHoverRuntime(e.clientX, e.clientY, index);
      paintIndex(index);
      e.preventDefault();
    }

    function onPointerMove(e) {
      let index = hitIndexFromEventTarget(e.target);

      // Pointer capture may move events off the original cell element during a drag.
      if ((index === null || index === undefined) && typeof hitIndexFromPoint === 'function') {
        index = hitIndexFromPoint(e.clientX, e.clientY);
      }

      updateHoverRuntime(e.clientX, e.clientY, index);

      if (!getIsPointerDown()) {
        return;
      }
      paintIndex(index);
    }

    function endPointer(e) {
      if (!getIsPointerDown()) {
        if (
          e &&
          e.clientX !== null &&
          e.clientX !== undefined &&
          e.clientY !== null &&
          e.clientY !== undefined
        ) {
          const hoverIndex = hitIndexFromPoint(e.clientX, e.clientY);
          updateHoverRuntime(e.clientX, e.clientY, hoverIndex);
        }
        return;
      }

      setIsPointerDown(false);
      setLastPaintIndex(-1);
      activePaintId = null;

      if (
        gridRoot &&
        gridRoot.releasePointerCapture &&
        e &&
        e.pointerId !== null &&
        e.pointerId !== undefined
      ) {
        try {
          gridRoot.releasePointerCapture(e.pointerId);
        } catch (_) {}
      }

      if (
        e &&
        e.clientX !== null &&
        e.clientX !== undefined &&
        e.clientY !== null &&
        e.clientY !== undefined
      ) {
        const hoverIndex = hitIndexFromPoint(e.clientX, e.clientY);
        updateHoverRuntime(e.clientX, e.clientY, hoverIndex);
      } else {
        hideCoordTip();
      }
    }

    function onPointerLeave() {
      if (getIsPointerDown()) {
        return;
      }
      hideCoordTip();
    }

    function bind() {
      if (!gridRoot) {
        return;
      }

      // Also listen on window so release outside the grid ends the paint session.
      gridRoot.addEventListener('pointerdown', onPointerDown);
      gridRoot.addEventListener('pointermove', onPointerMove);
      gridRoot.addEventListener('pointerleave', onPointerLeave);
      gridRoot.addEventListener('pointerup', endPointer);
      gridRoot.addEventListener('pointercancel', endPointer);
      gridRoot.addEventListener('contextmenu', (e) => {
        e.preventDefault();
      });

      window.addEventListener('pointerup', endPointer);
      window.addEventListener('pointercancel', endPointer);
    }

    return {
      bind,
      paintIndex
    };
  }

  window.appFeaturePainting = {
    create
  };
  window.AppFeaturePainting = window.appFeaturePainting;
})();
