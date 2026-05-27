# CMD_HELP — CL Command Help Text UDTF

Returns the XML help text for a CL command's panel group, retrieved via the
IBM i `QUHRHLPT` system API.  Because the implementation is pure ILE C++ there
is no JVM startup overhead — results are available immediately on any connection.

At most **one row** is returned per call.  The XML content mirrors what the
`QUHRHLPT` API produces for the requested Help IDs.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CMD_HELP`
**External program:** `SQLTOOLS/CMDHELP`

---

## Source files

| File | Target member | Purpose |
|------|---------------|---------|
| `/src/CMD_HELP/CMDHELP.CPP` | `SQLTOOLS/QCSRC(CMDHELP)` | ILE C++ UDTF program |
| `/src/CMD_HELP/CMDHELP.SQL` | `SQLTOOLS/QUDFSRC(CMDHELP)` | SQL UDTF definition |

---

## Signature

```sql
SELECT HELP_XML
  FROM TABLE(
    sqltools.CMD_HELP(
        LIBRARY_NAME => 'QSYS',
        CMD_NAME     => 'CHGVAR',
        HELPID       => 'VAR,VALUE'
    )
  ) AS t;
```

`LIBRARY_NAME` and `HELPID` are optional; `CMD_NAME` is required.

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

The name of the CL command whose help text is to be retrieved.
The value is case-insensitive.

---

### HELPID
**Type:** `VARCHAR(6000)`  **Default:** `*CMD`

A delimited list of Help IDs whose text is to be returned. Tokens may be
separated by blanks, commas, or colons.  Each token is a keyword (parameter
name) of the specified command. The value is case-insensitive. The special value
`*CMD` may be specified to indicate that the main command help text is to be returned.
`*CMD` may be included with the list of other helpid's. The command name itself may
also be specified.

**Examples of the HELPID Parameter:**

```sql
HELPID => 'VAR'
HELPID => 'VAR,VALUE'
HELPID => 'VAR VALUE TOVAR'
HELPID => 'OPTION:REPLACE:TODIR'
```

Internally the UDTF prefixes each token with the command name to form the
`CMDNAME/KEYWORD` Help ID string used as a CL command help text standard for
help text IDs.

---

## Result columns

| Column | Type | Description |
|--------|------|-------------|
| `HELP_XML` | `CLOB(16M) CCSID 1208` | XML help text returned by `QUHRHLPT` for the requested Help IDs |

> **Note:** The UDTF always returns exactly one row.  The XML structure is
> actually valid HTML that can be used in a web browser or other HTML interfaces.

---

## How it works

1. **OPEN** — Calls `QCDRCMDI` (`CMDI0100` format) to look up the panel group
   name and library associated with the command.  Three temporary `*USRSPC`
   objects are created in `QTEMP` to hold intermediate data.
2. **FETCH** — Tokenizes `HELPID`, builds the `Quh_Ids_To_Rtv_t` array, and
   calls `QUHRHLPT` (`RHLP0100` format).  The resulting XML is placed in the
   `HELP_XML` output column.  The EOF flag is set immediately so only one row
   is ever returned.
3. **CLOSE** — Deletes the three temporary user spaces from `QTEMP`.

---

## Usage examples

### Retrieve help for a single parameter

```sql
SELECT HELP_XML
  FROM TABLE(
    sqltools.CMD_HELP(
        CMD_NAME => 'CHGVAR',
        HELPID   => 'VAR'
    )
  ) AS t;
```

### Retrieve help for multiple parameters

```sql
SELECT HELP_XML
  FROM TABLE(
    sqltools.CMD_HELP(
        LIBRARY_NAME => 'QSYS',
        CMD_NAME     => 'CRTPF',
        HELPID       => 'FILE,SRCFILE,SRCMBR,RCDLEN'
    )
  ) AS t;
```

### Use inside a larger query with XMLTABLE

```sql
SELECT x.*
  FROM TABLE(
    sqltools.CMD_HELP(
        CMD_NAME => 'CHGVAR',
        HELPID   => 'VAR,VALUE'
    )
  ) AS h,
  XMLTABLE('...' PASSING XMLPARSE(DOCUMENT h.HELP_XML) ...) AS x;
```

---

## Error handling

| Condition | Behaviour |
|-----------|-----------|
| Command not found / `QCDRCMDI` fails | SQLSTATE `02000` (no data); function returns −1 |
| `QUHRHLPT` error | Error message is re-signalled to the caller |
| Invalid / empty `HELPID` | No tokens are built; `QUHRHLPT` is called with zero IDs and returns empty XML |

---

## Notes

- The UDTF is defined as `STATEMENT DETERMINISTIC` and `NO SQL`.
- `SCRATCHPAD 256` is used to carry panel-group metadata from OPEN to FETCH.
- No Java or JVM is involved — there is no warm-up delay on first call.
- The maximum XML output size is approximately 16 MB.
