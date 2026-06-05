// schema.js
// Runtime JSON validators for map assets and tool palette data.

(function (global) {
  'use strict';

  function stripBom(text) {
    // JSON.parse may throw if the string starts with BOM (U+FEFF).
    // We accept and ignore it.
    if (typeof text !== 'string') {
      return text;
    }
    if (text.charCodeAt(0) === 0xFEFF) {
      return text.slice(1);
    }
    return text;
  }

  function isPlainObject(v) {
    return (
      typeof v === 'object' &&
      v !== null &&
      (v.constructor === Object || Object.getPrototypeOf(v) === Object.prototype)
    );
  }

  // Schema validation reports invalid integers to callers instead of choosing fallbacks.
  function toIntStrict(v) {
    // Accept number or numeric string; reject floats and unsafe integers.
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

  function getMaxTileId() {
    const generated = global.appGeneratedTilePalette || null;
    if (generated && generated.maxTileId !== null && generated.maxTileId !== undefined) {
      return generated.maxTileId | 0;
    }
    return 32;
  }

  // MapForge internal runtime format:
  //   top-level object
  //   version: int
  //   width : int > 0
  //   height: int > 0
  //   tiles : array; length == width*height; each int >= 0; row-major by index rule
  //
  // This is not the final JSON format consumed by the Fusou runtime.
  // Exported assets use AssetSchemaV2, which follows JsonSchema.json.
  const INTERNAL_ASSET_SCHEMA_VERSION = 1;

  function validateInternalAsset(input) {
    const errors = [];

    if (!isPlainObject(input)) {
      return { ok: false, errors: ['Asset JSON root must be an object'] };
    }

    const vr = toIntStrict(input.version);
    if (!vr.ok) {
      errors.push('version must exist and be readable as an int');
    }

    const wr = toIntStrict(input.width);
    if (!wr.ok || wr.value <= 0) {
      errors.push('width must be an int and > 0');
    }

    const hr = toIntStrict(input.height);
    if (!hr.ok || hr.value <= 0) {
      errors.push('height must be an int and > 0');
    }

    const limitsSource = (global.appConstAsset && global.appConstAsset.LIMITS) || null;

    const width = wr.value | 0;
    const height = hr.value | 0;
    if (limitsSource) {
      // Schema validation rejects imported or exported JSON before it reaches state.
      const minW = (limitsSource.MIN_W | 0) || 1;
      const minH = (limitsSource.MIN_H | 0) || 1;
      const maxW = (limitsSource.MAX_W | 0) || 500;
      const maxH = (limitsSource.MAX_H | 0) || 500;
      const maxCells = (
        limitsSource.MAX_CELLS !== null &&
        limitsSource.MAX_CELLS !== undefined
      )
        ? (limitsSource.MAX_CELLS | 0)
        : (maxW * maxH);

      if (width < minW || width > maxW) {
        errors.push(`width out of range: ${minW}-${maxW}`);
      }
      if (height < minH || height > maxH) {
        errors.push(`height out of range: ${minH}-${maxH}`);
      }
      if ((width * height) > maxCells) {
        errors.push('map too large: cells limit exceeded');
      }
    }
    if (errors.length) {
      return { ok: false, errors };
    }

    const tiles = input.tiles;
    if (!Array.isArray(tiles)) {
      errors.push('tiles must be an array');
    }

    if (errors.length) {
      return { ok: false, errors };
    }

    const expected = width * height;

    if (tiles.length !== expected) {
      return {
        ok: false,
        errors: [
          `tiles.size() must equal width * height (expected ${expected}, actual ${tiles.length})`
        ],
      };
    }

    const outTiles = new Array(expected);
    for (let i = 0; i < expected; i++) {
      const ir = toIntStrict(tiles[i]);
      if (!ir.ok) {
        return { ok: false, errors: [`tiles[${i}] must be readable as an int`] };
      }
      if (ir.value < 0) {
        return { ok: false, errors: [`tiles[${i}] must be >= 0 (actual ${ir.value})`] };
      }
      outTiles[i] = ir.value;
    }

    const value = {
      version: vr.ok ? (vr.value | 0) : INTERNAL_ASSET_SCHEMA_VERSION,
      width,
      height,
      tiles: outTiles,
    };

    return { ok: true, value };
  }

  function parseJsonAndValidateInternalAsset(text) {
    if (typeof text !== 'string') {
      return { ok: false, errors: ['JSON text must be a string'] };
    }
    try {
      const obj = JSON.parse(stripBom(text));
      return validateInternalAsset(obj);
    } catch (e) {
      const msg = (e && e.message) ? e.message : String(e);
      return { ok: false, errors: [`Invalid JSON: ${msg}`] };
    }
  }

  global.internalAssetSchema = {
    SCHEMA_VERSION: INTERNAL_ASSET_SCHEMA_VERSION,
    validate: validateInternalAsset,
    coerceAndValidate: validateInternalAsset,
    parseJsonAndValidate: parseJsonAndValidateInternalAsset,
  };

  // MapForge-only palette configuration:
  //   { version:int, entries:[ {id:int>=0, glyph:string, name?:string} ] }
  // Glyph display width and character count are not validated here.
  const PALETTE_CONFIG_SCHEMA_VERSION = 1;

  function validatePaletteConfig(input) {
    const errors = [];

    if (!isPlainObject(input)) {
      return { ok: false, errors: ['PaletteConfig root must be an object'] };
    }

    const vr = toIntStrict(input.version);
    if (!vr.ok) {
      errors.push('version must exist and be readable as an int');
    }

    const entries = input.entries;
    if (!Array.isArray(entries)) {
      errors.push('entries must be an array');
    }

    if (errors.length) {
      return { ok: false, errors };
    }

    const seen = new Set();
    const out = [];

    for (let i = 0; i < entries.length; i++) {
      const e = entries[i];
      if (!isPlainObject(e)) {
        return { ok: false, errors: [`entries[${i}] must be an object`] };
      }

      const idr = toIntStrict(e.id);
      if (!idr.ok || idr.value < 0) {
        return { ok: false, errors: [`entries[${i}].id must be an int and >= 0`] };
      }

      if (seen.has(idr.value)) {
        return { ok: false, errors: [`entries contains duplicate id=${idr.value}`] };
      }
      seen.add(idr.value);

      const glyph = (e.glyph === null || e.glyph === undefined) ? '' : String(e.glyph);
      const name = (e.name === null || e.name === undefined) ? '' : String(e.name);

      out.push({ id: idr.value, glyph, name });
    }

    return {
      ok: true,
      value: {
        version: vr.ok ? (vr.value | 0) : PALETTE_CONFIG_SCHEMA_VERSION,
        entries: out,
      },
    };
  }

  function parseJsonAndValidatePaletteConfig(text) {
    if (typeof text !== 'string') {
      return { ok: false, errors: ['JSON text must be a string'] };
    }
    try {
      const obj = JSON.parse(stripBom(text));
      return validatePaletteConfig(obj);
    } catch (e) {
      const msg = (e && e.message) ? e.message : String(e);
      return { ok: false, errors: [`Invalid JSON: ${msg}`] };
    }
  }

  global.paletteConfigSchema = {
    SCHEMA_VERSION: PALETTE_CONFIG_SCHEMA_VERSION,
    validate: validatePaletteConfig,
    coerceAndValidate: validatePaletteConfig,
    parseJsonAndValidate: parseJsonAndValidatePaletteConfig,
  };

  // Asset format exported for the Fusou runtime:
  //   top-level object
  //   version: int == 2
  //   width : int > 0
  //   height: int > 0
  //   tiles : array; length == width*height; each int >= 0
  //   palette: array of { tileId:int>=0, fg:int in [0..15], bg:int in [0..15] }
  // The root JsonSchema.json and C++ loader expect this format.
  const ASSET_SCHEMA_VERSION = 2;

  function validateAssetV2(input) {
    const errors = [];

    if (!isPlainObject(input)) {
      return { ok: false, errors: ['Asset JSON root must be an object'] };
    }

    const vr = toIntStrict(input.version);
    if (!vr.ok || vr.value !== ASSET_SCHEMA_VERSION) {
      errors.push('version must be an int and == 2');
    }

    const wr = toIntStrict(input.width);
    if (!wr.ok || wr.value <= 0) {
      errors.push('width must be an int and > 0');
    }

    const hr = toIntStrict(input.height);
    if (!hr.ok || hr.value <= 0) {
      errors.push('height must be an int and > 0');
    }

    const limitsSource = (global.appConstAsset && global.appConstAsset.LIMITS) || null;

    const width = wr.value | 0;
    const height = hr.value | 0;
    if (limitsSource) {
      // Schema validation rejects imported or exported JSON before it reaches state.
      const minW = (limitsSource.MIN_W | 0) || 1;
      const minH = (limitsSource.MIN_H | 0) || 1;
      const maxW = (limitsSource.MAX_W | 0) || 500;
      const maxH = (limitsSource.MAX_H | 0) || 500;
      const maxCells = (
        limitsSource.MAX_CELLS !== null &&
        limitsSource.MAX_CELLS !== undefined
      )
        ? (limitsSource.MAX_CELLS | 0)
        : (maxW * maxH);

      if (width < minW || width > maxW) {
        errors.push(`width out of range: ${minW}-${maxW}`);
      }
      if (height < minH || height > maxH) {
        errors.push(`height out of range: ${minH}-${maxH}`);
      }
      if ((width * height) > maxCells) {
        errors.push('map too large: cells limit exceeded');
      }
    }

    if (errors.length) {
      return { ok: false, errors };
    }

    // tiles
    const tiles = input.tiles;
    if (!Array.isArray(tiles)) {
      errors.push('tiles must be an array');
    }
    if (errors.length) {
      return { ok: false, errors };
    }

    const expected = width * height;
    if (tiles.length !== expected) {
      return {
        ok: false,
        errors: [
          `tiles.size() must equal width * height (expected ${expected}, actual ${tiles.length})`
        ],
      };
    }

    const outTiles = new Array(expected);
    for (let i = 0; i < expected; i++) {
      const ir = toIntStrict(tiles[i]);
      if (!ir.ok) {
        return { ok: false, errors: [`tiles[${i}] must be readable as an int`] };
      }
      if (ir.value < 0 || ir.value > getMaxTileId()) {
        return {
          ok: false,
          errors: [`tiles[${i}] must be in 0..${getMaxTileId()} (actual ${ir.value})`]
        };
      }
      outTiles[i] = ir.value;
    }

    // palette
    const palette = (input.palette === null || input.palette === undefined) ? [] : input.palette;
    if (!Array.isArray(palette)) {
      errors.push('palette must be an array');
    }
    if (errors.length) {
      return { ok: false, errors };
    }

    const seenTileId = new Set();
    const outPalette = [];

    for (let p = 0; p < palette.length; p++) {
      const e = palette[p];
      if (!isPlainObject(e)) {
        return { ok: false, errors: [`palette[${p}] must be an object`] };
      }

      const tid = toIntStrict(e.tileId);
      if (!tid.ok || tid.value < 0 || tid.value > getMaxTileId()) {
        return {
          ok: false,
          errors: [`palette[${p}].tileId must be an int and in 0..${getMaxTileId()}`]
        };
      }

      const fg = toIntStrict(e.fg);
      if (!fg.ok || fg.value < 0 || fg.value > 15) {
        return { ok: false, errors: [`palette[${p}].fg must be an int and in 0..15`] };
      }

      const bg = toIntStrict(e.bg);
      if (!bg.ok || bg.value < 0 || bg.value > 15) {
        return { ok: false, errors: [`palette[${p}].bg must be an int and in 0..15`] };
      }

      if (seenTileId.has(tid.value)) {
        return { ok: false, errors: [`palette contains duplicate tileId=${tid.value}`] };
      }
      seenTileId.add(tid.value);

      outPalette.push({ tileId: tid.value | 0, fg: fg.value | 0, bg: bg.value | 0 });
    }

    return {
      ok: true,
      value: {
        version: ASSET_SCHEMA_VERSION,
        width,
        height,
        tiles: outTiles,
        palette: outPalette,
      },
    };
  }

  function parseJsonAndValidateAssetV2(text) {
    if (typeof text !== 'string') {
      return { ok: false, errors: ['JSON text must be a string'] };
    }
    try {
      const obj = JSON.parse(stripBom(text));
      return validateAssetV2(obj);
    } catch (e) {
      const msg = (e && e.message) ? e.message : String(e);
      return { ok: false, errors: [`Invalid JSON: ${msg}`] };
    }
  }

  // Asset importer uses this before selecting the strict validator.
  function detectAssetVersion(obj) {
    if (!isPlainObject(obj)) {
      return null;
    }
    const vr = toIntStrict(obj.version);
    if (!vr.ok) {
      return null;
    }
    if (vr.value === 2) {
      return 2;
    }
    return null;
  }

  global.assetSchemaV2 = {
    SCHEMA_VERSION: ASSET_SCHEMA_VERSION,
    validate: validateAssetV2,
    coerceAndValidate: validateAssetV2,
    parseJsonAndValidate: parseJsonAndValidateAssetV2,
  };
  global.detectAssetVersion = detectAssetVersion;

})(window);
