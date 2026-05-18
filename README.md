# WCDB Electron Demo

This monorepo contains:

- `apps/electron-demo`: a blank Electron app.
- `packages/wcdb-addon`: a business-oriented WCDB Node addon.

## Run the Electron demo

```bash
npm install
npm run dev:electron
```

## WCDB addon

See [`packages/wcdb-addon/README.md`](packages/wcdb-addon/README.md) for the current JS API and data model.

```bash
npm run build:wcdb-addon
npm run test:wcdb-addon
```
