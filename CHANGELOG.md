# Changelog

All notable changes to this project will be documented in this file.

## [1.7.1] - 2026-07-12
### Fixed
- Fixed an issue with the CObjList class in the OBJLIST.CPP file when the object type was not prefixed with an asterisk `*` it would cause it to fail.

## [1.7.0] - 2026-06-02
### What's New
-- Add New **HASH_SHA256** UDTF that returns the SHA256 HASH but works on IBM i V7R2 and later. A variation of the function named HASH_SHA256_UTF8 is also included that converts the input data to UTF-8 (CCSID:1208) before calculating the HASH.

## [1.6.0] - 2026-05-28
### What's New
-- Add New **CHECK_AUT** UDTF that returns 1=Authorized, 0=Not Authorized  or -1=Object Not found after checking user-specified authority to an object.

## [1.7.0] - 2026-06-01
### What's New
-- Add New **CMD_CHECK** UDTF that syntax checks CL Command in various environment such as CL programming. The returned resultset is a list of error message ID and message text, or and empty set (no row) when no syntax error is detected. This is the same function used by vscode-clle to syntax check CL program source.

## [1.5.0] - 2026-05-21

### What's New
- **New UDTF: CMD_HELP** — `src/CMD_HELP/CMDHELP.CPP`, `cmdhelp.sql`,
  `CMDHELP.MD`. Returns the HTML helptext for any CL command on the system
  using `GENCMDDOC` under the covers.
- **New UDTF: CMD_XML** — `src/CMD_XML/CMDXML.CPP`, `CMDXML.SQL`,
  `CMDXML.MD`. Returns the raw XML command definition for any CL command
  on the system by calling the `QCDRCMDD` API.
- `@author BobCozzi` tag added to all `.cpp` and `.sql` source files across
  CMD_HELP, CMD_XML, FILE_FIELDS, and SPOOLED_DATA folders.

## [1.4.0] - 2026-05-07

### What's New
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
- `/src/shared/h/COZUTILS.H` replaces the old include file name. This allows it to be more properly named as we add more UDTFs to this repo and use that same include file.
- The `BLDUDTF` command and its CLLE command processing program were updated and completed. They now support a full build process for the SPOOL_DATA UDTF and future functions we add to this repo.
- Note: Currently the BLDUDTF allows you to specify the Object Library (OBJLIB), however the SQL UDTF source `SPOOLDATA.SQL` that contains the initial `SPOOLED_DATA` function currently hard codes its schema/library as `SQLTOOLS`. This will change in the future. But for now, if you don't want to use `SQLTOOLS` as the library/schema name, then you will need to manually change `SQLTOOLS` in the `SPOOLDATA.SQL` source to your preferred schema/library name.

## [1.1.0] - 2026-05-05

### What's New
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

### What's New
- Initial release of the **SPOOLED_DATA** standalone UDTF.
- `src/SPOOLED_DATA/SPOOLDATA.CPP` — C++ source for the UDTF external
  program `SPOOLDATA`.
- `src/shared/h/COZUTILS.H` — Shared standalone utility helpers used by
  SPOOLED_DATA and other UDTFs.
- `src/SPOOLED_DATA/SPOOLDATA.SQL` — SQL DDL to create or replace
  `SQLTOOLS.SPOOLED_DATA` table function (specific name `SQLTOOLS.SPOOLED_DATA`).
