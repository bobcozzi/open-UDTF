# Changelog

All notable changes to this project will be documented in this file.

## [1.3.0] - 2026-05-06
- Corrected initial naming consistency and folder structures

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
