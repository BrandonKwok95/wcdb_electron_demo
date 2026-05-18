const path = require("node:path");
const { app, BrowserWindow, ipcMain } = require("electron");
const { BusinessDb } = require("@kwok/wcdb-addon");

let db;

function getDbPath() {
  return path.join(app.getPath("userData"), "wcdb-demo.db");
}

async function getDb() {
  if (!db) {
    db = new BusinessDb(getDbPath());
    await db.init();
  }

  return db;
}

function registerWcdbHandlers() {
  ipcMain.handle("wcdb:getInfo", async () => ({
    dbPath: getDbPath(),
    platform: process.platform
  }));

  ipcMain.handle("wcdb:init", async () => {
    await getDb();
    return { dbPath: getDbPath() };
  });

  ipcMain.handle("wcdb:business:create", async (_event, input) => {
    const businessDb = await getDb();
    return businessDb.businessItem.create(input);
  });

  ipcMain.handle("wcdb:business:get", async (_event, id) => {
    const businessDb = await getDb();
    return businessDb.businessItem.get(id);
  });

  ipcMain.handle("wcdb:business:list", async () => {
    const businessDb = await getDb();
    return businessDb.businessItem.list();
  });

  ipcMain.handle("wcdb:business:update", async (_event, id, patch) => {
    const businessDb = await getDb();
    return businessDb.businessItem.update(id, patch);
  });

  ipcMain.handle("wcdb:business:remove", async (_event, id) => {
    const businessDb = await getDb();
    return businessDb.businessItem.remove(id);
  });

  ipcMain.handle("wcdb:extra:create", async (_event, input) => {
    const businessDb = await getDb();
    return businessDb.extra.create(input);
  });

  ipcMain.handle("wcdb:extra:get", async (_event, id) => {
    const businessDb = await getDb();
    return businessDb.extra.get(id);
  });

  ipcMain.handle("wcdb:extra:list", async () => {
    const businessDb = await getDb();
    return businessDb.extra.list();
  });

  ipcMain.handle("wcdb:extra:listByBusinessItemId", async (_event, businessItemId) => {
    const businessDb = await getDb();
    return businessDb.extra.listByBusinessItemId(businessItemId);
  });

  ipcMain.handle("wcdb:extra:update", async (_event, id, patch) => {
    const businessDb = await getDb();
    return businessDb.extra.update(id, patch);
  });

  ipcMain.handle("wcdb:extra:remove", async (_event, id) => {
    const businessDb = await getDb();
    return businessDb.extra.remove(id);
  });
}

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 960,
    height: 640,
    minWidth: 720,
    minHeight: 480,
    show: false,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  mainWindow.once("ready-to-show", () => {
    mainWindow.show();
  });

  mainWindow.loadFile(path.join(__dirname, "renderer", "index.html"));
}

async function runSmoke() {
  const businessDb = await getDb();
  const item = await businessDb.businessItem.create({
    title: "electron smoke",
    content: "created from Electron main process"
  });
  const extra = await businessDb.extra.create({
    businessItemId: item.id,
    key: "runtime",
    value: "electron"
  });
  const loaded = await businessDb.businessItem.get(item.id);
  const extras = await businessDb.extra.listByBusinessItemId(item.id);

  if (!loaded || extras.length !== 1 || extras[0].id !== extra.id) {
    throw new Error("Electron WCDB smoke validation failed");
  }

  console.log("electron demo smoke passed:", getDbPath());
  const closingDb = db;
  db = null;
  await closingDb.close();
  app.quit();
}

app.whenReady().then(async () => {
  registerWcdbHandlers();

  if (process.env.WCDB_DEMO_SMOKE === "1") {
    await runSmoke();
    return;
  }

  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});

app.on("before-quit", async (event) => {
  if (!db) {
    return;
  }

  event.preventDefault();
  const closingDb = db;
  db = null;
  await closingDb.close();
  app.quit();
});
