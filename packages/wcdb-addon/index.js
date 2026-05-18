"use strict";

const path = require("node:path");

const wcdbRuntimeDir = path.join(__dirname, "build", "Release");
process.env.PATH = `${wcdbRuntimeDir}${path.delimiter}${process.env.PATH || ""}`;

const { NativeBusinessDb } = require("./build/Release/kwok_wcdb_addon.node");

class BusinessItemApi {
  constructor(nativeDb) {
    this.nativeDb = nativeDb;
  }

  create(input) {
    return this.nativeDb.businessItemCreate(input);
  }

  get(id) {
    return this.nativeDb.businessItemGet(id);
  }

  list() {
    return this.nativeDb.businessItemList();
  }

  update(id, patch) {
    return this.nativeDb.businessItemUpdate(id, patch);
  }

  remove(id) {
    return this.nativeDb.businessItemRemove(id);
  }
}

class ExtraApi {
  constructor(nativeDb) {
    this.nativeDb = nativeDb;
  }

  create(input) {
    return this.nativeDb.extraCreate(input);
  }

  get(id) {
    return this.nativeDb.extraGet(id);
  }

  list() {
    return this.nativeDb.extraList();
  }

  listByBusinessItemId(businessItemId) {
    return this.nativeDb.extraListByBusinessItemId(businessItemId);
  }

  update(id, patch) {
    return this.nativeDb.extraUpdate(id, patch);
  }

  remove(id) {
    return this.nativeDb.extraRemove(id);
  }
}

class BusinessDb {
  constructor(dbPath) {
    this.nativeDb = new NativeBusinessDb(dbPath);
    this.businessItem = new BusinessItemApi(this.nativeDb);
    this.extra = new ExtraApi(this.nativeDb);
  }

  init() {
    return this.nativeDb.init();
  }

  close() {
    return this.nativeDb.close();
  }
}

module.exports = {
  BusinessDb
};
