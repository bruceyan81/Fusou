// helpers/app-context.js
// Assembles the static-page modules into one runtime context.
// Dependency direction: DOM refs and state are created first, interaction helpers
// attach to them, palette helpers bridge state and UI, and feature modules bind last.

(function () {
  'use strict';

  function createViews(win, refs) {
    return {
      statusView: win.appUiStatusView.create(refs),
      rulerView: win.appUiRulerView.create(refs),
      gridView: win.appUiGridView.create(refs),
      paletteView: win.appUiPaletteView.create(refs)
    };
  }

  function createLimits(appBoot, refs, appConstAsset) {
    const assetLimits = appBoot.getLimits(appConstAsset);
    appBoot.applyNumberInputLimits(refs.inputWidth, refs.inputHeight, assetLimits);

    return {
      assetLimits,
      getLimits: () => {
        return assetLimits;
      }
    };
  }

  function createPaletteRuntime(win) {
    const console16 = (win.appConstAsset && win.appConstAsset.CONSOLE16)
      ? win.appConstAsset.CONSOLE16
      : null;

    const defaultImportFg = (console16 && typeof console16.DEFAULT_FG === 'number')
      ? (console16.DEFAULT_FG | 0)
      : 0;
    const defaultImportBg = (console16 && typeof console16.DEFAULT_BG === 'number')
      ? (console16.DEFAULT_BG | 0)
      : 15;

    const brushColorHelpers = win.appHelpersBrushColors.create({
      console16
    });

    const paletteHelpers = win.appHelpersPalette.create({
      defaultImportFg,
      defaultImportBg
    });

    const brushColorRuntime = win.appHelpersBrushColorRuntime.create({
      brushColorHelpers,
      defaultBrushColors:
        win.appDefaults && win.appDefaults.DEFAULT_BRUSH_COLORS
          ? win.appDefaults.DEFAULT_BRUSH_COLORS
          : null
    });

    const paletteBridge = win.appHelpersPaletteBridge.create({
      paletteHelpers,
      brushColorHelpers,
      brushColorRuntime
    });

    return {
      brushColorHelpers,
      paletteHelpers,
      brushColorRuntime,
      paletteBridge,
      getBrushColor: (id) => {
        return brushColorRuntime.getBrushColor(id);
      }
    };
  }

  function createInteractionRuntime(win, doc, refs, state) {
    const coordTipHelper = win.appHelpersCoordTip.create({
      coordTipEl: refs.coordTipEl,
      state,
      windowRef: win,
      coordTipOffsetX: 12,
      coordTipOffsetY: 14
    });

    const gridHoverHelper = win.appHelpersGridHover.create({
      gridRoot: refs.gridRoot,
      documentRef: doc,
      coordTip: coordTipHelper
    });

    const paintingSessionHelper = win.appHelpersPaintingSession.create({
      selectRenderBrush: refs.selectRenderBrush
    });

    coordTipHelper.setHoverRuntimeProvider(gridHoverHelper.getHoverRuntime);

    const paintingRuntimeApi = {
      hideCoordTip: gridHoverHelper.hideCoordTip,
      requestHoverTipUpdate: gridHoverHelper.requestHoverTipUpdate,

      setHoverRuntime: gridHoverHelper.setHoverRuntime,
      getHoverRuntime: gridHoverHelper.getHoverRuntime,

      hitIndexFromEventTarget: gridHoverHelper.hitIndexFromEventTarget,
      hitIndexFromPoint: gridHoverHelper.hitIndexFromPoint,

      setIsPointerDown: paintingSessionHelper.setIsPointerDown,
      getIsPointerDown: paintingSessionHelper.getIsPointerDown,

      setLastPaintIndex: paintingSessionHelper.setLastPaintIndex,
      getLastPaintIndex: paintingSessionHelper.getLastPaintIndex,

      getPaintRaf: paintingSessionHelper.getPaintRaf,
      setPaintRaf: paintingSessionHelper.setPaintRaf,

      getPendingPaint: paintingSessionHelper.getPendingPaint
    };

    return {
      coordTipHelper,
      gridHoverHelper,
      paintingSessionHelper,
      paintingRuntimeApi
    };
  }

  function createPaletteUi(state, views, paletteRuntime, paletteEditingFeatureRef) {
    function getPaletteEditingFeature() {
      return paletteEditingFeatureRef && paletteEditingFeatureRef.current
        ? paletteEditingFeatureRef.current
        : null;
    }

    function rebuildBrushOptions(paletteConfig) {
      views.paletteView.rebuildBrushOptions(paletteConfig);
    }

    function rebuildPaletteTable(paletteConfig) {
      const paletteEditingFeature = getPaletteEditingFeature();

      views.paletteView.rebuildPaletteTable(paletteConfig, {
        reconcileBrushColors: paletteRuntime.paletteBridge.reconcileBrushColors,
        getBrushColor: paletteRuntime.getBrushColor,
        onColorChange: paletteEditingFeature ? paletteEditingFeature.onColorChange : null,
        onEntryChange: paletteEditingFeature ? paletteEditingFeature.onEntryChange : null
      });
    }

    function renderAllCells() {
      views.gridView.renderAllCells(
        state,
        paletteRuntime.getBrushColor,
        views.rulerView.render
      );
    }

    return {
      rebuildPaletteMap: views.gridView.rebuildPaletteMap,
      rebuildBrushOptions,
      rebuildPaletteTable,
      renderAllCells
    };
  }

  function createPaletteSync(win, refs, state, views, sessions, paletteRuntime, paletteUi) {
    return {
      uiSync: win.appHelpersAppStateSync.create({
        importSession: sessions.importSession,
        importInfoEl: refs.importInfo,

        gridView: views.gridView,
        rulerView: views.rulerView,
        state,
        getBrushColor: paletteRuntime.getBrushColor,
        rebuildPaletteMap: paletteUi.rebuildPaletteMap,
        rebuildBrushOptions: paletteUi.rebuildBrushOptions,
        rebuildPaletteTable: paletteUi.rebuildPaletteTable,

        paletteView: views.paletteView,
        paletteBridge: paletteRuntime.paletteBridge
      })
    };
  }

  function createPaletteEditingFeature(
    win,
    state,
    views,
    paletteRuntime,
    interactionRuntime,
    paletteUi
  ) {
    const statusView = views.statusView;
    const brushColorRuntime = paletteRuntime.brushColorRuntime;
    const paletteBridge = paletteRuntime.paletteBridge;
    const paintingSessionHelper = interactionRuntime.paintingSessionHelper;

    return win.appFeaturePaletteEditing.create({
      runtime: {
        state,
        reconcileBrushColors: paletteBridge.reconcileBrushColors,
        getBrushColors: brushColorRuntime.getBrushColors,
        getBrushId: paintingSessionHelper.getBrushId,
        setPaintId: paintingSessionHelper.setBrushId
      },

      status: {
        clearStatus: statusView.clearStatus,
        setStatus: statusView.setStatus
      },

      ui: paletteUi
    });
  }

  function createFeatures(
    win,
    doc,
    refs,
    state,
    limits,
    views,
    sessions,
    sync,
    paletteRuntime,
    interactionRuntime
  ) {
    const uiSync = sync.uiSync;
    const importSession = sessions.importSession;
    const statusView = views.statusView;
    const gridView = views.gridView;
    const brushColorRuntime = paletteRuntime.brushColorRuntime;
    const paletteBridge = paletteRuntime.paletteBridge;
    const paintingSessionHelper = interactionRuntime.paintingSessionHelper;
    const paintingRuntimeApi = interactionRuntime.paintingRuntimeApi;

    const importExportFeature = win.appFeatureImportExport.create({
      runtime: {
        state,
        ioAsset: win.ioAsset,
        ioPalette: win.ioPalette,

        buildExportPaletteFromUI: paletteBridge.buildExportPaletteFromUI,
        resetBrushColorsToBaseForEntries: paletteBridge.resetBrushColorsToBaseForEntries,
        applyImportedPaletteToBrushColors: paletteBridge.applyImportedPaletteToBrushColors,
        reconcileBrushColors: paletteBridge.reconcileBrushColors,

        getBrushColors: brushColorRuntime.getBrushColors,
        getBaseBrushColors: brushColorRuntime.getBaseBrushColors
      },

      status: {
        clearStatus: statusView.clearStatus,
        setStatus: statusView.setStatus,
        setDebug: statusView.setDebug
      },

      ui: {
        renderAllCells: uiSync.renderAllCells,
        rebuildPaletteMap: gridView.rebuildPaletteMap,
        rebuildBrushOptions: uiSync.rebuildBrushOptions,
        rebuildPaletteTable: uiSync.rebuildPaletteTable
      },

      session: {
        getLastImportedAssetFileName: importSession.getAssetFileName,
        setLastImportedAssetFileName: importSession.setAssetFileName,
        getLastImportedPaletteFileName: importSession.getPaletteFileName,
        setLastImportedPaletteFileName: importSession.setPaletteFileName,
        updateImportInfo: uiSync.updateImportInfo
      },

      browser: {
        documentRef: doc,
        urlRef: win.URL,
        blobCtor: win.Blob
      },

      controls: {
        inputImportJsonFile: refs.inputImportJsonFile
      }
    });

    const resizeFeature = win.appFeatureResize.create({
      runtime: {
        state,
        getLimits: limits.getLimits
      },

      status: {
        clearStatus: statusView.clearStatus,
        setStatus: statusView.setStatus
      },

      ui: {
        renderAllCells: uiSync.renderAllCells
      },

      controls: {
        inputWidth: refs.inputWidth,
        inputHeight: refs.inputHeight,
        selectResizePolicy: refs.selectResizePolicy
      }
    });

    const paintingFeature = win.appFeaturePainting.create({
      runtime: {
        state,
        getBrushId: paintingSessionHelper.getBrushId,

        hideCoordTip: paintingRuntimeApi.hideCoordTip,
        requestHoverTipUpdate: paintingRuntimeApi.requestHoverTipUpdate,

        setHoverRuntime: paintingRuntimeApi.setHoverRuntime,
        getHoverRuntime: paintingRuntimeApi.getHoverRuntime,

        hitIndexFromEventTarget: paintingRuntimeApi.hitIndexFromEventTarget,
        hitIndexFromPoint: paintingRuntimeApi.hitIndexFromPoint,

        setIsPointerDown: paintingRuntimeApi.setIsPointerDown,
        getIsPointerDown: paintingRuntimeApi.getIsPointerDown,

        setLastPaintIndex: paintingRuntimeApi.setLastPaintIndex,
        getLastPaintIndex: paintingRuntimeApi.getLastPaintIndex,

        getPaintRaf: paintingRuntimeApi.getPaintRaf,
        setPaintRaf: paintingRuntimeApi.setPaintRaf,

        getPendingPaint: paintingRuntimeApi.getPendingPaint
      },

      controls: {
        gridRoot: refs.gridRoot
      }
    });

    return {
      importExportFeature,
      resizeFeature,
      paintingFeature
    };
  }

  function assembleCore(win, doc, appBoot) {
    const refs = win.appUiDom.getRefs();
    const views = createViews(win, refs);
    const limits = createLimits(appBoot, refs, win.appConstAsset);
    const state = win.assetState.createState();
    const sessions = {
      importSession: win.appHelpersImportSession.create()
    };

    return {
      env: {
        windowRef: win,
        documentRef: doc
      },
      refs,
      views,
      limits,
      state,
      sessions
    };
  }

  function assemblePaletteModule(win, refs, state, views, sessions, interactionRuntime) {
    const paletteRuntime = createPaletteRuntime(win);

    // Palette rows call into the editing feature, while the feature needs palette UI
    // callbacks. This holder is the only intentional construction cycle.
    const paletteEditingFeatureRef = { current: null };

    const paletteUi = createPaletteUi(
      state,
      views,
      paletteRuntime,
      paletteEditingFeatureRef
    );
    const paletteEditingFeature = createPaletteEditingFeature(
      win,
      state,
      views,
      paletteRuntime,
      interactionRuntime,
      paletteUi
    );
    const sync = createPaletteSync(
      win,
      refs,
      state,
      views,
      sessions,
      paletteRuntime,
      paletteUi
    );
    paletteEditingFeatureRef.current = paletteEditingFeature;

    return {
      paletteRuntime,
      sync,
      paletteUi,
      paletteEditingFeature
    };
  }

  function assembleInteractionModule(win, doc, refs, state) {
    return {
      interactionRuntime: createInteractionRuntime(win, doc, refs, state)
    };
  }

  function assembleFeatures(
    win,
    doc,
    refs,
    state,
    limits,
    views,
    sessions,
    sync,
    paletteRuntime,
    interactionRuntime
  ) {
    return {
      features: createFeatures(
        win,
        doc,
        refs,
        state,
        limits,
        views,
        sessions,
        sync,
        paletteRuntime,
        interactionRuntime
      )
    };
  }

  /**
   * @param {Object} options - Context creation options.
   * @param {Window} options.windowRef - Window object used for globals and browser APIs.
   * @param {Document} options.documentRef - Document object used for DOM references.
   * @returns {Object} Runtime context grouped by environment, state, views, helpers, and features.
   */
  function create(options) {
    options = options || {};

    const win = options.windowRef || window;
    const doc = options.documentRef || win.document || document;

    const appBoot = win.appHelpersAppBoot;
    if (!appBoot) {
      throw new Error('appHelpersAppBoot missing');
    }

    const core = assembleCore(win, doc, appBoot);
    const interactionModule = assembleInteractionModule(
      win,
      doc,
      core.refs,
      core.state
    );
    const paletteModule = assemblePaletteModule(
      win,
      core.refs,
      core.state,
      core.views,
      core.sessions,
      interactionModule.interactionRuntime
    );
    const featureModule = assembleFeatures(
      win,
      doc,
      core.refs,
      core.state,
      core.limits,
      core.views,
      core.sessions,
      paletteModule.sync,
      paletteModule.paletteRuntime,
      interactionModule.interactionRuntime
    );

    return {
      // Environment and shared primitives
      env: core.env,
      refs: core.refs,
      state: core.state,
      limits: core.limits,
      views: core.views,

      // Runtime helpers shared by UI synchronization and features
      paletteRuntime: paletteModule.paletteRuntime,
      interactionRuntime: interactionModule.interactionRuntime,
      sessions: core.sessions,
      sync: paletteModule.sync,

      // User-facing behavior modules wired by app-wiring.js
      features: {
        importExportFeature: featureModule.features.importExportFeature,
        resizeFeature: featureModule.features.resizeFeature,
        paintingFeature: featureModule.features.paintingFeature,
        paletteEditingFeature: paletteModule.paletteEditingFeature
      }
    };
  }

  function getWindowRef(ctx) {
    return (ctx && ctx.env && ctx.env.windowRef) || window;
  }

  function getDocumentRef(ctx) {
    const win = getWindowRef(ctx);
    return (ctx && ctx.env && ctx.env.documentRef) || win.document || document;
  }

  /**
   * @param {Object} ctx - Runtime context returned by create().
   * @returns {void}
   */
  function wire(ctx) {
    const win = getWindowRef(ctx);
    const doc = getDocumentRef(ctx);
    const appBoot = win.appHelpersAppBoot;
    const appWiring = win.appHelpersAppWiring;
    const refs = ctx.refs;
    const features = ctx.features;

    appBoot.assertCoreRefs(refs);

    appWiring.wireApp({
      views: {
        rulerView: ctx.views.rulerView
      },

      features: {
        paintingFeature: features.paintingFeature,
        resizeFeature: features.resizeFeature,
        importExportFeature: features.importExportFeature,
        paletteEditingFeature: features.paletteEditingFeature
      },

      controls: {
        formEl: refs.formEl,
        btnClear: refs.btnClear,
        btnImportJson: refs.btnImportJson,
        btnExportJson: refs.btnExportJson,
        selectRenderBrush: refs.selectRenderBrush
      }
    });
  }

  /**
   * @param {Object} ctx - Runtime context returned by create().
   * @returns {void}
   */
  function subscribe(ctx) {
    ctx.state.subscribe((patch) => {
      ctx.sync.uiSync.handlePatch(patch);
    });
  }

  /**
   * @param {Object} ctx - Runtime context returned by create().
   * @returns {void}
   */
  function init(ctx) {
    const win = getWindowRef(ctx);
    const appInit = win.appHelpersAppInit;
    const uiSync = ctx.sync.uiSync;

    appInit.initApp({
      runtime: {
        state: ctx.state,
        paletteBridge: ctx.paletteRuntime.paletteBridge
      },
      status: {
        clearStatus: ctx.views.statusView.clearStatus,
        setStatus: ctx.views.statusView.setStatus
      },
      ui: {
        rebuildPaletteMap: ctx.views.gridView.rebuildPaletteMap,
        rebuildBrushOptions: uiSync.rebuildBrushOptions,
        rebuildPaletteTable: uiSync.rebuildPaletteTable,
        renderAllCells: uiSync.renderAllCells,
        updateImportInfo: uiSync.updateImportInfo,
        renderRulers: ctx.views.rulerView.render
      },
      config: {
        appDefaults: win.appDefaults,
        appConstAsset: win.appConstAsset
      }
    });
  }

  window.appHelpersAppContext = {
    create,
    wire,
    subscribe,
    init
  };
})();
