# FIELD_LIST — File Field Definitions UDTF

Returns field (column) definitions for one or more IBM i database files,
similar to the `DSPFFD` CL command but accessible directly from SQL.
Both the SQL data type definition and the RPG IV field declaration are
returned for each field.  Any number of files may be specified, allowing
all field formats to be retrieved in a single query.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.FIELD_LIST`
**External program:** `SQLTOOLS/FIELDLIST`

---

## Source files

| File | Target member | Purpose |
|------|---------------|---------|
| `/src/FILE_FIELDS/FIELDLIST.CPP` | `SQLTOOLS/QCSRC(FIELDLIST)` | ILE C++ UDTF program |
| `/src/FILE_FIELDS/FIELDLIST.SQL` | `SQLTOOLS/QUDFSRC(FIELDLIST)` | SQL UDTF definition |
| `/src/shared/h/COZUTILS.H`       | `SQLTOOLS/H(COZUTILS)`        | Shared utility header |
| `/src/shared/h/OBJLIST.H`        | `SQLTOOLS/H(OBJLIST)`         | Shared CObjList header |
---

## Signature

```sql
SELECT *
  FROM TABLE(
    sqltools.FIELD_LIST(
        LIBRARY_NAME => '*LIBL',
        FILE_NAME    => 'MYFILE',
        FILE_TYPE    => '*ALL'
    )
  ) AS t;
```

All parameters are optional except `FILE_NAME`.

---

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(128)`  **Default:** `*LIBL`

The library (schema) used to qualify unqualified file names specified on
`FILE_NAME`.  The parameter is case-insensitive unless the name is enclosed
in double-quotes.

| Value | Meaning |
|-------|---------|
| `*LIBL` *(default)* | Job library list |
| `*CURLIB` | Job's current library |
| `*USRLIBL` | User portion of the job library list |
| `*ALLUSR` | All user libraries |
| `*ALL` | All libraries |
| *library-name* | Specific library |

---

### FILE_NAME
**Type:** `CLOB(1M)`  **Required**

One or more file names whose field definitions are to be returned.
Names are separated by one or more blanks and/or commas.  Each name may be
optionally library-qualified (`LIBRARY/FILE`).  When unqualified, the
library specified on `LIBRARY_NAME` is used.

The parameter accepts full names and generic names (e.g. `ORD*`).
Long SQL names are supported but may not be generic names.
The parameter is case-insensitive.

**Examples:**

```sql
FILE_NAME => 'CUSTOMER'
FILE_NAME => 'DATALIB/CUSTOMER DATALIB/ORDDATA'
FILE_NAME => 'CUSTOMER, ORDDATA, QIWS/QCUSTCDT'
FILE_NAME => 'DATALIB/CUSTOMER orddata testlib/ORDDATA'
```

---

### FILE_TYPE
**Type:** `VARCHAR(32)`  **Default:** `*ALL`

Filters results to files matching the specified type(s).  Multiple values
may be separated by blanks and/or commas (e.g. `'PF LF'`).
The leading asterisk and letter case are ignored.

| Value | Meaning |
|-------|---------|
| `*ALL` *(default)* | All database file types |
| `PF` | Physical files (*DATA and *SRC) |
| `LF` | Logical views |
| `DDMF` | DDM files |
| `*DATA` | Non-source physical files (PF, LF, DDMF) |
| `*SRC` | Source physical files only (PF-SRC) |

> **Note:** Specifying `PF` returns both data and source physical files.
> Use `*SRC` to restrict results to source files only.

---

## Result columns

### File identification

| Column | Type | Description |
|--------|------|-------------|
| `OBJLIB` | VARCHAR(10) | Library containing the file |
| `FILE_NAME` | VARCHAR(10) | Short system file name |
| `LONG_FILE_NAME` | VARCHAR(128) | Long SQL file name |
| `FILE_ATTR` | VARCHAR(10) | File attribute: PF, LF, DDMF |
| `FILE_TYPE` | VARCHAR(10) | File type: \*SRC or \*DATA |
| `SQL_OBJECT_TYPE` | VARCHAR(10) | SQL object type: TABLE, VIEW, INDEX |
| `FILE_TEXT` | VARCHAR(50) | File text description |
| `CRTDATE` | TIMESTAMP(0) | File creation timestamp |

### Record format

| Column | Type | Description |
|--------|------|-------------|
| `FIELD_COUNT` | INT | Number of columns in the record format |
| `RCDFMT_COUNT` | INT | Number of record formats in the file |
| `RCDFMT` | VARCHAR(10) | Record format name |
| `RCDFMT_ID` | VARCHAR(13) | Record format identifier |
| `RCDFMT_ORDINAL` | INT | Ordinal position of the record format |
| `rcdFmt_TEXT` | VARCHAR(50) | Record format text description |
| `RCDLEN` | SMALLINT | Record length |
| `LVLCHK` | VARCHAR(10) | Level checking active: \*YES or \*NO |
| `LVLCHK_ID` | VARCHAR(13) | 13-byte Level Check ID |

