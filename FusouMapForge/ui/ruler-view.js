// ui/rulerView.js
(function () {
  'use strict';

  const RULER_STEP = 5;

  function create(refs) {
    refs = refs || {};

    const gridWrap = refs.gridWrap || null;
    const rulerTopInner = refs.rulerTopInner || null;
    const rulerLeftInner = refs.rulerLeftInner || null;

    let rulerSyncRaf = 0;

    function getCssVarNumber(varName, fallback) {
      try {
        const raw = getComputedStyle(document.documentElement).getPropertyValue(varName);
        const n = parseFloat(String(raw || '').trim());
        return Number.isFinite(n) ? n : fallback;
      } catch (_) {
        return fallback;
      }
    }

    function getCellSizePx() {
      return getCssVarNumber('--cell-size', 18);
    }

    function clearEl(el) {
      if (!el) {
        return;
      }
      el.textContent = '';
    }

    function makeTickLine(axis) {
      const line = document.createElement('div');
      line.style.position = 'absolute';
      line.style.pointerEvents = 'none';
      line.style.opacity = '0.9';

      if (axis === 'x') {
        line.style.top = '0';
        line.style.bottom = '0';
        line.style.width = '0';
        line.style.borderLeft = '1px solid #bbb';
      } else {
        line.style.left = '0';
        line.style.right = '0';
        line.style.height = '0';
        line.style.borderTop = '1px solid #bbb';
      }
      return line;
    }

    function makeTickLabel(text) {
      const lab = document.createElement('div');
      lab.textContent = String(text);
      lab.style.position = 'absolute';
      lab.style.pointerEvents = 'none';
      lab.style.whiteSpace = 'nowrap';
      lab.style.fontSize = '11px';
      lab.style.lineHeight = '1';
      lab.style.color = '#444';
      return lab;
    }

    function syncNow() {
      if (!gridWrap) {
        return;
      }
      if (rulerTopInner) {
        rulerTopInner.style.transform = 'translateX(' + (-gridWrap.scrollLeft) + 'px)';
      }
      if (rulerLeftInner) {
        rulerLeftInner.style.transform = 'translateY(' + (-gridWrap.scrollTop) + 'px)';
      }
    }

    function requestSync() {
      if (rulerSyncRaf) {
        return;
      }
      rulerSyncRaf = requestAnimationFrame(() => {
        rulerSyncRaf = 0;
        syncNow();
      });
    }

    function render(w, h) {
      if (!rulerTopInner || !rulerLeftInner) {
        return;
      }

      w = w | 0;
      h = h | 0;
      if (w <= 0 || h <= 0) {
        return;
      }

      const cell = getCellSizePx();

      rulerTopInner.style.width = (w * cell) + 'px';
      rulerTopInner.style.height = '100%';

      rulerLeftInner.style.height = (h * cell) + 'px';
      rulerLeftInner.style.width = '100%';

      clearEl(rulerTopInner);
      clearEl(rulerLeftInner);

      for (let x = 0; x < w; x += RULER_STEP) {
        const leftPx = x * cell;

        const line = makeTickLine('x');
        line.style.left = leftPx + 'px';

        const lab = makeTickLabel(x);
        lab.style.left = (leftPx + 2) + 'px';
        lab.style.bottom = '2px';

        rulerTopInner.appendChild(line);
        rulerTopInner.appendChild(lab);
      }

      for (let y = 0; y < h; y += RULER_STEP) {
        const topPx = y * cell;

        const lineY = makeTickLine('y');
        lineY.style.top = topPx + 'px';

        const labY = makeTickLabel(y);
        labY.style.top = (topPx + 2) + 'px';
        labY.style.right = '4px';
        labY.style.textAlign = 'right';

        rulerLeftInner.appendChild(lineY);
        rulerLeftInner.appendChild(labY);
      }

      syncNow();
    }

    function bind() {
      if (gridWrap) {
        gridWrap.addEventListener('scroll', () => {
          requestSync();
        });
      }

      window.addEventListener('resize', () => {
        requestSync();
      });
    }

    return {
      render,
      syncNow,
      requestSync,
      bind
    };
  }

  window.appUiRulerView = {
    create
  };
})();
