# DLTOBJ - Delete IBM i Object Function

Deletes an IBM i object and returns a status code.
This function supports both traditional 10-character system names and long SQL names
for `LIBRARY_NAME` and `OBJECT_NAME`.

By default, it protects system libraries from accidental deletion.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.DELETE_OBJECT`
**External program:** `SQLTOOLS/DLTOBJ`

---

## Source files

| File                      | Target member              | Purpose                           |
| ------------------------- | -------------------------- | --------------------------------- |
| `/src/DLT_OBJ/DLTOBJ.CPP` | `SQLTOOLSRC/QCSRC(DLTOBJ)` | ILE C++ external function program |
| `/src/DLT_OBJ/DLTOBJ.SQL` | `SQLTOOLS/QUDFSRC(DLTOBJ)` | SQL function definition           |

---

## Signature

```sql
SELECT SQLTOOLS.DLTOBJ(
    LIBRARY_NAME  => '*LIBL',
    OBJECT_NAME   => 'MYFILE',
    OBJTYPE       => '*FILE',
    ASP_NAME      => '*SYSBAS',
    RMVMSG        => '*NO',
    PROTECT       => '*YES',
    CONVERT_NAME  => '*YES'
) AS DELETE_STATUS;
```

---

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(128)`  **Default:** `*LIBL`

Library containing the object to delete.
You may specify a short system name, long SQL name, or symbolic values.

| Value               | Meaning                        |
| ------------------- | ------------------------------ |
| `*LIBL` *(default)* | Resolve using job library list |
| `*CURLIB`           | Resolve using current library  |
| *library-name*      | Use the specified library      |

---

### OBJECT_NAME
**Type:** `VARCHAR(128)`  **Required**

Object name to delete.
May be a short system name,  a generic object name, or for *FILE objects a long SQL name may be specified.
`*ALL` is blocked by the function.

---

### OBJTYPE
**Type:** `VARCHAR(10)`  **Required**

IBM i object type for the object being deleted.
Leading `*` is optional and upper/lower case is ignored.

Examples:
- `*FILE`
- `PGM`
- `*SRVPGM`

---

### ASP_NAME
**Type:** `VARCHAR(10)`  **Default:** `*SYSBAS`

ASP device name where the library is located.
Upper/lower case is ignored.

| Value                 | Meaning             |
| --------------------- | ------------------- |
| `*`                   | Any ASP             |
| `*SYSBAS` *(default)* | System ASP          |
| `*CURASPGRP`          | Current ASP group   |
| `*ALLAVL`             | All available ASPs  |
| *asp-device-name*     | Specific ASP device |

---

### RMVMSG
**Type:** `VARCHAR(10)`  **Default:** `*NO`

Controls whether a successful object-delete completion message such as `CPC2191` is suppressed
(removed from the joblog). Note that other messages, such as CPF2105 (object not found) are never suppressed.
Upper/lower case is ignored.

| Value             | Meaning                     |
| ----------------- | --------------------------- |
| `*NO` *(default)* | Keep completion message     |
| `*YES`            | Suppress completion message |
| `0` / `1`         | Same as `*NO` / `*YES`      |

---

### PROTECT
**Type:** `VARCHAR(10)`  **Default:** `*YES`

Controls library safety checks.
When enabled, the function rejects deletes in protected system libraries.

Protected prefixes include libraries that start with:
- `Q` All libraries that start with `Q` such as QSYS, QUSRSYS, QGPL, etc.
- `SYS` Libraries that start with `SYS`.
- `SQL` Libraries that start with `SQL`.

`QTEMP` is always/explicitly allowed.

| Value              | Meaning                         |
| ------------------ | ------------------------------- |
| `*YES` *(default)* | Enforce protected-library check |
| `*NO`              | Skip protected-library check    |
| `TRUE` / `FALSE`   | Accepted equivalents            |

---

### CONVERT_NAME
**Type:** `VARCHAR(10)`  **Default:** `*YES`

Controls automatic uppercasing of `LIBRARY_NAME` and `OBJECT_NAME` before resolution.

| Value              | Meaning                    |
| ------------------ | -------------------------- |
| `*YES` *(default)* | Convert names to uppercase |
| `*NO`              | Preserve incoming case     |
| `TRUE` / `FALSE`   | Accepted equivalents       |

Notes:
- Names enclosed in double quotes (e.g., `"Old.S36f"`) are nover converted to upper case.
- This helps when working with rare mixed/lowercase object names.

---

## Return value

Returns `INT`:
- `1` = Object deleted
- `0` = Not deleted (blocked, not found, or API error) See joblog for details.

---

## Usage examples

### Delete file `MYTEMPFILE` from library list

```sql
VALUES SQLTOOLS.DLTOBJ(
    OBJECT_NAME => 'MYTEMPFILE',
    OBJTYPE     => '*FILE'
);
```

### Delete from a specific library without case conversion

```sql
VALUES SQLTOOLS.DLTOBJ(
    LIBRARY_NAME => 'MYLIB',     -- Library name specified in uppercase since CONVERT_NAME=>*NO specified.
    OBJECT_NAME  => 'Old.S36f',  -- Unquoted legacy mixed-case object name.
    OBJTYPE      => '*FILE',
    CONVERT_NAME => '*NO' -- Do not convert object or library name.
);
```
### Delete from a specific library without case conversion

```sql
VALUES SQLTOOLS.DLTOBJ(
    LIBRARY_NAME => 'myLib',  -- Converted to upperc case.
    OBJECT_NAME  => '"Old.S36f"',  -- Double-quotes prevent converting to upper case
    OBJTYPE      => '*FILE',
    CONVERT_NAME => '*YES'  -- Converts library name, but ignores OBJECT_NAME due to its quoting
);
```

### Allow delete in a protected-prefix library

```sql
VALUES SQLTOOLS.DLTOBJ(
    LIBRARY_NAME => 'QGPL',
    OBJECT_NAME  => 'testdata', -- converted to upper case
    OBJTYPE      => '*DTAARA',
    CONVERT_NAME => '*YES',
    PROTECT      => '*NO'
);
```

### Delete an SQL Table with a long name

```sql
VALUES SQLTOOLS.DLTOBJ(
    LIBRARY_NAME => 'QGPL',
    OBJECT_NAME  => 'testdata', -- converted to upper case
    OBJTYPE      => '*DTAARA',
    CONVERT_NAME => '*YES',
    PROTECT      => '*NO'
);
```

---

## Notes

- Implemented as an external ILE C++ SQL function (`LANGUAGE C++`, `NO SQL`).
- Not deterministic and marked `EXTERNAL ACTION` because it deletes objects.
- Long names are resolved to system names before the delete API is called.
- If `OBJECT_NAME` contains `*`, system name resolution for the object name is bypassed.
- If `LIBRARY_NAME` begins with or contains `*`, system name resolution for the library is bypassed.
