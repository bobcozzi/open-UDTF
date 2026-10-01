# LAST_SPLF - Last Spool File UDTF

Returns identity information for the most recently created spool file.
You can scope lookup to the current job or the current user.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.LAST_SPLF`
**External program:** `SQLTOOLS/LASTSPLF`

---

## Source files

| File                           | Target member                | Purpose              |
| ------------------------------ | ---------------------------- | -------------------- |
| `/src/LAST_SPLF/LAST_SPLF.CPP` | `SQLTOOLS/QCSRC(LASTSPLF)`   | ILE C++ UDTF program |
| `/src/LAST_SPLF/LAST_SPLF.SQL` | `SQLTOOLS/QUDFSRC(LASTSPLF)` | SQL UDTF definition  |

---

## Signature

```sql
SELECT *
  FROM TABLE(SQLTOOLS.LAST_SPLF(SCOPE => 'JOB')) AS T;
```

---

## Parameters

### SCOPE
**Type:** `VARCHAR(10)`  **Default:** `JOB`

Controls lookup scope:
- `*JOB` (or `JOB`) - most recent spool file in the current job
- `*USER` (or `USER`) - most recent spool file created by the user

Leading `*` is optional and case is ignored.

---

## Result columns

| Column               | Type           | Description           |
| -------------------- | -------------- | --------------------- |
| `SPLFNAME`           | `VARCHAR(10)`  | Spool file name       |
| `SPLNBR`             | `INT`          | Spool file number     |
| `JOB`                | `VARCHAR(28)`  | Qualified job ID      |
| `JOBNAME`            | `VARCHAR(10)`  | Job name              |
| `JOBUSER`            | `VARCHAR(10)`  | Job user              |
| `JOBNBR`             | `VARCHAR(6)`   | Job number            |
| `SYSNAME`            | `VARCHAR(8)`   | System/partition name |
| `CREATION_TIMESTAMP` | `TIMESTAMP(0)` | Creation timestamp    |
