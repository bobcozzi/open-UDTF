# FIELD_LIST — File Field Definitions UDTF

Returns file field (column) definitions similar to `DSPFFD`, but directly in SQL.
For each field, both SQL and RPG IV data type definitions are returned.
The function works for all device types, database, printer, workstation, DDMF etc.
All parameters ignore upper/lower case unless the value is enclosed in double quotes: (e.g., "OLDFile")

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.FIELD_LIST`
**External program:** `SQLTOOLS/FIELDLIST`

---

## Source files

| File                            | Target member                 | Purpose              |
| ------------------------------- | ----------------------------- | -------------------- |
| `/src/FIELD_LIST/FIELDLIST.CPP` | `SQLTOOLS/QCSRC(FIELDLIST)`   | ILE C++ UDTF program |
| `/src/FIELD_LIST/FIELDLIST.SQL` | `SQLTOOLS/QUDFSRC(FIELDLIST)` | SQL UDTF definition  |

---
## Compiling / Build
After uploading the two source file members (FILELIST.SQL and FILELIST.CPP) to an IBM i Host, you need to compile them using the SQL RUNSQLSTM command and the C++ compiler CRTCPPMOD+CRTPGM, respectively.
The C++ requires a 2-step process in order to correctly specify the *PGM's Activation Group, which must be *CALLER for SQL functions.
Here is an example assuming you uploaded to my recommended SQLTOOLS library:
### Compiling FIELDLIST.CPP
`CRTCPPMOD MODULE(SQLTOOLS/FIELDLIST) SRCFILE(SQLTOOLSRC/QCSRC) OUTPUT(*PRINT) DBGVIEW(*SOURCE) SYSIFCOPT(*IFS64IO)`
`CRTPGM PGM(SQLTOOLS/FIELDLIST) MODULE(SQLTOOLS/FIELDLIST) ACTGRP(*CALLER)`
### Compiling FIELDLIST.SQL
`RUNSQLSTM SRCMBR(FIELDLIST) SRCFILE(SQLTOOLS/QSQLSRC)`

## Signature

```sql
SELECT *
  FROM TABLE(
    sqltools.FIELD_LIST(
      LIBRARY_NAME => '*LIBL',
      FILE_NAME    => '<file_name>',  -- Required
      RCDFMT       => '*ALL',
      OVR          => 'NO',
      LOG          => 'NO'
    )
  ) AS t;
```

---

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(10)`  **Default:** `*LIBL`

Library used to qualify `FILE_NAME`.

| Value               | Meaning              |
| ------------------- | -------------------- |
| `*LIBL` *(default)* | Use job library list |
| `*CURLIB`           | Use current library  |
| *library-name*      | Use specific library |

Upper/lower case is ignored unless enclosed in double quotes.

### FILE_NAME
**Type:** `VARCHAR(10)`  **Required**

File object name to inspect.
Upper/lower case is ignored unless enclosed in double quotes.

Examples:

```sql
FILE_NAME => 'CUSTOMER'
FILE_NAME => 'QCustCDT' -- auto converted to 'QCUSTCDT'
FILE_NAME => "OldFile"  -- case conversion is not performed
```

### RCDFMT
**Type:** `VARCHAR(10)`  **Default:** `*ALL`

Record format whose fields are returned.

| Value              | Meaning                          |
| ------------------ | -------------------------------- |
| `*ALL` *(default)* | Return all record formats        |
| `*FIRST`           | Return first record format       |
| `*LAST`            | Return last record format        |
| `*ONLY`            | Return only format (or `*FIRST`) |
| *record-format*    | Return specific record format    |

### OVR
**Type:** `VARCHAR(10)`  **Default:** `NO`

Controls whether active overrides that are applied to the name specified on the FILE_NAME parameter are respected.

| Value  | Meaning                        |
| ------ | ------------------------------ |
| `*NO`  | Do not use overrides (default) |
| `*YES` | Respect active overrides       |

Leading `*` is optional and case is ignored. Only the first character is tested (after trimming off any leading '*')

### LOG
**Type:** `VARCHAR(10)`  **Default:** `NO`

Controls debug logging to the job log.

