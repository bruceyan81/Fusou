// io.js
// Import/export helpers for map assets and tool palette data.

(function (global) {
  'use strict';

  function sanitizeFileName(name) {
    let s = String(name || 'download.json');
    // Remove characters invalid on Windows filenames and common path separators.
    s = s.replace(/[\\\/:*?\x22<>|]+/g, '_');
    s = s.replace(/\s+/g, ' ').trim();
    if (!s) {
      s = 'download.json';
    }
    // Prevent super long names (simple cap).
    if (s.length > 180) {
      s = s.slice(0, 180);
    }
    return s;
  }

  function downloadBlob(blob, filename) {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = sanitizeFileName(filename || 'download.json');
    a.style.display = 'none';
    document.body.appendChild(a);
    a.click();
    a.remove();
    // Release the blob URL as soon as possible.
    setTimeout(() => { URL.revokeObjectURL(url); }, 0);
  }

  function readTextFromFile(file) {
    if (!file) {
      return Promise.reject(new Error('readTextFromFile: file missing'));
    }

    // Prefer promise-based Blob.text() when available.
    if (typeof file.text === 'function') {
      return file.text();
    }

    // Fallback to FileReader.readAsText().
    return new Promise((resolve, reject) => {
      const fr = new FileReader();
      fr.onerror = () => { reject(fr.error || new Error('FileReader error')); };
      fr.onload = () => { resolve(String(fr.result || '')); };
      fr.readAsText(file);
    });
  }

  function suggestAssetFileName(asset, options) {
    const fileOptions = options || {};
    const base = fileOptions.mapName ? String(fileOptions.mapName) : 'map';
    const width = (asset && asset.width) | 0;
    const height = (asset && asset.height) | 0;
    return sanitizeFileName(`${base}_${width}x${height}.json`);
  }

  function stripBomLocal(text) {
    if (typeof text !== 'string') {
      return text;
    }
    if (text.charCodeAt(0) === 0xFEFF) {
      return text.slice(1);
    }
    return text;
  }

  function coercePaletteArrayForV2(palette) {
    // palette is expected to be an array of {tileId, fg, bg}
    // Strict AssetSchemaV2 validation runs after this shape normalization.
    if (!Array.isArray(palette)) {
      return [];
    }
    const normalizedPalette = [];
    for (let i = 0; i < palette.length; i++) {
      const entry = palette[i] || {};
      normalizedPalette.push({
        tileId: Number(entry.tileId),
        fg: Number(entry.fg),
        bg: Number(entry.bg),
      });
    }
    return normalizedPalette;
  }

  function buildAssetV2(assetMap, paletteArray) {
    return {
      version: 2,
      width: assetMap.width,
      height: assetMap.height,
      tiles: assetMap.tiles,
      palette: coercePaletteArrayForV2(paletteArray),
    };
  }

  function exportAssetAsJson(asset, options) {
    const exportOptions = options || {};

    if (exportOptions.palette === null || exportOptions.palette === undefined) {
      throw new Error('export asset failed: palette is required for JSON asset export');
    }

    if (!global.assetSchemaV2) {
      throw new Error('assetSchemaV2 not loaded');
    }

    // First, validate/normalize the map part using the stable internal contract.
    if (!global.internalAssetSchema) {
      throw new Error('internalAssetSchema not loaded');
    }
    const internalValidation = global.internalAssetSchema.coerceAndValidate(asset);
    if (!internalValidation.ok) {
      throw new Error(
        `export asset failed (map invalid):\n${(internalValidation.errors || []).join('\n')}`
      );
    }

    const assetV2 = buildAssetV2(internalValidation.value, exportOptions.palette);
    const assetV2Validation = global.assetSchemaV2.coerceAndValidate(assetV2);
    if (!assetV2Validation.ok) {
      throw new Error(
        `export asset failed (JSON asset invalid):\n${(assetV2Validation.errors || []).join('\n')}`
      );
    }

    const assetJson = JSON.stringify(assetV2Validation.value, null, 2);
    const assetBlob = new Blob([assetJson], { type: 'application/json;charset=utf-8' });
    const assetFilename = sanitizeFileName(
      exportOptions.filename ||
      exportOptions.fileName ||
      suggestAssetFileName(internalValidation.value, exportOptions)
    );
    downloadBlob(assetBlob, assetFilename);
  }

  function parseAssetJsonText(text) {
    if (!global.internalAssetSchema) {
      return { ok: false, errors: ['internalAssetSchema not loaded'] };
    }
    if (!global.detectAssetVersion) {
      return { ok: false, errors: ['detectAssetVersion not loaded'] };
    }

    const raw = String(text || '');
    let obj;

    try {
      obj = JSON.parse(stripBomLocal(raw));
    } catch (e) {
      const msg = (e && e.message) ? e.message : String(e);
      return { ok: false, errors: [`Invalid JSON: ${msg}`] };
    }

    const ver = global.detectAssetVersion(obj);
    if (ver === 2) {
      if (!global.assetSchemaV2) {
        return { ok: false, errors: ['assetSchemaV2 not loaded'] };
      }

      const assetV2Validation = global.assetSchemaV2.coerceAndValidate(obj);
      if (!assetV2Validation.ok) {
        return { ok: false, errors: assetV2Validation.errors || ['Asset v2 invalid'] };
      }

      const internalMap = {
        // Internal state keeps its own version separate from exported JSON assets.
        version: 1,
        width: assetV2Validation.value.width,
        height: assetV2Validation.value.height,
        tiles: assetV2Validation.value.tiles,
      };

      return {
        ok: true,
        value: internalMap,
        detectedVersion: 2,
        importedPalette: assetV2Validation.value.palette || [],
      };
    }

    // Fail fast instead of accepting a shape the editor cannot round-trip.
    return { ok: false, errors: ['Unsupported asset version (expect v2)'] };
  }

  function suggestPaletteFileName(options) {
    const opt = options || {};
    const base = opt.name ? String(opt.name) : 'palette';
    return sanitizeFileName(`${base}.json`);
  }

  function exportPaletteConfigAsJson(paletteConfig, options) {
    if (!global.paletteConfigSchema) {
      throw new Error('paletteConfigSchema not loaded');
    }

    const validation = global.paletteConfigSchema.coerceAndValidate(paletteConfig);
    if (!validation.ok) {
      throw new Error(`export palette failed:\n${(validation.errors || []).join('\n')}`);
    }

    const exportOptions = options || {};
    const paletteJson = JSON.stringify(validation.value, null, 2);
    const paletteBlob = new Blob([paletteJson], { type: 'application/json;charset=utf-8' });

    const filename = sanitizeFileName(
      exportOptions.filename || exportOptions.fileName || suggestPaletteFileName(exportOptions)
    );
    downloadBlob(paletteBlob, filename);
  }

  function parsePaletteJsonText(text) {
    if (!global.paletteConfigSchema) {
      return { ok: false, errors: ['paletteConfigSchema not loaded'] };
    }
    return global.paletteConfigSchema.parseJsonAndValidate(String(text || ''));
  }

  global.ioAsset = {
    exportAssetAsJson,
    parseAssetJsonText,
    readTextFromFile,
  };

  global.ioPalette = {
    exportPaletteConfigAsJson,
    parsePaletteJsonText,
    readTextFromFile,
  };

})(window);
