# BLDUDTF — Build SQL UDTF Command

`BLDUDTF` (Build UDTF) is an IBM i CL command that compiles and registers a SQL
User-Defined Table Function (UDTF) backed by an external C or C++ program.
It handles the full build pipeline in a single command:

1. Optionally drops the existing SQL function.
2. Compiles the external C/C++ source into a module.
3. Links the module into a program object.
4. Runs the SQL DDL source to register the function in the SQL catalog.

The Command Processing Program (CPP) **BLDUDTF.CLLE** carries out all four steps.

---

## Syntax

```
BLDUDTF  UDTF(member-name)
         EXTPGM(program-name)
         EXTPGMSRC(source-file / source-library)
         EXTPGMMBR(*EXTPGM | member-name)
         UDTFSRC(source-file / source-library)
         OBJLIB(library-name)
         DBGVIEW(*SOURCE | *NONE | *STMT | *LIST)
         DROP(*NO | *DROP | *YES)
         SPECNAME(*UDTF | specific-name)
```

---

## Parameters

### UDTF — UDTF Source Member *(Required)*

| Attribute | Value |
|-----------|-------|
| Type | `*NAME` (up to 10 characters) |
| Required | Yes |

The name of the UDTF source member that contains the `CREATE FUNCTION` SQL
statement (typically stored in `QUDFSRC` or `QSQLSRC`).
This name is also used as the compiled program object name written to `OBJLIB`.

---

### EXTPGM — External Program Name *(Required)*

| Attribute | Value |
|-----------|-------|
| Type | `*CHAR` (up to 10 characters) |
| Required | Yes |

The name of the external program object that SQL will call at run time.
This must match the name coded in the `EXTERNAL NAME` clause of the
`CREATE FUNCTION` statement.

---

### EXTPGMSRC — External Program Source File

| Attribute | Value |
|-----------|-------|
| Type | Qualified file name (`file / library`) |
| Default file | `QCSRC` |
| Default library | `SQLTOOLS` |

The qualified source file that contains the C or C++ source member for the
external program.  Specify as `source-file/source-library`, for example:

```
EXTPGMSRC(QCSRC/MYLIB)
```

The command uses `RTVMBRD` against this file to determine the source member's
actual source type (`CPP` vs. `C`) so it can choose the correct compiler.

---

### EXTPGMMBR — External Program Source Member

| Attribute | Value |
|-----------|-------|
| Type | `*NAME` (up to 10 characters) |
| Default | `*EXTPGM` |

The name of the source member within `EXTPGMSRC` that holds the C/C++ code.
The special value **`*EXTPGM`** (default) tells the command to use the same
name supplied on the `EXTPGM` parameter.

---

### UDTFSRC — UDTF SQL Source File

| Attribute | Value |
|-----------|-------|
| Type | Qualified file name (`file / library`) |
| Default file | `QUDFSRC` (also accepts `QSQLSRC`) |
| Default library | `SQLTOOLS` |

The qualified source file that contains the `CREATE FUNCTION` SQL statement
for the UDTF.  The CPP runs `RUNSQLSTM` against this file (member `UDTF`) as
the final build step, creating or replacing the SQL function registration in
the catalog.  `OBJLIB` is passed as the default RDB collection (`DFTRDBCOL`)
so unqualified object references in the DDL resolve correctly.

---

### OBJLIB — Target Library

| Attribute | Value |
|-----------|-------|
| Type | `*CHAR` (up to 10 characters) |
| Default | `SQLTOOLS` |

The library where the compiled module and program objects are placed.
`CRTCPPMOD`, `CRTPGM`, and `RUNSQLSTM` (`DFTRDBCOL`) all target this library.

---

### DBGVIEW — Debug View

| Attribute | Value |
|-----------|-------|
| Type | Restricted value list |
| Default | `*SOURCE` |
| Values | `*NONE`, `*STMT`, `*SOURCE`, `*LIST` |

Controls the level of debug information compiled into the module.

| Value | Behavior |
|-------|----------|
| `*NONE` | No debug data. Compiler uses full optimization (`OPTIMIZE(30)`) and inlining (`INLINE(*ON)`). |
| `*SOURCE` | Source-level debug view. Optimization is reduced to level `10` to keep debug symbols accurate. |
| `*STMT` | Statement-level debug data. Same optimization reduction as `*SOURCE`. |
| `*LIST` | Compiler listing debug data. Same optimization reduction. |