### Field definition

| Column | Type | Description |
|--------|------|-------------|
| `ORDINAL_POSITION` | INT | Field ordinal position in the record |
| `FIELD_NAME` | VARCHAR(10) | Short system field name |
| `FIELD_ALIAS` | VARCHAR(128) | Field ALIAS or SQL long field name |
| `FIELD_TEXT` | VARCHAR(50) | Field text description |
| `DATA_TYPE` | VARCHAR(10) | SQL data type |
| `FIELD_LENGTH` | INT | Declared field length |
| `DECPOS` | SMALLINT | Decimal positions (numeric fields) |
| `BUFFER_LENGTH` | INT | Bytes the field occupies in the buffer |
| `FROM_POS` | INT | Field start position in buffer |
| `TO_POS` | INT | Field end position in buffer |
| `CCSID` | INT | Coded character set identifier |
| `ALWNULL` | VARCHAR(10) | Whether null values are allowed |
| `FIELD_USAGE` | VARCHAR(10) | Field usage: Input, Output, or Both |
| `COLUMN_HEADINGS` | VARCHAR(60) | 3-part column heading, 20 bytes each |
| `DEFN_KWDS` | VARCHAR(64) | Definition keywords: DATFMT, CCSID, etc. |
| `RPG_FIELD_DEFN` | VARCHAR(32) | RPG IV field declaration |
| `FIELD_PROC` | VARCHAR(10) | RCAC exit program name |
| `FIELD_PROCLIB` | VARCHAR(10) | Library of the RCAC exit program |

---

## Examples

### All fields in a single file
```sql
SELECT *
  FROM TABLE(
    sqltools.FIELD_LIST(FILE_NAME => 'QCUSTCDT', LIBRARY_NAME => 'QIWS')
  ) AS t;
```

### Fields from multiple files
```sql
SELECT *
  FROM TABLE(
    sqltools.FIELD_LIST(
        LIBRARY_NAME => 'DATALIB',
        FILE_NAME    => 'CUSTOMER, ORDDATA, ORDLINES'
    )
  ) AS t
 ORDER BY FILE_NAME, ORDINAL_POSITION;
```

### Only physical data files in a library
```sql
SELECT FILE_NAME, FIELD_NAME, DATA_TYPE, FIELD_LENGTH, DECPOS, FIELD_TEXT
  FROM TABLE(
    sqltools.FIELD_LIST(
        LIBRARY_NAME => 'MYLIB',
        FILE_NAME    => '*ALL',
        FILE_TYPE    => '*DATA'
    )
  ) AS t
 ORDER BY FILE_NAME, ORDINAL_POSITION;
```

### Find all files that contain a field named CUSTNO
```sql
SELECT OBJLIB, FILE_NAME, ORDINAL_POSITION, DATA_TYPE, FIELD_LENGTH
  FROM TABLE(
    sqltools.FIELD_LIST(LIBRARY_NAME => 'MYLIB', FILE_NAME => '*ALL')
  ) AS t
 WHERE FIELD_NAME = 'CUSTNO';
```

---

## Build

Use the `BLDUDTF` helper command to compile and register the UDTF:

```clle
BLDUDTF UDTF(FIELDLIST)
        EXTPGM(FIELDLIST)
        EXTPGMSRC(SQLTOOLSRC/QCSRC)
        UDTFSRC(SQLTOOLSRC/QUDFSRC)
        OBJLIB(SQLTOOLS)
        DBGVIEW(*NONE)
        DROP(*NO)
```

Assumptions:
- The target (object) library is `SQLTOOLS` (recommended)
- Source code resides in library `SQLTOOLSRC`
- No debug info is stored; use `DBGVIEW(*SOURCE)` when debugging
- `DROP(*NO)` is safe because the DDL uses `CREATE OR REPLACE FUNCTION`;
  specify `DROP(*YES)` if the build fails due to an existing object

---

## Notes

- Results are equivalent to running `DSPFFD` but are available directly
  in SQL and can be joined, filtered, and aggregated like any table.
- Both the SQL data type (`DATA_TYPE`, `DEFN_KWDS`) and the RPG IV
  declaration (`RPG_FIELD_DEFN`) are returned for each field.
- `COLUMN_HEADINGS` contains the three 20-byte heading parts
  concatenated into a single 60-byte column.
- The `FIELD_ALIAS` column reflects the `ALIAS` DDS keyword or the
  long column name defined on a SQL `CREATE TABLE` statement.
- For files with multiple record formats, one set of rows is returned
  per format.  Use `RCDFMT` to distinguish between formats.