| Value  | Meaning                            |
| ------ | ---------------------------------- |
| `*NO`  | No debug logging (default)         |
| `*YES` | Write debug information to job log |

Leading `*` is optional and case is ignored. Only the first character is tested (after trimming off any leading '*')
---

## Result columns

The function returns the following columns (from `FIELDLIST.SQL`):

| Column                | Type            | Description                                                                                                                         |
| --------------------- | --------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| `LIBRARY_NAME`        | `VARCHAR(10)`   | Library where the file object was found.                                                                                            |
| `FILE_NAME`           | `VARCHAR(10)`   | System object name of the file.                                                                                                     |
| `FILE_TYPE`           | `VARCHAR(10)`   | File attribute/type (for example PF, LF, DSPF, PRTF).                                                                               |
| `RCDFMT`              | `VARCHAR(10)`   | Record format name for the field.                                                                                                   |
| `COLUMN_COUNT`        | `INT`           | Number of fields in this record format.                                                                                             |
| `ORDINAL_POSITION`    | `INT`           | Field sequence number within the record format.                                                                                     |
| `COLUMN_NAME`         | `VARCHAR(10)`   | Short/system field (column) name.                                                                                                   |
| `SQL_DATA_TYPE`       | `VARCHAR(16)`   | SQL data type keyword for the field.                                                                                                |
| `RPG_DATA_TYPE`       | `VARCHAR(16)`   | RPG IV data type keyword for DCL-S style declarations.                                                                              |
| `COLUMN_LENGTH`       | `INT`           | Declared length (characters/bytes/digits depending on data type).                                                                   |
| `DATA_TYPE`           | `CHAR(1)`       | One-byte internal data type code.                                                                                                   |
| `DEC_POS`             | `INT`           | Decimal scale for numeric fields; usually null or 0 for non-decimal types.                                                          |
| `BUFFER_LENGTH`       | `INT`           | Storage length in bytes used by the field in the record buffer.                                                                     |
| `INPUT_BUFFER_POS`    | `INT`           | Starting byte position of the field in the input buffer.                                                                            |
| `OUTPUT_BUFFER_POS`   | `INT`           | Starting byte position of the field in the output buffer.                                                                           |
| `USAGE`               | `VARCHAR(10)`   | Field usage direction (input, output, or both).                                                                                     |
| `COLUMN_CCSID`        | `INT`           | CCSID associated with character data for the field.                                                                                 |
| `SQL_DEFN`            | `VARCHAR(64)`   | Complete SQL data type including length/precision/scale as applicable.                                                              |
| `RPG_DEFN`            | `VARCHAR(64)`   | Complete RPG IV data type including length/precision/scale as applicable.                                                           |
| `COLHDG1`             | `VARCHAR(20)`   | Column heading line 1 (DDS-style heading text line 1).                                                                              |
| `COLHDG2`             | `VARCHAR(20)`   | Column heading line 2 (DDS-style heading text line 2).                                                                              |
| `COLHDG3`             | `VARCHAR(20)`   | Column heading line 3 (DDS-style heading text line 3).                                                                              |
| `INTERNAL_NAME`       | `VARCHAR(10)`   | Internal/system column name.                                                                                                        |
| `LONG_NAME`           | `VARCHAR(255)`  | Long SQL column name or DDS alias name when available.                                                                              |
| `COLUMN_TEXT`         | `VARCHAR(50)`   | Descriptive text for the field/column.                                                                                              |
| `RCDFMT_TEXT`         | `VARCHAR(50)`   | Descriptive text for the record format.                                                                                             |
| `EDIT_CODE`           | `VARCHAR(2)`    | Edit code assigned to the field for display/print formatting.                                                                       |
| `EDIT_WORD`           | `VARCHAR(64)`   | Edit word pattern used for display/print formatting.                                                                                |
| `DATE_TIME_FMT`       | `VARCHAR(4)`    | Date/time format keyword associated with the field.                                                                                 |
| `DATE_TIME_SEP`       | `CHAR(1)`       | Separator character used by the date/time format.                                                                                   |
| `UCS2_DSP_LENGTH`     | `INT`           | Display length in UCS-2 units for applicable character fields.                                                                      |
| `DBCS_CHARS`          | `INT`           | Number of double-byte characters for applicable VARGRAPHIC/GRAPHIC data.                                                            |
| `LOB_MAX_LENGTH`      | `INT`           | Maximum allowed length for LOB fields.                                                                                              |
| `LOB_PAD_LENGTH`      | `INT`           | Pad length information for applicable LOB definitions.                                                                              |
| `UDT_NAME`            | `VARCHAR(128)`  | User-defined type name when the field uses a UDT.                                                                                   |
| `UDT_LIBNAME`         | `VARCHAR(10)`   | Library/schema containing the user-defined type.                                                                                    |
| `DSP_ROW_NBR`         | `INT`           | Display/printer row position (DSPF/PRTF file types only).                                                                           |
| `DSP_COL_NBR`         | `INT`           | Display/printer column position (DSPF/PRTF file types only).                                                                        |
| `IS_NULLABLE`         | `CHAR(1)`       | '1'/'0' flag indicating whether null values are allowed.                                                                            |
| `IS_UPDATEABLE`       | `CHAR(1)`       | '1'/'0' flag indicating whether the column can be updated.                                                                          |
| `IS_VARCHAR`          | `CHAR(1)`       | '1'/'0' flag indicating variable-length character storage.                                                                          |
| `IS_HIDDEN`           | `CHAR(1)`       | '1'/'0' flag indicating a hidden column.                                                                                            |
| `IS_ROWID`            | `CHAR(1)`       | '1'/'0' flag indicating a ROWID column.                                                                                             |
| `IS_IDENTITY`         | `CHAR(1)`       | '1'/'0' flag indicating an identity column.                                                                                         |
| `IS_TIMESTAMP`        | `CHAR(1)`       | '1'/'0' flag indicating a timestamp column.                                                                                         |
| `HAS_DEFAULT`         | `CHAR(1)`       | '1'/'0' flag indicating that a default value/expression is defined.                                                                 |
| `GENERATED_WHEN`      | `CHAR(10)`      | Generated-column timing (for example DEFAULT/ALWAYS), when applicable.                                                              |
| `GENERATED_FOR`       | `VARCHAR(32)`   | Generated-column mode/context (for example identity or date/time stampt).                                                           |
| `PROC_PGMNAME`        | `VARCHAR(10)`   | The name of the field procedure program that will be called when this field is written/inserted into, read/fetched from or updated. |
| `PROC_LIBNAME`        | `VARCHAR(10)`   | The name of the library in which the field procedure program is located.                                                            |
| `TIMESTAMP_PRECISION` | `SMALLINT`      | Fractional-seconds/precision for timestamp columns.                                                                                 |
| `DEFAULT_VALUE`       | `VARCHAR(2000)` | Default value (if any) for the column.                                                                                              |