> **Tip:** Use `*NONE` for production builds and `*SOURCE` or `*STMT` during
> development and troubleshooting.

---

### DROP — DROP FUNCTION Before Create

| Attribute | Value |
|-----------|-------|
| Type | `*LGL` (logical) |
| Default | `*NO` |
| Values | `*DROP`, `*YES`, `*NO` |

Controls whether the existing SQL function is dropped before the new program
is built.

| Value | Behavior |
|-------|----------|
| `*DROP` / `*YES` | Executes `DROP FUNCTION objlib.specific-name` before compiling. |
| `*NO` | Skips the `DROP`; the existing SQL function registration is left intact. |

> **Note:** Dropping the function removes its SQL registration but does **not**
> delete the underlying `*PGM` object.  Use `*NO` when you only want to
> recompile the program without altering the SQL catalog.

---

### SPECNAME — Specific Name for DROP

| Attribute | Value |
|-----------|-------|
| Type | `*PNAME` (up to 128 characters) |
| Default | `*UDTF` |
| Shown when | `DROP(*DROP)` or `DROP(*YES)` |

The specific name used in the `DROP FUNCTION` statement.
The special value **`*UDTF`** (default) substitutes the value supplied on the
`UDTF` parameter, which is the most common case.

Supply an explicit name only when the SQL-registered specific name differs
from the UDTF source member name — for example:

```
SPECNAME(SPOOL_DATA)
```

---

## What the Command Does — Step by Step

```
BLDUDTF
  │
  ├─1─ Adjust compiler settings
  │      If DBGVIEW ≠ *NONE → OPTIMIZE(10)  [INLINE always *ON]
  │      Else               → OPTIMIZE(30)  [INLINE always *ON]
  │
  ├─2─ Resolve member name
  │      If EXTPGMMBR = *EXTPGM → use EXTPGM value as member name
  │
  ├─3─ RTVMBRD — determine source type
  │      Reads EXTPGMSRC/EXTPGMMBR to get:
  │        • Actual source library (EXTPGMLIB)
  │        • Source type (CPP or C)
  │
  ├─4─ DROP FUNCTION (when DROP = *YES / *DROP)
  │      Resolves SPECNAME (*UDTF → UDTF value)
  │      Runs:  DROP FUNCTION objlib.specname
  │      (errors from DROP are locally monitored and ignored)
  │
  ├─5─ Compile module (when SRCTYPE = CPP)
  │      CRTCPPMOD MODULE(OBJLIB/EXTPGM)
  │                SRCFILE(EXTPGMLIB/file)
  │                SRCMBR(EXTPGMMBR)
  │                LANGLVL(*EXTENDED0X)
  │                TERASPACE(*YES *TSIFC)
  │                STGMDL(*INHERIT)
  │                DTAMDL(*P128)
  │                PACKSTRUCT(16)
  │                DBGVIEW(DBGVIEW)
  │                ...
  │
  ├─6─ Create program
  │      CRTPGM PGM(OBJLIB/UDTF)
  │             ACTGRP(*CALLER)
  │             USRPRF(*USER)
  │             STGMDL(*INHERIT)
  │             REPLACE(*YES)
  │
  └─7─ Register SQL function
         RUNSQLSTM SRCFILE(UDTFSRC-lib/UDTFSRC-file)
                   SRCMBR(UDTF)
                   COMMIT(*NONE)
                   NAMING(*SYS)
                   ERRLVL(30)
                   DATFMT(*ISO)
                   DFTRDBCOL(OBJLIB)
                   DBGVIEW(DBGVIEW)
```

---

## Compiler Options Applied

The following fixed compiler options are always used when compiling C++ source:

