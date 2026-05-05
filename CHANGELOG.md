# Changelog

All notable changes to this project will be documented in this file.

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
  program `ST_SPOOLDATA`.
- `src/spooled_data/h/SPLUTILS.H` — Standalone spool-read utility helpers
  (replaces `coz::` namespace; all functions are `static inline` with `spu_`
  prefix).
- `src/spooled_data/sql/SPOOLDATA.SQL` — SQL DDL to create or replace
  `SQLTOOLS.SPOOLED_DATA` table function (specific name `ST_SPOOLDATA`).