---

## Examples

### Basic usage (single file)
```sql
SELECT *
  FROM TABLE(
    sqltools.FIELD_LIST(
      'QIWS',
      FILE_NAME    => 'QCUSTCDT'
    )
  ) AS t
 ORDER BY RCDFMT, ORDINAL_POSITION;
```

### First record format only
```sql
SELECT RCDFMT, ORDINAL_POSITION, COLUMN_NAME, SQL_DEFN, RPG_DEFN
  FROM TABLE(
    sqltools.FIELD_LIST(
      'QIWS',
      'QCUSTCDT',
      '*FIRST'
    )
  ) AS t
 ORDER BY ORDINAL_POSITION;
```

### Respect overrides and enable debug logging
```sql
SELECT COLUMN_NAME, SQL_DATA_TYPE, COLUMN_LENGTH, DEC_POS
  FROM TABLE(
    sqltools.FIELD_LIST(
      LIBRARY_NAME => '*LIBL',
      FILE_NAME    => 'MYFILE',
      RCDFMT       => '*ALL',
      OVR          => '*YES',
      LOG          => '*YES'
    )
  ) AS t
 ORDER BY RCDFMT, ORDINAL_POSITION;
```

---

## Notes

- Output is similar to `DSPFFD`, but queryable with SQL.
- Both SQL and RPG IV datatype definitions are returned (`SQL_DEFN`, `RPG_DEFN`).
- `LONG_NAME` contains the SQL long column name (or DDS alias when applicable).