| Option | Value | Purpose |
|--------|-------|---------|
| `LANGLVL` | `*EXTENDED0X` | Enables C++0x/C++11 language extensions |
| `TERASPACE` | `*YES *TSIFC` | Enables teraspace storage with teraspace IFS I/O |
| `STGMDL` | `*INHERIT` | Inherits storage model from activation group |
| `DTAMDL` | `*P128` | 128-bit pointer data model |
| `PACKSTRUCT` | `16` | Structure packing alignment of 16 bytes |
| `SYSIFCOPT` | `*IFS64IO` | 64-bit IFS I/O interfaces |
| `WEAKTMPL` | `*YES` | Allows weak template instantiations |
| `DEFINE` | `__MULTI__ _MULTI_THREADED` | Marks code as multi-thread safe |
| `OPTION` | `*RTTIALL *SHOWSRC` | Full RTTI; include source in listing |
| `MSGLMT` | `*NOMAX 20` | No message limit; severity cap of 20 |
| `REPLACE` | `*YES` | Replace existing module |
| `TGTRLS` | `*CURRENT` | Target current OS release |

The resulting program is linked with:

| Option | Value |
|--------|-------|
| `ACTGRP` | `*CALLER` — runs in the caller's activation group |
| `USRPRF` | `*USER` — adopts the calling user's profile |
| `STGMDL` | `*INHERIT` |
| `OPTION` | `*DUPPROC *DUPVAR` — tolerates duplicate procedure/variable names |
| `ALWLIBUPD` | `*YES` — allows library list updates at bind time |

---

## Examples

### Build with all defaults (production build)

```
BLDUDTF UDTF(SPOOLDATA) EXTPGM(SPOOLDATA)
```

Compiles `QCSRC/SQLTOOLS(SPOOLDATA)` into module and program `SQLTOOLS/SPOOLDATA`,
then runs `RUNSQLSTM` against `QUDFSRC/SQLTOOLS(SPOOLDATA)` to register the SQL function.
The existing SQL function is **not** dropped first (default `DROP(*NO)`).

---

### Drop using a different specific name

```
BLDUDTF UDTF(SPOOLDATA)  EXTPGM(SPOOLDATA)
        DROP(*DROP)  SPECNAME(SPOOL_DATA)
```

Runs `DROP FUNCTION SQLTOOLS.SPOOL_DATA` before building — useful when the
SQL specific name differs from the UDTF source member name.

---

### Force DROP before build

```
BLDUDTF UDTF(SPOOLDATA) EXTPGM(SPOOLDATA)
        DROP(*DROP)
```

Runs `DROP FUNCTION SQLTOOLS.SPOOLDATA` before compiling, then rebuilds the
program and re-registers the SQL function.  Errors from the `DROP` (e.g., the
function does not yet exist) are ignored.

---

### Development build with source-level debug

```
BLDUDTF UDTF(SPOOLDATA) EXTPGM(SPOOLDATA)
        DBGVIEW(*SOURCE)
```

Reduces optimization to level 10, disables inlining, and embeds source-level
debug data so the ILE debugger can step through the C++ source.

---

### Build from non-default source locations

```
BLDUDTF UDTF(MYudtf)       EXTPGM(MYPGM)
        EXTPGMSRC(QCSRC/DEVLIB)
        EXTPGMMBR(MYCSRC)
        UDTFSRC(QSQLSRC/DEVLIB)
        OBJLIB(DEVLIB)
```

Reads C++ source from `DEVLIB/QCSRC(MYCSRC)` and writes the compiled objects
to `DEVLIB`.

---

## Error Handling

The CPP monitors for `CPF0000` (any CPF escape message) at the program level.
If any step fails the program branches to `ENDPGM` and the job log will contain
the diagnostic messages from the failing step (`RTVMBRD`, `CRTCPPMOD`, `CRTPGM`,
or `RUNSQLSTM`).  The `DROP FUNCTION` SQL statement has its own local
`MONMSG CPF0000` so a missing-function error does not abort the build.

---

## Related Objects

| Object | Type | Purpose |
|--------|------|---------|
| `BLDUDTF` | `*CMD` | The command described in this document |
| `BLDUDTF` | `*PGM` | CL program that implements the command |
| `QCSRC` | `*FILE` | Default source file for C/C++ source members |
| `QUDFSRC` / `QSQLSRC` | `*FILE` | Default source files for SQL `CREATE FUNCTION` DDL |
| `SQLTOOLS` | `*LIB` | Default library for all objects |
