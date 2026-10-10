# ADD_MBR — Add a Source Member to a Source File

Adds a member to an IBM i source physical file using a `/QSYS.LIB`-style path name. The function can create or update the member, optionally setting the source type and member text, and return the resulting member metadata.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.ADD_MBR`
**External program:** `SQLTOOLS/ADD_MBR`

---

## Source files

| File                         | Target member                 | Purpose                                       |
| ---------------------------- | ----------------------------- | --------------------------------------------- |
| `/src/ADD_MBR/ADD_MBR.RPGLE` | `SQLTOOLS/QRPGLESRC(ADD_MBR)` | External RPG IV program implementing the UDTF |
| `/src/ADD_MBR/ADD_MBR.SQL`   | `SQLTOOLS/QUDFSRC(ADD_MBR)`   | SQL function declaration for the UDTF         |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.ADD_MBR(
      PATH_NAME => '/QSYS.LIB/DEVSRC.LIB/QRPGLESRC.FILE/NEWMBR.MBR',
      SRCTYPE   => 'RPGLE',
      MBR_TEXT  => 'New RPG member'
    )
  ) AS T;
```

or

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.ADD_MBR(
      PATH_NAME => '/QSYS.LIB/DEVSRC.LIB/QCLSRC.FILE/EODRUN.CLLE',
      MBR_TEXT  => 'End-of-Day rouinte'
    )
  ) AS T;
```

If `SRCTYPE` is omitted, the suffix from `PATH_NAME` can be used as the source type when it is a valid member type such as `.RPGLE`, `.SQL`, or `.CLLE`.

---

## Parameters

### PATH_NAME
**Type:** `VARCHAR(4096)`  **Default:** `NULL`

The fully qualified `/QSYS.LIB` path to the source file member to create or update.

Examples:

```text
/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE/NEWMBR.MBR
/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE/NEWMBR.RPGLE
```

If the path name does not end in `.MBR`, the suffix is treated as a candidate source type when `SRCTYPE` is not provided.

---

### SRCTYPE
**Type:** `VARCHAR(10)`  **Default:** `NULL`

The source type to assign to the member, such as `SQL`, `RPGLE`, `CLLE`, `CMD`, or another valid IBM i source type.

When `SRCTYPE` is not supplied and the suffix on `PATH_NAME` is a valid source type, the suffix is used automatically.

---

### MBR_TEXT
**Type:** `VARCHAR(50)`  **Default:** `NULL`

The member text description assigned when the member is created or updated.

---

## Result columns

The function returns one row with the member metadata after processing.

| Column    | Type          | Description                        |
| --------- | ------------- | ---------------------------------- |
| `LIBNAME` | `VARCHAR(10)` | Library containing the source file |
| `OBJNAME` | `VARCHAR(10)` | Source file name                   |
| `OBJTYPE` | `VARCHAR(10)` | Object type of the file            |
| `MBRNAME` | `VARCHAR(10)` | Member name                        |
| `SRCTYPE` | `VARCHAR(10)` | Source type assigned to the member |
| `OBJTEXT` | `VARCHAR(50)` | Member text description            |
| `ASPNAME` | `VARCHAR(10)` | ASP name (if applicable)           |

---

## Example queries

### Create a new RPG member

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.ADD_MBR(
      PATH_NAME => '/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE/NEWPGM.RPGLE',
      SRCTYPE   => 'RPGLE',
      MBR_TEXT  => 'New RPG source member'
    )
  ) AS T;
```

### Infer source type from the path suffix

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.ADD_MBR(
      PATH_NAME => '/QSYS.LIB/SQLTOOLS.LIB/QRPGLESRC.FILE/NEWPGM.SQL',
      MBR_TEXT  => 'SQL source member'
    )
  ) AS T;
```

---

## Implementation notes

This UDTF is implemented as an external RPG IV program that uses the IBM i `Qp0lSetAttr` and related APIs to ensure the member exists and to update its text and source type. It works with `/QSYS.LIB` naming and can also infer the member type from the path suffix when the source type parameter is omitted.

It is intended for situations where the client wants to add or update a source member as part of a SQL-driven tooling workflow, especially from VS Code or other SQL-access environments.
