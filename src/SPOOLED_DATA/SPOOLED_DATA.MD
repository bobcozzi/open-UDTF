# SPOOLED_DATA - Read Spool File Text UDTF

Reads spool file content directly and returns one row per line.
No intermediate CPYSPLF database file is required.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.SPOOLED_DATA`
**External program:** `SQLTOOLS/SPOOL_DATA`

---

## Source files

| File                               | Target member                  | Purpose              |
| ---------------------------------- | ------------------------------ | -------------------- |
| `/src/SPOOLED_DATA/SPOOL_DATA.CPP` | `SQLTOOLS/QCSRC(SPOOL_DATA)`   | ILE C++ UDTF program |
| `/src/SPOOLED_DATA/SPOOL_DATA.SQL` | `SQLTOOLS/QUDFSRC(SPOOL_DATA)` | SQL UDTF definition  |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.SPOOLED_DATA(
      SPLFNAME => '*LAST',
      SPLNBR   => '*LAST',
      JOB_NAME => '*'
    )
  ) AS T;
```

---

## Key parameters

- `SPLFNAME`, `SPLNBR`, `JOB_NAME`: identify spool file
- `BLANK_LINES`: include or drop blank lines (`*DROP` default)
- `EOF`: wait or stop for open spool files
- `START_PAGE`, `START_LINE`: starting position
- `INT_JOBID`, `INT_SPLFID`: internal IDs that override name-based lookup
- `OPTIONS`: text transforms and behavior flags (for example `*UPPER`, `*LOWER`, `*KEEP`, `*DLTSPLF`)
- `SPOOLED_FILE_NAME`, `SPOOLED_FILE_NUMBER`: compatibility aliases

---

## Result columns

| Column             | Type                           | Description                |
| ------------------ | ------------------------------ | -------------------------- |
| `SPLFNAME`         | `VARCHAR(10)`                  | Spool file name            |
| `SPLNBR`           | `INT`                          | Spool file number          |
| `ORDINAL_POSITION` | `INT`                          | Row sequence               |
| `PAGENBR`          | `INT`                          | Page number                |
| `LINENBR`          | `INT`                          | Line number on page        |
| `SPOOLED_DATA`     | `VARCHAR(2048) FOR MIXED DATA` | Line text                  |
| `PAGE_COUNT`       | `INT`                          | Total page count           |
| `PAGE_LENGTH`      | `SMALLINT`                     | Lines per page             |
| `PAGE_WIDTH`       | `SMALLINT`                     | Characters per line        |
| `CRTSYSNAME`       | `VARCHAR(8)`                   | Creating system name       |
| `ORGSYSNAME`       | `VARCHAR(8)`                   | Origin system name         |
| `JOB`              | `VARCHAR(28)`                  | Qualified job name         |
| `SPOOLED_DATA_BIN` | `VARCHAR(2048) FOR BIT DATA`   | Binary-preserved line data |

---

## Notes

- Designed as a standalone UDTF implementation.
- Useful for SQL-based log/report parsing and automation.