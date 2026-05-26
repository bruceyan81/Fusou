// Runtime state for the editor.
// This module keeps only the current asset and palette state shape.

(function (global) {
  'use strict';

  // Runtime state mirrors schema integer rules before deciding whether to use fallbacks.
  function toIntStrict(v) {
    const n = (typeof v === 'string') ? Number(v.trim()) : Number(v);
    if (!Number.isFinite(n)) {
      return { ok: false, value: 0 };
    }
    const t = Math.trunc(n);
    if (t !== n) {
      return { ok: false, value: 0 };
    }
    if (!Number.isSafeInteger(t)) {
      return { ok: false, value: 0 };
    }
    return { ok: true, value: t };
  }

  // State mutations use this after validation boundaries to keep tile ids non-negative.
  function clampNonNegInt(v, fallback) {
    const r = toIntStrict(v);
    if (!r.ok) {
      return fallback | 0;
    }
    if (r.value < 0) {
      return fallback | 0;
    }
    return r.value | 0;
  }

  // Return a detached palette object so callers cannot mutate internal state by reference.
  function copyPaletteConfig(pc) {
    if (!pc || !Array.isArray(pc.entries)) {
      return { version: 1, entries: [] };
    }
    return {
      version: (pc.version | 0) || 1,
      entries: pc.entries.map((e) => {
        return {
          id: (e && e.id) | 0,
          glyph: String(e && e.glyph !== null && e.glyph !== undefined ? e.glyph : ''),
          name: String(e && e.name !== null && e.name !== undefined ? e.name : ''),
        };
      }),
    };
  }

  function getAssetLimits() {
    const limits = (global.appConstAsset && global.appConstAsset.LIMITS) || null;
    // State keeps its own guard because resize/load can be called outside the form UI.
    const minW = (limits && limits.MIN_W !== null && limits.MIN_W !== undefined)
      ? (limits.MIN_W | 0)
      : 1;
    const minH = (limits && limits.MIN_H !== null && limits.MIN_H !== undefined)
      ? (limits.MIN_H | 0)
      : 1;
    const maxW = (limits && limits.MAX_W !== null && limits.MAX_W !== undefined)
      ? (limits.MAX_W | 0)
      : 500;
    const maxH = (limits && limits.MAX_H !== null && limits.MAX_H !== undefined)
      ? (limits.MAX_H | 0)
      : 500;
    const maxCells = (limits && limits.MAX_CELLS !== null && limits.MAX_CELLS !== undefined)
      ? (limits.MAX_CELLS | 0)
      : (maxW * maxH);

    return {
      minW,
      minH,
      maxW,
      maxH,
      maxCells
    };
  }

  // Default runtime data stays aligned with constants when they are available.
  function normalizeDefaultAsset() {
    const C = global.appConstAsset && global.appConstAsset.DEFAULTS
      ? global.appConstAsset.DEFAULTS
      : null;
    let w = (C && C.WIDTH) ? (C.WIDTH | 0) : 40;
    let h = (C && C.HEIGHT) ? (C.HEIGHT | 0) : 25;
    w = Math.max(1, w | 0);
    h = Math.max(1, h | 0);
    return { version: 1, width: w, height: h, tiles: new Array(w * h).fill(0) };
  }

  function normalizeDefaultPalette() {
    return { version: 1, entries: [] };
  }

  // AssetState is the only live runtime state container used by the current app.
  function createAssetState(initialAsset, initialPalette) {
    if (!global.internalAssetSchema) {
      throw new Error('internalAssetSchema not loaded');
    }
    if (!global.paletteConfigSchema) {
      throw new Error('paletteConfigSchema not loaded');
    }

    // Validate incoming snapshots once, then store normalized runtime primitives locally.
    let asset;
    if (initialAsset !== null && initialAsset !== undefined) {
      const vr0 = global.internalAssetSchema.coerceAndValidate(initialAsset);
      if (!vr0.ok) {
        throw new Error('Initial asset invalid:\n' + (vr0.errors || []).join('\n'));
      }
      asset = vr0.value;
    } else {
      asset = normalizeDefaultAsset();
    }

    let width = asset.width | 0;
    let height = asset.height | 0;

    // Tile ids are always non-negative; Uint32Array keeps storage compact and fixed-size.
    let tiles = new Uint32Array(asset.tiles.map((x) => {
      return (clampNonNegInt(x, 0) >>> 0);
    }));

    let paletteConfig;
    if (initialPalette !== null && initialPalette !== undefined) {
      const pr0 = global.paletteConfigSchema.coerceAndValidate(initialPalette);
      if (!pr0.ok) {
        throw new Error('Initial palette invalid:\n' + (pr0.errors || []).join('\n'));
      }
      paletteConfig = pr0.value;
    } else {
      paletteConfig = normalizeDefaultPalette();
    }

    // Subscribers receive coarse-grained patches so views can react without sharing mutable state.
    const listeners = [];

    function subscribe(fn) {
      listeners.push(fn);
      return () => {
        const i = listeners.indexOf(fn);
        if (i >= 0) {
          listeners.splice(i, 1);
        }
      };
    }

    function emit(patch) {
      for (let i = 0; i < listeners.length; i++) {
        try { listeners[i](patch); } catch (e) { console.error(e); }
      }
    }

    function getSize() {
      return { width: width | 0, height: height | 0 };
    }

    function indexOfXY(x, y) { return (y | 0) * width + (x | 0); }

    function getTileAtIndex(i) {
      i = i | 0;
      if (i < 0 || i >= tiles.length) {
        return 0;
      }
      return tiles[i] >>> 0;
    }

    function setTilesAtIndices(indices, values) {
      if (!Array.isArray(indices) || !Array.isArray(values)) {
        return;
      }
      if (indices.length !== values.length) {
        return;
      }

      const changedI = [];
      const changedV = [];

      for (let k = 0; k < indices.length; k++) {
        const i = indices[k] | 0;
        if (i < 0 || i >= tiles.length) {
          continue;
        }

        const v = clampNonNegInt(values[k], 0) >>> 0;
        if (tiles[i] === v) {
          continue;
        }

        tiles[i] = v;
        changedI.push(i);
        changedV.push(v);
      }

      if (changedI.length > 0) {
        emit({ type: 'tile', indices: changedI, values: changedV });
      }
    }

    function setTileAtIndex(i, id) {
      i = i | 0;
      if (i < 0 || i >= tiles.length) {
        return;
      }

      const v = clampNonNegInt(id, 0) >>> 0;
      if (tiles[i] === v) {
        return;
      }

      tiles[i] = v;
      emit({ type: 'tile', indices: [i], values: [v] });
    }

    function setTileXY(x, y, id) {
      setTileAtIndex(indexOfXY(x, y), id);
    }

    function clearAll() {
      tiles.fill(0);
      emit({ type: 'clear' });
    }

    function resize(newW, newH, policy) {
      const limits = getAssetLimits();
      if (newW < limits.minW || newW > limits.maxW) {
        return false;
      }
      if (newH < limits.minH || newH > limits.maxH) {
        return false;
      }
      if ((newW * newH) > limits.maxCells) {
        return false;
      }

      newW = clampNonNegInt(newW, 1);
      newH = clampNonNegInt(newH, 1);
      policy = (policy === 'keepTopLeft') ? 'keepTopLeft' : 'clear';

      const oldW = width;
      const oldH = height;
      const old = tiles;

      width = newW | 0;
      height = newH | 0;
      tiles = new Uint32Array(width * height); // initialized to 0

      if (policy === 'keepTopLeft') {
        const copyW = Math.min(oldW, width);
        const copyH = Math.min(oldH, height);
        for (let y = 0; y < copyH; y++) {
          const fromBase = y * oldW;
          const toBase = y * width;
          for (let x = 0; x < copyW; x++) {
            tiles[toBase + x] = old[fromBase + x];
          }
        }
      }

      emit({ type: 'resize', width: width | 0, height: height | 0, policy });
      return true;
    }

    function getAsset() {
      // Export a plain object so IO code never depends on typed arrays.
      return {
        version: 1,
        width: width | 0,
        height: height | 0,
        tiles: Array.from(tiles, (x) => { return Number(x >>> 0); }),
      };
    }

    function loadAsset(nextAsset) {
      // Validate first so rejected imports never partially mutate the live state.
      const vr = global.internalAssetSchema.coerceAndValidate(nextAsset);
      if (!vr.ok) {
        emit({ type: 'loadError', errors: vr.errors || [] });
        return false;
      }

      const w = vr.value.width | 0;
      const h = vr.value.height | 0;
      const limits = getAssetLimits();

      // Apply limits to the imported dimensions before swapping any runtime fields.
      if (w < limits.minW || w > limits.maxW) {
        emit({
          type: 'loadError',
          errors: ['width out of range: ' + limits.minW + '-' + limits.maxW]
        });
        return false;
      }
      if (h < limits.minH || h > limits.maxH) {
        emit({
          type: 'loadError',
          errors: ['height out of range: ' + limits.minH + '-' + limits.maxH]
        });
        return false;
      }
      if ((w * h) > limits.maxCells) {
        emit({ type: 'loadError', errors: ['map too large: cells limit exceeded'] });
        return false;
      }

      // Swap the runtime snapshot only after validation and limit checks both pass.
      asset = vr.value;
      width = w;
      height = h;
      tiles = new Uint32Array(vr.value.tiles.map((x) => {
        return (clampNonNegInt(x, 0) >>> 0);
      }));

      emit({ type: 'load' });
      return true;
    }

    function getPaletteConfig() {
      return copyPaletteConfig(paletteConfig);
    }

    // Whole-palette replacement keeps import and table editing on the same validation path.
    function replacePaletteConfig(next) {
      const vr = global.paletteConfigSchema.coerceAndValidate(next);
      if (!vr.ok) {
        emit({ type: 'paletteError', errors: vr.errors || [] });
        return false;
      }
      paletteConfig = vr.value;
      emit({ type: 'palette', palette: copyPaletteConfig(paletteConfig) });
      return true;
    }

    function setPaletteEntry(id, glyph, name) {
      const idr = toIntStrict(id);
      if (!idr.ok || idr.value < 0) {
        emit({ type: 'paletteError', errors: ['palette id must be int >= 0'] });
        return false;
      }

      // Glyphs remain display-driven; the schema only enforces entry shape.
      const entry = {
        id: idr.value,
        glyph: String(glyph !== null && glyph !== undefined ? glyph : ''),
        name: String(name !== null && name !== undefined ? name : '')
      };

      // Upsert by id so callers do not need to branch between add and edit flows.
      const cur = getPaletteConfig();
      const kept = cur.entries.filter((e) => { return (e.id | 0) !== (entry.id | 0); });
      kept.push(entry);
      kept.sort((a, b) => { return (a.id | 0) - (b.id | 0); });

      return replacePaletteConfig({ version: cur.version | 0, entries: kept });
    }

    function deletePaletteEntry(id) {
      const idr = toIntStrict(id);
      if (!idr.ok || idr.value < 0) {
        emit({ type: 'paletteError', errors: ['palette id must be int >= 0'] });
        return false;
      }

      const cur = getPaletteConfig();
      const kept = cur.entries.filter((e) => { return (e.id | 0) !== (idr.value | 0); });
      return replacePaletteConfig({ version: cur.version | 0, entries: kept });
    }

    return {
      // tiles/model
      getSize,
      getTileAtIndex,
      setTileAtIndex,
      setTileXY,
      clearAll,
      resize,
      getAsset,
      loadAsset,

      // palette
      getPaletteConfig,
      replacePaletteConfig,
      setPaletteEntry,
      deletePaletteEntry,

      // events
      subscribe,
      setTilesAtIndices,

    };
  }

  global.assetState = { createState: createAssetState };
  global.AssetState = global.assetState;

})(window);
