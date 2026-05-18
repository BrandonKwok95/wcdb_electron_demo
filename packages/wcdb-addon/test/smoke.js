"use strict";

const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const assert = require("node:assert/strict");

const { BusinessDb } = require("..");

function prepareDbPath() {
  const stableDir = path.join(os.tmpdir(), "kwok-wcdb-addon");
  const stableDbPath = path.join(stableDir, "smoke.db");

  fs.mkdirSync(stableDir, { recursive: true });
  try {
    for (const filePath of [stableDbPath, `${stableDbPath}-wal`, `${stableDbPath}-shm`]) {
      if (fs.existsSync(filePath)) {
        fs.rmSync(filePath, { force: true });
      }
    }
    return stableDbPath;
  } catch (error) {
    if (error.code !== "EBUSY" && error.code !== "EPERM") {
      throw error;
    }

    const fallbackDir = fs.mkdtempSync(path.join(os.tmpdir(), "kwok-wcdb-addon-"));
    return path.join(fallbackDir, "smoke.db");
  }
}

const dbPath = prepareDbPath();

(async () => {
  const db = new BusinessDb(dbPath);

  await db.init();

  const samples = [
    {
      title: "hello wcdb",
      content: "created from node addon"
    },
    {
      title: "electron integration",
      content: "load @kwok/wcdb-addon from Electron main process"
    },
    {
      title: "renderer query",
      content: "read business_items through a small IPC API"
    },
    {
      title: "update flow",
      content: "verify updates refresh the row content"
    },
    {
      title: "delete flow",
      content: "delete one row while keeping other smoke rows"
    },
    {
      title: "wcdb smoke data",
      content: "inspect this row from an external database viewer"
    },
    {
      title: "addon boundary",
      content: "expose business CRUD instead of raw WCDB APIs1111"
    },
    {
      title: "packaging check",
      content: "ensure WCDB.dll is available beside the native addon"
    }
  ];

  const createdItems = [];
  for (const sample of samples) {
    const item = await db.businessItem.create(sample);
    assert.ok(item.id > 0);
    createdItems.push(item);
  }

  const created = createdItems[0];
  const loaded = await db.businessItem.get(created.id);
  assert.equal(loaded.title, "hello wcdb");
  assert.equal(loaded.content, "created from node addon");

  const createdExtra = await db.extra.create({
    businessItemId: created.id,
    key: "source",
    value: "smoke"
  });
  assert.ok(createdExtra.id > 0);
  assert.equal(createdExtra.businessItemId, created.id);

  const secondExtra = await db.extra.create({
    businessItemId: created.id,
    key: "channel",
    value: "node-addon"
  });
  assert.ok(secondExtra.id > 0);

  const unrelatedExtra = await db.extra.create({
    businessItemId: createdItems[1].id,
    key: "owner",
    value: "electron-demo"
  });
  assert.ok(unrelatedExtra.id > 0);

  const loadedExtra = await db.extra.get(createdExtra.id);
  assert.equal(loadedExtra.key, "source");
  assert.equal(loadedExtra.value, "smoke");

  const extrasForFirstItem = await db.extra.listByBusinessItemId(created.id);
  assert.equal(extrasForFirstItem.length, 2);
  assert.deepEqual(
    extrasForFirstItem.map((extra) => extra.key),
    ["source", "channel"]
  );

  assert.equal(await db.extra.update(createdExtra.id, { value: "updated-smoke" }), true);
  assert.equal((await db.extra.get(createdExtra.id)).value, "updated-smoke");

  assert.equal(await db.businessItem.update(created.id, { content: "updated" }), true);
  assert.equal((await db.businessItem.get(created.id)).content, "updated");

  const items = await db.businessItem.list();
  assert.equal(items.length, samples.length);
  assert.equal(items[0].id, created.id);

  const extras = await db.extra.list();
  assert.equal(extras.length, 3);

  const removed = createdItems[createdItems.length - 1];
  assert.equal(await db.businessItem.remove(removed.id), true);
  assert.equal(await db.businessItem.get(removed.id), null);
  assert.equal((await db.businessItem.list()).length, samples.length - 1);

  assert.equal(await db.extra.remove(secondExtra.id), true);
  assert.equal(await db.extra.get(secondExtra.id), null);
  assert.equal((await db.extra.listByBusinessItemId(created.id)).length, 1);

  await db.close();

  console.log("wcdb addon smoke test passed:", dbPath);
})().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
