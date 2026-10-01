# CMD_CHECK - CL Command Syntax Checker UDTF

Syntax-checks a CL command string and returns diagnostic messages.
Use this function when you want command validation without running the command.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CMD_CHECK`
**External program:** `SQLTOOLS/CMDCHECK`

---

## Source files

| File                           | Target member                | Purpose              |
| ------------------------------ | ---------------------------- | -------------------- |
| `/src/CMD_CHECK/CMD_CHECK.CPP` | `SQLTOOLS/QCSRC(CMDCHECK)`   | ILE C++ UDTF program |
| `/src/CMD_CHECK/CMD_CHECK.SQL` | `SQLTOOLS/QUDFSRC(CMDCHECK)` | SQL UDTF definition  |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.CMD_CHECK(
      CMD      => 'DLTF FILE(QTEMP/TESTPF)',
      CHECKOPT => '*CLLE'
    )
  ) AS T;
```

---

## Parameters

### CMD
**Type:** `VARCHAR(6000)`  **Required**

The CL command string to syntax-check.

### CHECKOPT
**Type:** `VARCHAR(14)`  **Default:** `*CLLE`

Controls syntax-check mode. Leading `*` is optional and case is ignored.

Common values:
- `*CLLE` or `*ILE` - ILE CL program source checking (default)
- `*CHECK` or `*CL` - command-entry style check
- `*LIMIT` - limited-user checking
- `*CLP` - OPM CL checking
- `*CMD` - command-definition statement checking
- `*BINDER` or `*BND` - binder source checking
- `*PDM` - PDM option command checking

---

## Result columns

| Column      | Type            | Description                         |
| ----------- | --------------- | ----------------------------------- |
| `MSGID`     | `VARCHAR(7)`    | Message ID returned during checking |
| `MSGSEV`    | `INT`           | Message severity                    |
| `MSGTEXT`   | `VARCHAR(512)`  | Message text                        |
| `CMDSTRING` | `VARCHAR(6000)` | Reserved output column              |

---

## Notes

- This UDTF checks syntax only; it does not execute the command.
- For execute/check combined behavior, see `CMD_RUN`.