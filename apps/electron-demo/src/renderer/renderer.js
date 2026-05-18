"use strict";

const platformElement = document.querySelector("#platform");
const dbPathElement = document.querySelector("#db-path");
const statusElement = document.querySelector("#status");
const resultElement = document.querySelector("#result");
const businessListElement = document.querySelector("#business-list");
const businessCountElement = document.querySelector("#business-count");
const extraListElement = document.querySelector("#extra-list");
const extraCountElement = document.querySelector("#extra-count");

let businessItems = [];
let extras = [];
let selectedBusinessId = null;
let selectedExtraId = null;
let sequence = 1;

function setStatus(text) {
  statusElement.textContent = text;
}

function showResult(value) {
  resultElement.textContent = JSON.stringify(value, null, 2);
}

function rowButton(label, onClick) {
  const button = document.createElement("button");
  button.type = "button";
  button.className = "row-button";
  button.textContent = label;
  button.addEventListener("click", onClick);
  return button;
}

function renderBusinessItems() {
  businessCountElement.textContent = `${businessItems.length} rows`;
  businessListElement.replaceChildren();

  if (businessItems.length === 0) {
    businessListElement.append(emptyState("暂无 business_items 数据"));
    return;
  }

  for (const item of businessItems) {
    const row = document.createElement("article");
    row.className = item.id === selectedBusinessId ? "record selected" : "record";

    const content = document.createElement("div");
    content.innerHTML = `
      <strong>#${item.id} ${escapeHtml(item.title)}</strong>
      <span>${escapeHtml(item.content)}</span>
    `;

    row.append(
      content,
      rowButton("选中", () => {
        selectedBusinessId = item.id;
        renderBusinessItems();
      })
    );
    businessListElement.append(row);
  }
}

function renderExtras() {
  extraCountElement.textContent = `${extras.length} rows`;
  extraListElement.replaceChildren();

  if (extras.length === 0) {
    extraListElement.append(emptyState("暂无 extras 数据"));
    return;
  }

  for (const item of extras) {
    const row = document.createElement("article");
    row.className = item.id === selectedExtraId ? "record selected" : "record";

    const content = document.createElement("div");
    content.innerHTML = `
      <strong>#${item.id} ${escapeHtml(item.key)}</strong>
      <span>businessItemId: ${item.businessItemId} | ${escapeHtml(item.value)}</span>
    `;

    row.append(
      content,
      rowButton("选中", () => {
        selectedExtraId = item.id;
        renderExtras();
      })
    );
    extraListElement.append(row);
  }
}

function emptyState(text) {
  const node = document.createElement("p");
  node.className = "empty";
  node.textContent = text;
  return node;
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

async function refreshAll() {
  businessItems = await window.wcdbDemo.businessItem.list();
  extras = await window.wcdbDemo.extra.list();

  if (!selectedBusinessId && businessItems.length > 0) {
    selectedBusinessId = businessItems[0].id;
  }

  if (!businessItems.some((item) => item.id === selectedBusinessId)) {
    selectedBusinessId = businessItems[0]?.id ?? null;
  }

  if (!extras.some((item) => item.id === selectedExtraId)) {
    selectedExtraId = extras[0]?.id ?? null;
  }

  renderBusinessItems();
  renderExtras();
}

async function runAction(action) {
  setStatus("running");

  try {
    const result = await actions[action]();
    await refreshAll();
    showResult(result);
    setStatus("done");
  } catch (error) {
    showResult({
      name: error.name,
      message: error.message
    });
    setStatus("error");
  }
}

const actions = {
  async init() {
    return window.wcdbDemo.init();
  },

  async refresh() {
    await refreshAll();
    return { businessItems: businessItems.length, extras: extras.length };
  },

  async "business-create"() {
    const item = await window.wcdbDemo.businessItem.create({
      title: `业务记录 ${sequence}`,
      content: `created from Electron renderer button ${sequence}`
    });
    selectedBusinessId = item.id;
    sequence += 1;
    return item;
  },

  async "business-list"() {
    return window.wcdbDemo.businessItem.list();
  },

  async "business-get"() {
    assertSelectedBusiness();
    return window.wcdbDemo.businessItem.get(selectedBusinessId);
  },

  async "business-update"() {
    assertSelectedBusiness();
    return window.wcdbDemo.businessItem.update(selectedBusinessId, {
      content: `updated at ${new Date().toLocaleTimeString()}`
    });
  },

  async "business-remove"() {
    const item = businessItems.at(-1);
    if (!item) {
      throw new Error("没有可删除的 business item");
    }
    return window.wcdbDemo.businessItem.remove(item.id);
  },

  async "extra-create"() {
    assertSelectedBusiness();
    const item = await window.wcdbDemo.extra.create({
      businessItemId: selectedBusinessId,
      key: `extra-${sequence}`,
      value: `value from renderer ${sequence}`
    });
    selectedExtraId = item.id;
    sequence += 1;
    return item;
  },

  async "extra-list"() {
    return window.wcdbDemo.extra.list();
  },

  async "extra-get"() {
    assertSelectedExtra();
    return window.wcdbDemo.extra.get(selectedExtraId);
  },

  async "extra-update"() {
    assertSelectedExtra();
    return window.wcdbDemo.extra.update(selectedExtraId, {
      value: `extra updated at ${new Date().toLocaleTimeString()}`
    });
  },

  async "extra-remove"() {
    const item = extras.at(-1);
    if (!item) {
      throw new Error("没有可删除的 extra");
    }
    return window.wcdbDemo.extra.remove(item.id);
  }
};

function assertSelectedBusiness() {
  if (!selectedBusinessId) {
    throw new Error("请先新增或选中一条 business item");
  }
}

function assertSelectedExtra() {
  if (!selectedExtraId) {
    throw new Error("请先新增或选中一条 extra");
  }
}

async function bootstrap() {
  if (platformElement && window.electronDemo) {
    platformElement.textContent = window.electronDemo.platform;
  }

  const info = await window.wcdbDemo.getInfo();
  dbPathElement.textContent = info.dbPath;

  await window.wcdbDemo.init();
  await refreshAll();
  showResult(info);
}

document.addEventListener("click", (event) => {
  const button = event.target.closest("[data-action]");
  if (!button) {
    return;
  }

  runAction(button.dataset.action);
});

bootstrap().catch((error) => {
  showResult({
    name: error.name,
    message: error.message
  });
  setStatus("error");
});
