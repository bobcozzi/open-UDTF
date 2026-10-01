# JOB_ATTR — Job Status Information UDTF

Returns one row of job status information for a target IBM i job. This UDTF calls the IBM-supplied `QUSRJOBI` API to return job attributes that are not normally exposed through the standard `QSYS2` catalog views.

It is useful for operational checks such as identifying the active job state, message queue, output queue, and printer device associated with a job. In the CLPROMPTER extension for VSCODE, we use this to determine if the host SQL Job is in MSGW status, its associated message queue, and the message key needed to send a reply.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.JOB_ATTR`
**External program:** `SQLTOOLS/JOB_ATTR`

---

## Source files

| File                           | Target member                  | Purpose                                       |
| ------------------------------ | ------------------------------ | --------------------------------------------- |
| `/src/JOB_ATTR/JOB_ATTR.RPGLE` | `SQLTOOLS/QRPGLESRC(JOB_ATTR)` | External RPG IV program that calls `QUSRJOBI` |
| `/src/JOB_ATTR/JOB_ATTR.SQL`   | `SQLTOOLS/QSQLSRC(JOB_ATTR)`   | SQL function declaration for the UDTF         |

---

## Compiling / Build

The SQL function definition is created with `RUNSQLSTM`, and the external RPG IV program is compiled as a normal IBM i program.

The project uses the standard RPG compile flow, typically with `CRTBNDRPG` or `CRTRPGMOD` + `CRTPGM` using `ACTGRP(*CALLER)`, which is required for SQL function entry points.

### Example build flow

```cl
CRTBNDRPG PGM(SQLTOOLS/JOB_ATTR) SRCFILE(SQLTOOLS/QRPGLESRC) SRCMBR(JOB_ATTR) DBGVIEW(*SOURCE)

RUNSQLSTM SRCFILE(SQLTOOLS/QSQLSRC) SRCMBR(JOB_ATTR)
```
> The DFTACTGRP and ACTGRP keywords are also embedded in the source CTL-OPT (header) keyword so they are unnecessary on the CRTBNDRPG command itself.
> The exact library and source-file names may vary based on where you store the members, but the build pattern is the same.

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.JOB_ATTR(
      JOB_NAME => '*'
    )
  ) AS T;
```

The input parameter accepts the target job in the format:

```text
JOB_NUMBER/JOB_USER/JOB_NAME
```

Examples:

```text
'*'                  -- current job
'123456/BOBCOZZI/QZDASOINIT'
'000123/USER1/JOB1'
```

If `JOB_NAME` is omitted or set to `'*'`, the function uses the current job context.

---

## Parameters

### JOB_NAME
**Type:** `VARCHAR(28)`  **Default:** `'*'`

The fully qualified job identifier for the target job.

| Value                 | Meaning                           |
| --------------------- | --------------------------------- |
| `'*'` *(default)*     | Current job                       |
| `nnnnnn/user/jobname` | Specific job in the target system |

The RPG code parses the value by splitting it at the `/` characters and populates `JOB_NBR`, `JOB_USER`, and `JOB_NAME` accordingly.

---

## Result columns

The function returns a single row of job metadata.

| Column              | Type          | Description                                               |
| ------------------- | ------------- | --------------------------------------------------------- |
| `JOB`               | `VARCHAR(28)` | Full qualified job name in the form `nnnnnn/user/jobname` |
| `JOB_NAME`          | `VARCHAR(10)` | Job name                                                  |
| `JOB_USER`          | `VARCHAR(10)` | Job user profile                                          |
| `JOB_NBR`           | `VARCHAR(6)`  | Job number                                                |
| `JOB_DATE`          | `DATE`        | Date the job was started or assigned                      |
| `JOB_STATUS`        | `VARCHAR(10)` | Current job status                                        |
| `JOB_TYPE`          | `VARCHAR(1)`  | Job type code                                             |
| `JOB_SUBTYPE`       | `VARCHAR(1)`  | Job subtype                                               |
| `SUBSYSTEM_NAME`    | `VARCHAR(10)` | Subsystem name                                            |
| `LAST_FUNCTION`     | `VARCHAR(14)` | Last function or current function in progress             |
| `ACTIVE_JOB_STATUS` | `VARCHAR(4)`  | Active job status code                                    |
| `RUNPTY`            | `INT`         | Run priority                                              |
| `POOL_NAME`         | `VARCHAR(10)` | Memory pool name                                          |
| `POOL_ID`           | `INT`         | System pool identifier                                    |
| `REPLY`             | `VARCHAR(1)`  | Message reply indicator                                   |
| `MSGKEY_HEX`        | `VARCHAR(8)`  | Message key in hexadecimal form                           |
| `MSGKEY`            | `BINARY(4)`   | Native message key value                                  |
| `MSGQ_NAME`         | `VARCHAR(10)` | Message queue name                                        |
| `MSGQ_LIB`          | `VARCHAR(10)` | Message queue library                                     |
| `MSGQ_LIB_ASP`      | `VARCHAR(10)` | Message queue library ASP                                 |
| `OUTQ_NAME`         | `VARCHAR(10)` | Output queue name                                         |
| `OUTQ_LIB`          | `VARCHAR(10)` | Output queue library                                      |
| `OUTQ_PTY`          | `VARCHAR(2)`  | Output queue priority                                     |
| `PRTDEV_NAME`       | `VARCHAR(10)` | Printer device name                                       |

---

## Example queries

### Current job

```sql
SELECT *
  FROM TABLE(SQLTOOLS.JOB_ATTR('*')) AS J;
```

### Specific job

```sql
SELECT JOB_NAME, JOB_USER, JOB_NBR, JOB_STATUS, ACTIVE_JOB_STATUS,
       MSGQ_NAME, MSGQ_LIB, OUTQ_NAME, OUTQ_LIB
  FROM TABLE(
    SQLTOOLS.JOB_ATTR('123456/BOBCOZZI/QZDASOINIT')
  ) AS J;
```

### Job routing and output queue details

```sql
SELECT JOB, JOB_NAME, JOB_STATUS, SUBSYSTEM_NAME, LAST_FUNCTION,
       OUTQ_NAME, OUTQ_LIB, PRTDEV_NAME
  FROM TABLE(SQLTOOLS.JOB_ATTR('*')) AS J;
```

---

## Implementation notes

This UDTF is implemented as an external RPG IV program rather than a C++ or SQL-only UDTF. It wraps the `QUSRJOBI` API and maps the returned `JOBI0200` and `JOBI0300` structures into SQL result columns.

Notable behaviors:

- It supports a default target of the current job.
- It normalizes the input into `JOB_NBR`, `JOB_USER`, and `JOB_NAME` using a simple slash-splitting routine.
- It converts message key bytes to a readable hexadecimal string (`MSGKEY_HEX`).
- It returns selected job queue and printer output information when available.

This makes it a practical UDTF for operational SQL reporting and support scripts on IBM i.

---
