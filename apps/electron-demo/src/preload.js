const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electronDemo", {
  platform: process.platform
});

contextBridge.exposeInMainWorld("wcdbDemo", {
  getInfo: () => ipcRenderer.invoke("wcdb:getInfo"),
  init: () => ipcRenderer.invoke("wcdb:init"),
  businessItem: {
    create: (input) => ipcRenderer.invoke("wcdb:business:create", input),
    get: (id) => ipcRenderer.invoke("wcdb:business:get", id),
    list: () => ipcRenderer.invoke("wcdb:business:list"),
    update: (id, patch) => ipcRenderer.invoke("wcdb:business:update", id, patch),
    remove: (id) => ipcRenderer.invoke("wcdb:business:remove", id)
  },
  extra: {
    create: (input) => ipcRenderer.invoke("wcdb:extra:create", input),
    get: (id) => ipcRenderer.invoke("wcdb:extra:get", id),
    list: () => ipcRenderer.invoke("wcdb:extra:list"),
    listByBusinessItemId: (businessItemId) =>
      ipcRenderer.invoke("wcdb:extra:listByBusinessItemId", businessItemId),
    update: (id, patch) => ipcRenderer.invoke("wcdb:extra:update", id, patch),
    remove: (id) => ipcRenderer.invoke("wcdb:extra:remove", id)
  }
});
