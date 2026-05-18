# @kwok/wcdb-addon

`@kwok/wcdb-addon` 是一个业务型 WCDB Node addon。它不直接导出完整 WCDB API，而是把当前项目需要的数据库能力封装成稳定的 JS 接口。

当前 addon 暴露一个 `BusinessDb`，同一个实例内部持有同一个 WCDB database，并按业务域分组：

- `db.businessItem.xxx`：操作 `business_items` 表。
- `db.extra.xxx`：操作 `extras` 表，和 `business_items` 在同一个数据库文件里。

## 构建与测试

在仓库根目录执行：

```bash
npm run build:wcdb-addon
npm run test:wcdb-addon
```

addon 链接仓库内置的 WCDB 平台产物：

- Windows x64: `packages/wcdb-addon/vendor/wcdb/windows-x64`
- macOS arm64: `packages/wcdb-addon/vendor/wcdb/darwin-arm64`

构建后会把对应平台的运行时库复制到 addon 的 `build/Release` 目录。

## 使用示例

当前接口都是异步接口，返回 `Promise`：

```js
const { BusinessDb } = require("@kwok/wcdb-addon");

async function main() {
  const db = new BusinessDb("D:\\repo\\wcdb_electron_demo\\demo.db");

  await db.init();

  const item = await db.businessItem.create({
    title: "hello wcdb",
    content: "created from node addon"
  });

  const extra = await db.extra.create({
    businessItemId: item.id,
    key: "source",
    value: "electron"
  });

  console.log(await db.businessItem.get(item.id));
  console.log(await db.extra.listByBusinessItemId(item.id));

  await db.extra.update(extra.id, { value: "updated" });
  await db.businessItem.update(item.id, { content: "updated" });

  await db.extra.remove(extra.id);
  await db.businessItem.remove(item.id);
  await db.close();
}

main().catch(console.error);
```

## 数据模型

### BusinessItemInput

传给 `db.businessItem.create()` 的输入对象：

```ts
interface BusinessItemInput {
  title: string;
  content?: string;
}
```

### BusinessItem

addon 返回的业务对象：

```ts
interface BusinessItem {
  id: number;
  title: string;
  content: string;
  updatedAt: number;
}
```

### ExtraInput

传给 `db.extra.create()` 的输入对象：

```ts
interface ExtraInput {
  businessItemId: number;
  key: string;
  value?: string;
}
```

### Extra

addon 返回的扩展对象：

```ts
interface Extra {
  id: number;
  businessItemId: number;
  key: string;
  value: string;
  updatedAt: number;
}
```

字段说明：

- `id`: 自增主键。
- `businessItemId`: 关联的 `business_items.id`。
- `key`: 扩展字段名。
- `value`: 扩展字段值，未传时为空字符串。
- `updatedAt`: Unix 毫秒时间戳。

## 接口说明

### `new BusinessDb(dbPath)`

创建数据库实例。构造函数只创建 native database 对象，实际打开数据库和建表由 `init()` 完成。

参数：

- `dbPath: string`: SQLite/WCDB 数据库文件路径。

### `db.init()`

打开数据库，并创建当前业务需要的表：`business_items` 和 `extras`。

返回：

- `Promise<boolean>`: 初始化成功时 resolve `true`。

### `db.businessItem.create(input)`

插入一条业务记录。

返回：

- `Promise<BusinessItem>`: 插入后的完整对象，包含自增 `id`。

### `db.businessItem.get(id)`

按主键查询一条业务记录。

返回：

- `Promise<BusinessItem | null>`。

### `db.businessItem.list()`

查询 `business_items` 表内全部业务记录。

返回：

- `Promise<BusinessItem[]>`。

### `db.businessItem.update(id, patch)`

按主键更新业务记录。`patch` 支持 `title` 和 `content`。

返回：

- `Promise<boolean>`: 找到记录并更新成功时为 `true`，记录不存在时为 `false`。

### `db.businessItem.remove(id)`

按主键删除业务记录。

返回：

- `Promise<boolean>`。

### `db.extra.create(input)`

插入一条扩展记录。

返回：

- `Promise<Extra>`: 插入后的完整对象，包含自增 `id`。

### `db.extra.get(id)`

按主键查询一条扩展记录。

返回：

- `Promise<Extra | null>`。

### `db.extra.list()`

查询 `extras` 表内全部扩展记录。

返回：

- `Promise<Extra[]>`。

### `db.extra.listByBusinessItemId(businessItemId)`

查询某条业务记录下面的扩展记录。

返回：

- `Promise<Extra[]>`。

### `db.extra.update(id, patch)`

按主键更新扩展记录。`patch` 支持 `key` 和 `value`。

返回：

- `Promise<boolean>`: 找到记录并更新成功时为 `true`，记录不存在时为 `false`。

### `db.extra.remove(id)`

按主键删除扩展记录。

返回：

- `Promise<boolean>`。

### `db.close()`

关闭当前 `BusinessDb` 实例，后续继续调用其它接口会 reject `database is closed`。

返回：

- `Promise<void>`。

关闭后继续调用其它接口会 reject `database is closed`。

## 代码分层

- `src/addon`: N-API 接入层，负责 JS 参数解析、Promise/AsyncWorker、对象转换和方法导出。
- `src/orm`: WCDB ORM model 层，当前包含 `BusinessItem` 和 `Extra`。
- `src/services`: 实际处理数据库的业务层，当前包含 `BusinessItemService` 和 `ExtraService`。

横向新增表时，按同样方式增加一组 `orm + service + addon method + JS wrapper + d.ts`，并挂到 `BusinessDb` 的新 namespace 上，例如 `db.someTable.xxx`。

## 当前边界

- 这是业务型封装，不提供通用 SQL 执行器，也不导出 WCDB 原始 API。
- 当前所有 JS 接口基于 N-API `AsyncWorker`，数据库操作在线程池执行，不同步阻塞 JS 调用线程。
- 每个 worker 会基于同一个 `dbPath` 创建短生命周期的 `WCDB::Database` 句柄，避免多个异步 worker 共享同一个 native database 对象。
- native 侧只用轻量 mutex 保护 `closed` / `initialized` 这类生命周期状态，不再把每个 CRUD 操作串行化。
- 当前内置的是 Windows x64 和 macOS arm64 版本的 WCDB 产物；后续如果要支持 Linux 或其他架构，可以在 `vendor/wcdb` 下继续按平台增加目录。
