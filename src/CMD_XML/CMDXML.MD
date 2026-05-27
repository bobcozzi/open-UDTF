# CMD_XML — CL Command Definition UDTF

Returns the full source definition of a CL command as XML, retrieved via the
IBM i `QCDRCMDD` system API (`CMDD0200` format).  The XML contains all
definition statements for the command — `CMD`, `PARM`, `QUAL`, `ELEM`, `DEP`,
and `PMTCTL` — making it suitable for processing with `XMLTABLE` or other
SQL XML functions.

Because the implementation is pure ILE C++ there is no JVM startup overhead.
At most **one row** is returned per call.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CMD_XML`
**External program:** `SQLTOOLS/CMDXML`

---

## Source files

| File | Target member | Purpose |
|------|---------------|---------|
| `/src/CMD_XML/CMDXML.CPP` | `SQLTOOLS/QCSRC(CMDXML)` | ILE C++ UDTF program |
| `/src/CMD_XML/CMDXML.SQL` | `SQLTOOLS/QUDFSRC(CMDXML)` | SQL UDTF definition |

---

## Signature

```sql
SELECT CMD_XML
  FROM TABLE(
    sqltools.CMD_XML(
        LIBRARY_NAME => '*LIBL',
        CMD_NAME     => 'CHGVAR'
    )
  ) AS t;
```

`LIBRARY_NAME` is optional; `CMD_NAME` is required.

---

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(10)`  **Default:** `*LIBL`

The library used to locate the `*CMD` object named on `CMD_NAME`.
The value is case-insensitive.

| Value | Meaning |
|-------|---------|
| `*LIBL` *(default)* | Job library list |
| `*CURLIB` | Job's current library |
| *library-name* | Specific library |

---

### CMD_NAME
**Type:** `VARCHAR(10)`  **Required**

The name of the CL command whose definition XML is to be retrieved.
The value is case-insensitive.

---

## Result columns

| Column | Type | Description |
|--------|------|-------------|
| `CMD_XML` | `CLOB(16M) CCSID 1208` | Full command definition XML returned by `QCDRCMDD` (`CMDD0200` format) |

The XML includes the following CL source statement types where present in
the command definition:

| Statement | Description |
|-----------|-------------|
| `CMD` | Command-level attributes (prompt text, validity checker, etc.) |
| `PARM` | Parameter definitions |
| `QUAL` | Qualifier definitions (for qualified object name parameters) |
| `ELEM` | Element definitions (for list parameters) |
| `DEP` | Dependency rules between parameters |
| `PMTCTL` | Prompt control rules |

> **Note:** The UDTF always returns exactly one row.

---

## How it works

1. **OPEN** — Calls `QCDRCMDD` with API format `CMDD0200` and output format
   `DEST0100` to retrieve the complete command definition into a temporary
   `*USRSPC` object created in `QTEMP`.
2. **FETCH** — Copies the returned bytes from the user space directly into
   the `CMD_XML` CLOB output column.  The EOF flag is set immediately so
   only one row is ever returned.
3. **CLOSE** — Deletes the temporary user space from `QTEMP`.

---

## Usage examples

### Retrieve the raw XML for a command

```sql
SELECT CMD_XML
  FROM TABLE(
    sqltools.CMD_XML(CMD_NAME => 'CRTPF')
  ) AS t;
```

### Extract parameter names with XMLTABLE

```sql
SELECT x.KWD, x.TYPE, x.DFT
  FROM TABLE(
    sqltools.CMD_XML(
        LIBRARY_NAME => 'QSYS',
        CMD_NAME     => 'CRTPF'
    )
  ) AS t,
  XMLTABLE('$doc/QcdCLCmd/Cmd/Parm'
      PASSING XMLPARSE(DOCUMENT t.CMD_XML) AS "doc"
      COLUMNS
          KWD  VARCHAR(10) PATH '@Kwd',
          TYPE VARCHAR(10) PATH '@Type',
          DFT  VARCHAR(32) PATH '@Dft'
  ) AS x;
```

### Combine with CMD_HELP to get both definition and help text

```sql
SELECT x.CMD_XML, h.HELP_XML
  FROM TABLE(sqltools.CMD_XML (CMD_NAME => 'CHGVAR')) AS x,
       TABLE(sqltools.CMD_HELP(CMD_NAME => 'CHGVAR',
                               HELPID   => 'VAR,VALUE')) AS h;
```

---

## Error handling

| Condition | Behaviour |
|-----------|-----------|
| Command not found / `QCDRCMDD` fails | SQLSTATE `02000` (no data); function returns −1 |
| `QCDRCMDD` returns zero bytes | SQLSTATE `02000`; function returns −1 |
| Error re-signalled from `QCDRCMDD` | IBM i message is re-signalled to the caller |

---

## Notes

- The UDTF is defined as `STATEMENT DETERMINISTIC` and `NO SQL`.
- `SCRATCHPAD 256` carries the command name, library, and user-space name
  from OPEN to FETCH.
- `CARDINALITY 1` informs the optimizer that exactly one row is returned,
  enabling better join and access-plan decisions.
- No Java or JVM is involved — there is no warm-up delay on first call.
- The maximum XML output size is approximately 15 MB (`coz_MAXMEM15M`).
