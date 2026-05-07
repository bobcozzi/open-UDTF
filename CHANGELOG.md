# Changelog

All notable changes to this project will be documented in this file.

## [1.4.0] - 2026-05-07

### Added
- **New UDTF: FILE_FIELDS** — `src/FILE_FIELDS/FIELDLIST.CPP`, `FIELDLIST.SQL`,
  `FIELDLIST.MD`. Returns field metadata (name, type, length, etc.) for any
  database file on the system.
- `src/shared/h/OBJLIST.H` and `src/shared/QCSRC/objlist.cpp` — shared module
  source bound into every UDTF program via `CRTPGM MODULE(OBJLIB/OBJLIST)`.
- `BLDUDTF` command: new `BLDDEP` parameter controls dependent-module build
  behavior:
  - `*BIND` *(default)* — checks if `OBJLIB/OBJLIST *MODULE` already exists
    (`CHKOBJ`); skips compile if present, builds only if missing.
  - `*REFRESH` — always recompiles `OBJLIB/OBJLIST` before linking; use after
    modifying shared headers such as `COZUTILS.H` or `OBJLIST.H`.
- `BLDUDTF.CLLE`: `CRTPGM` now binds `MODULE(&OBJLIB/OBJLIST)` with
  `ENTMOD(&OBJLIB/&UDTF)` so the shared module is always included. Added
  `ADDLIBLE` for the source library before compilation.
- `src/shared/h/COZUTILS.H`: `coz_TEMP_USRSPACE` now calls `QUSCUSAT` with
  key=3 (`AutoExtend='1'`) immediately after `QUSCRTUS`, preventing MCH0601
  user-space overflow at runtime.
- `src/shared/h/COZUTILS.H`: added `coz_rmvmsg()` free function and
  `coz::RMVMSG()` wrapper — removes the most-recent job-log message (e.g.
  CPC2206 owner-change completion message) using `QMHRCVPM`/`QMHRMVPM`.

### Changed
- `BLDUDTF.MD`: fully documented `BLDDEP` parameter (table, step-by-step
  flow, examples, and Related Objects updated to include `OBJLIST *MODULE`).
- `BLDUDTF.CLLE`: `CRTCPPMOD` options aligned between dep-module and main-
  module builds (`*SHOWINC` added, `INLINE`/`OPTIMIZE` now driven by
  `&DBGVIEW`).

## [1.3.0] - 2026-05-06
- Corrected initial naming consistency and folder structures.

## [1.2.0] - 2026-05-06
-`/src/h/cozutils.h` replaces the old include file name. This allows it to be more properly named as we add more UDTFs to this repo and use that same include file.
- The `BLDUDTF` command and its CLLE command processing program were updated and completed. They now support a full build process for the SPOOL_DATA UDTF and future functions we add to this repo.
- Note: Currently the BLDUDTF allows you to specify the Object Library (OBJLIB), however the SQL UDTF source `SPOOLDATA.SQL` that contains the initial `SPOOLED_DATA` function currently hard codes its schema/library as `SQLTOOLS`. This will change in the future. But for now, if you don't want to use `SQLTOOLS` as the library/schema name, then you will need to manually change `SQLTOOLS` in the `SPOOLDATA.SQL` source to your preferred schema/library name.

## [1.1.0] - 2026-05-05

### Added
- `src/build/BLDUDTF.CLLE` — Generic IBM i CL build program for compiling and
  registering any UDTF in this project.
  - Accepts three optional parameters: `&OBJLIB` (target library),
    `&EXTPGM` (external C++ program/module name), and `&UDTFMBR` (SQL DDL
    source member). All default to the SPOOLED_DATA UDTF values when omitted.
  - Step 1: `CRTCPPMOD` — compiles the C++ source member into a module.
  - Step 2: `CRTPGM` — binds the module into a `*PGM` object.
  - Step 3: `RUNSQLSTM` — registers (or replaces) the UDTF via the SQL DDL
    source member.
  - Designed to be reused as-is for future UDTFs by passing different
    parameter values at call time.

## [1.0.0] - 2026-05-05

### Added
- Initial release of the **SPOOLED_DATA** standalone UDTF.
- `src/spooled_data/c/SPOOLDATA.CPP` — C++ source for the UDTF external
  program `SPOOLDATA`.
- `src/spooled_data/h/SPLUTILS.H` — Standalone spool-read utility helpers
  (replaces `coz::` namespace; all functions are `static inline` with `spu_`
  prefix).
- `src/spooled_data/sql/SPOOLDATA.SQL` — SQL DDL to create or replace
  `SQLTOOLS.SPOOLED_DATA` table function (specific name `SQLTOOLS.SPOOLED_DATA`).
