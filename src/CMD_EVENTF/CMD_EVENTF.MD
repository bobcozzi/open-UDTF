# CMD_EVENTF - Event File Message Reader UDTF

Reads CL command event-file diagnostics and returns them as a relational result set.
This is useful when compile/check output is written to an event file member.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CMD_EVENTF`
**External program:** `SQLTOOLS/CMD_EVENTF`

---

## Source files

| File                             | Target member                  | Purpose              |
| -------------------------------- | ------------------------------ | -------------------- |
| `/src/CMD_EVENTF/CMD_EVENTF.CPP` | `SQLTOOLS/QCSRC(CMD_EVENTF)`   | ILE C++ UDTF program |
| `/src/CMD_EVENTF/CMD_EVENTF.SQL` | `SQLTOOLS/QUDFSRC(CMD_EVENTF)` | SQL UDTF definition  |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.CMD_EVENTF(
      LIBRARY_NAME => '*LIBL',
      EVENTF_NAME  => 'EVENTF',
      MBR_NAME     => 'QCLSRC'
    )
  ) AS T;
```

---

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(10)`  **Default:** `*LIBL`

Library containing the source/event file.

### EVENTF_NAME
**Type:** `VARCHAR(10)`  **Default:** `EVENTF`

Event file object name.

### MBR_NAME
**Type:** `VARCHAR(10)`  **Required**

Event file member name to read.

---

## Result columns

| Column             | Type            | Description            |
| ------------------ | --------------- | ---------------------- |
| `EVENT_TIMESTAMP`  | `CHAR(14)`      | Event record timestamp |
| `SOURCE_ID`        | `INTEGER`       | Source identifier      |
| `SOURCE_FILE`      | `VARCHAR(256)`  | Source file path/name  |
| `SOURCE_TIMESTAMP` | `CHAR(14)`      | Source timestamp       |
| `SOURCE_FLAG`      | `INTEGER`       | Source flag/status     |
| `STMT_LINE`        | `INTEGER`       | Statement line number  |
| `START_LINE`       | `INTEGER`       | Start line in source   |
| `START_COLUMN`     | `INTEGER`       | Start column in source |
| `END_LINE`         | `INTEGER`       | End line in source     |
| `END_COLUMN`       | `INTEGER`       | End column in source   |
| `MESSAGE_ID`       | `CHAR(7)`       | Message ID             |
| `SEVERITY_CHAR`    | `CHAR(1)`       | Severity class         |
| `MSGSEV`           | `INTEGER`       | Numeric severity       |
| `MSGTEXT`          | `VARCHAR(2048)` | Message text           |

---

## Notes

- Returns one row per event/diagnostic message found in the member.
- Best used with command-check or compile pipelines that write EVENTF output.
