# OBJ_ATTR — Retrieve Object and Member Attributes

Returns metadata for an IBM i object or source member. The function can work with a standard library/object name combination or with a `/QSYS.LIB` path name, and it can optionally convert the path letter case before resolving the object.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.OBJ_ATTR`
**External program:** `SQLTOOLS/OBJ_ATTR`

---

## Source files

| File                           | Target member                  | Purpose                                       |
| ------------------------------ | ------------------------------ | --------------------------------------------- |
| `/src/OBJ_ATTR/OBJ_ATTR.RPGLE` | `SQLTOOLS/QRPGLESRC(OBJ_ATTR)` | External RPG IV program implementing the UDTF |
| `/src/OBJ_ATTR/OBJ_ATTR.SQL`   | `SQLTOOLS/QUDFSRC(OBJ_ATTR)`   | SQL function declaration for the UDTF         |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.OBJ_ATTR(
      PATH_NAME      => '/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE,MYOBJ.MBR',
      LIBRARY_NAME   => '*LIBL',
      OBJECT_NAME    => 'MYOBJ',
      OBJTYPE        => '*FILE',
      MBR_NAME       => 'MYOBJ',
      CVTOPTION      => 'NO'
    )
  ) AS T;
```

---

## Parameters

### PATH_NAME
**Type:** `VARCHAR(4096)`  **Default:** `NULL`

A `/QSYS.LIB` object name or IFS path name to inspect. When this value is supplied, it is used to resolve the object directly and the library/object/member parameters may be ignored.

Examples:

```text
/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE,MYOBJ.MBR
/QSYS.LIB/SQLTOOLS.LIB/QCLSRC.FILE,JOB_ATTR.MBR
/home/myuser/test.txt
```

---

### LIBRARY_NAME
**Type:** `VARCHAR(10)`  **Default:** `'*LIBL'`

The library containing the target object. This parameter is used when `OBJECT_NAME` is being resolved by library/object name rather than by path.

---

### OBJECT_NAME
**Type:** `VARCHAR(10)`  **Default:** `NULL`

The name of the object to inspect. This may be a database file, program, source file, or other IBM i object name.

---

### OBJTYPE
**Type:** `VARCHAR(10)`  **Default:** `'*FILE'`

The IBM i object type, such as `*FILE`, `*LIB`, `*PGM`, or `*SRVPGM`. The leading asterisk and upper/lower case are normally ignored.

---

### MBR_NAME
**Type:** `VARCHAR(10)`  **Default:** `NULL`

The member name to inspect when the target object is a source file or a file object with members. If omitted for a file object, attributes for the file itself are returned.

---

### CVTOPTION
**Type:** `VARCHAR(10)`  **Default:** `'NO'`

Controls how the path name should be treated for case conversion.

| Value   | Meaning                            |
| ------- | ---------------------------------- |
| `NO`    | Use the path exactly as supplied   |
| `UPPER` | Convert the path to uppercase      |
| `LOWER` | Convert the path to lowercase      |
| `YES`   | Equivalent to uppercase conversion |
| `NONE`  | No conversion                      |

---

## Result columns

The function returns a single row of metadata for the object or member.

| Column              | Type           | Description                            |
| ------------------- | -------------- | -------------------------------------- |
| `created_Date`      | `TIMESTAMP(0)` | Object creation date/time              |
| `access_Date`       | `TIMESTAMP(0)` | Last access time                       |
| `changed_Date`      | `TIMESTAMP(0)` | Last changed time                      |
| `data_changed_date` | `TIMESTAMP(0)` | Last data change time                  |
| `last_used`         | `TIMESTAMP(0)` | Last used timestamp                    |
| `last_used_days`    | `INT`          | Days since last use                    |
| `last_used_reset`   | `TIMESTAMP(0)` | Last reset time for last-used tracking |
| `Allocate_size`     | `BIGINT`       | Allocated size                         |
| `data_size`         | `BIGINT`       | Data size                              |
| `object_CCSID`      | `INT`          | CCSID of the object                    |
| `ASP_Nbr`           | `SMALLINT`     | Auxiliary storage pool number          |
| `OBJTEXT`           | `VARCHAR(50)`  | Object or member text                  |
| `LIBNAME`           | `VARCHAR(10)`  | Library name                           |
| `OBJNAME`           | `VARCHAR(10)`  | Object name                            |
| `OBJTYPE`           | `VARCHAR(10)`  | Object type                            |
| `MBRNAME`           | `VARCHAR(10)`  | Member name                            |
| `SRCTYPE`           | `VARCHAR(10)`  | Source member type                     |
| `ASPNAME`           | `VARCHAR(10)`  | ASP name                               |

---

## Example queries

### Resolve a source member from a /QSYS.LIB path

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.OBJ_ATTR(
      PATH_NAME => '/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE,JOB_ATTR.MBR'
    )
  ) AS T;
```

### Resolve by library and object name

```sql
SELECT LIBNAME, OBJNAME, OBJTYPE, MBRNAME, SRCTYPE, OBJTEXT
  FROM TABLE(
    SQLTOOLS.OBJ_ATTR(
      LIBRARY_NAME => 'SQLTOOLS',
      OBJECT_NAME  => 'QRPGLESRC',
      OBJTYPE      => '*FILE',
      MBR_NAME     => 'JOB_ATTR'
    )
  ) AS T;
```

---

## Implementation notes

This UDTF is implemented as an external RPG IV program using the IBM i object-attribute APIs and related path conversion routines. It supports both conventional library/object resolution and `/QSYS.LIB` path-based resolution, making it suitable for catalog-style inspection of objects and members from SQL.

The function is especially useful for tools that need to read member text, source type, object metadata, or library/object details without needing a custom CL or native API call from the calling program.
