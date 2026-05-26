// app.js
(function () {
  'use strict';

  // Validate required globals before building the app context.
  if (!window.appHelpersAppBoot) {
    throw new Error('Required global missing: appHelpersAppBoot');
  }
  if (!window.appHelpersAppInit) {
    throw new Error('Required global missing: appHelpersAppInit');
  }
  if (!window.appHelpersGridHover) {
    throw new Error('Required global missing: appHelpersGridHover');
  }
  if (!window.appHelpersCoordTip) {
    throw new Error('Required global missing: appHelpersCoordTip');
  }
  if (!window.appHelpersPaintingSession) {
    throw new Error('Required global missing: appHelpersPaintingSession');
  }
  if (!window.appHelpersAppWiring) {
    throw new Error('Required global missing: appHelpersAppWiring');
  }
  if (!window.appHelpersAppStateSync) {
    throw new Error('Required global missing: appHelpersAppStateSync');
  }
  if (!window.appHelpersAppContext) {
    throw new Error('Required global missing: appHelpersAppContext');
  }

  const appBoot = window.appHelpersAppBoot;
  const appContext = window.appHelpersAppContext;
  appBoot.assertBootGlobals(window);

  // The context object centralizes references, features, and lifecycle hooks.
  const ctx = appContext.create({
    windowRef: window,
    documentRef: document
  });

  // Keep the boot sequence explicit: wire dependencies, subscribe to state, then initialize UI.
  appContext.wire(ctx);
  appContext.subscribe(ctx);
  appContext.init(ctx);

})();
