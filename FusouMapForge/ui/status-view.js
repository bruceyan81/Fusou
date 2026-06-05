// ui/statusView.js
(function () {
  'use strict';

  function create(refs) {
    refs = refs || {};

    const statusEl = refs.statusEl || null;
    const debugEl = refs.debugEl || null;

    function setStatus(msg, type) {
      if (!statusEl) {
        return;
      }
      statusEl.textContent = String(msg || '');
      statusEl.style.borderColor = (type === 'error') ? '#c33' : '#ddd';
    }

    function clearStatus() {
      setStatus('', '');
      if (debugEl) {
        debugEl.textContent = '';
      }
    }

    function setDebug(text) {
      if (!debugEl) {
        return;
      }
      debugEl.textContent = String(text || '');
    }

    function hide(el) {
      if (el) {
        el.style.display = 'none';
      }
    }

    function show(el, display) {
      if (el) {
        el.style.display = display || '';
      }
    }

    return {
      setStatus,
      clearStatus,
      setDebug,
      hide,
      show
    };
  }

  window.appUiStatusView = {
    create
  };
})();
