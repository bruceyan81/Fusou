// features/resize.js
(function () {
  'use strict';

  function toInt(v, fallback) {
    const n = Number(v);
    if (!Number.isFinite(n)) {
      return fallback | 0;
    }
    return Math.trunc(n) | 0;
  }

  /**
   * @param {Object} deps - Feature dependencies.
   * @param {Object} deps.runtime - State and asset size limit collaborators.
   * @param {Object} deps.status - Status output callbacks.
   * @param {Object} deps.controls - Resize form controls.
   * @returns {Object} Resize feature API.
   */
  function create(deps) {
    deps = deps || {};

    const runtime = deps.runtime || null;
    const status = deps.status || null;
    const controls = deps.controls || null;

    const state = (runtime && runtime.state) || deps.state || null;
    const getLimits = (runtime && runtime.getLimits) || deps.getLimits || null;

    const clearStatus = (status && status.clearStatus) || deps.clearStatus || (() => {});
    const setStatus = (status && status.setStatus) || deps.setStatus || (() => {});

    const inputWidth = (controls && controls.inputWidth) || deps.inputWidth || null;
    const inputHeight = (controls && controls.inputHeight) || deps.inputHeight || null;
    const selectResizePolicy =
      (controls && controls.selectResizePolicy) || deps.selectResizePolicy || null;

    function readLimits() {
      if (typeof getLimits === 'function') {
        return getLimits() || {};
      }
      return {};
    }

    function syncInputsFromState() {
      if (!state || typeof state.getAsset !== 'function') {
        return;
      }

      const asset = state.getAsset();
      if (inputWidth) {
        inputWidth.value = String((asset.width | 0) || 0);
      }
      if (inputHeight) {
        inputHeight.value = String((asset.height | 0) || 0);
      }
    }

    function readRequestedSize() {
      return {
        width: toInt(inputWidth && inputWidth.value, 0),
        height: toInt(inputHeight && inputHeight.value, 0),
        policy:
          (selectResizePolicy && selectResizePolicy.value === 'keepTopLeft')
            ? 'keepTopLeft'
            : 'clear'
      };
    }

    function validateSize(width, height, limits) {
      limits = limits || {};

      // UI validation gives immediate feedback before state.resize performs its own guard.
      const minW = (limits.MIN_W !== null && limits.MIN_W !== undefined)
        ? (limits.MIN_W | 0)
        : 1;
      const minH = (limits.MIN_H !== null && limits.MIN_H !== undefined)
        ? (limits.MIN_H | 0)
        : 1;
      const maxW = (limits.MAX_W !== null && limits.MAX_W !== undefined)
        ? (limits.MAX_W | 0)
        : 500;
      const maxH = (limits.MAX_H !== null && limits.MAX_H !== undefined)
        ? (limits.MAX_H | 0)
        : 500;
      const maxCells = (limits.MAX_CELLS !== null && limits.MAX_CELLS !== undefined)
        ? (limits.MAX_CELLS | 0)
        : (maxW * maxH);

      if (width < minW || width > maxW) {
        return {
          ok: false,
          message: 'Resize failed: width must be ' + minW + '-' + maxW + '.'
        };
      }

      if (height < minH || height > maxH) {
        return {
          ok: false,
          message: 'Resize failed: height must be ' + minH + '-' + maxH + '.'
        };
      }

      if ((width * height) > maxCells) {
        return {
          ok: false,
          message: 'Resize failed: cells limit exceeded.'
        };
      }

      return { ok: true };
    }

    function doResize() {
      if (!state || typeof state.resize !== 'function') {
        setStatus('Resize failed.', 'error');
        return false;
      }

      clearStatus();

      const req = readRequestedSize();
      const limits = readLimits();
      const vr = validateSize(req.width, req.height, limits);

      if (!vr.ok) {
        setStatus(vr.message, 'error');
        syncInputsFromState();
        return false;
      }

      const ok = state.resize(req.width, req.height, req.policy);
      if (!ok) {
        setStatus('Resize failed.', 'error');
        syncInputsFromState();
        return false;
      }

      syncInputsFromState();
      setStatus(
        'Resized to ' + (req.width | 0) + '×' + (req.height | 0) + '.',
        'ok'
      );
      return true;
    }

    function doClear() {
      if (!state || typeof state.clearAll !== 'function') {
        setStatus('Clear failed.', 'error');
        return false;
      }

      clearStatus();
      state.clearAll();
      syncInputsFromState();
      setStatus('Cleared map.', 'ok');
      return true;
    }

    function bind(options) {
      options = options || {};

      const formEl = options.formEl || null;
      const btnClear = options.btnClear || null;

      syncInputsFromState();

      if (formEl) {
        formEl.addEventListener('submit', (e) => {
          e.preventDefault();
          doResize();
        });
      }

      if (btnClear) {
        btnClear.addEventListener('click', () => {
          doClear();
        });
      }
    }

    return {
      bind,
      doResize,
      doClear,
      syncInputsFromState
    };
  }

  window.appFeatureResize = {
    create
  };
})();
