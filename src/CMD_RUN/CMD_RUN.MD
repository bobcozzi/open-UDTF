# CMD_RUN - CL Command Runner and Checker UDTF

Runs or syntax-checks a CL command and returns resulting messages.
When run mode is used and no messages are generated, the result set can be empty.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CMD_RUN`
**External program:** `SQLTOOLS/CMDRUN`

---

## Source files

| File                       | Target member              | Purpose              |
| -------------------------- | -------------------------- | -------------------- |
| `/src/CMD_RUN/CMD_RUN.CPP` | `SQLTOOLS/QCSRC(CMDRUN)`   | ILE C++ UDTF program |
| `/src/CMD_RUN/CMD_RUN.SQL` | `SQLTOOLS/QUDFSRC(CMDRUN)` | SQL UDTF definition  |

---

## Signature

```sql
SELECT *
  FROM TABLE(
    SQLTOOLS.CMD_RUN(
      CMD  => 'DSPLIBL',
      MODE => '*RUN'
    )
  ) AS T;
```

---

## Parameters

### CMD
**Type:** `VARCHAR(32700)`  **Required**

CL command string to run or check.

### MODE
**Type:** `VARCHAR(14)`  **Default:** `*RUN`

Controls behavior:
- `*RUN` - syntax-check then run
- `*LIMIT` or `*RUNLIMIT` - syntax-check then run as limited user
- `*CHECK` - syntax-check only
- `*CHECKLIMIT` - limited-user syntax-check only

Leading `*` is optional and case is ignored.

---

## Result columns

| Column             | Type            | Description               |
| ------------------ | --------------- | ------------------------- |
| `ORDINAL_POSITION` | `SMALLINT`      | Output message order      |
| `MSGID`            | `VARCHAR(7)`    | Message ID                |
| `MSGSEV`           | `INT`           | Message severity          |
| `MSGTYPE`          | `VARCHAR(10)`   | Message type              |
| `SENT_TIMESTAMP`   | `TIMESTAMP(6)`  | Message timestamp         |
| `MSGTEXT`          | `VARCHAR(2000)` | First-level message text  |
| `SENT_BY_USER`     | `VARCHAR(10)`   | Sending user              |
| `SENT_FROM_PGM`    | `VARCHAR(10)`   | Sender program            |
| `SENT_FROM_STMT`   | `CHAR(4)`       | Sender statement          |
| `SENT_FROM_MOD`    | `VARCHAR(10)`   | Sender module             |
| `SENT_FROM_PROC`   | `VARCHAR(255)`  | Sender procedure          |
| `SENT_TO_PGM`      | `VARCHAR(10)`   | Target program            |
| `SENT_TO_STMT`     | `CHAR(4)`       | Target statement          |
| `SENT_TO_MOD`      | `VARCHAR(10)`   | Target module             |
| `SENT_TO_PROC`     | `VARCHAR(255)`  | Target procedure          |
| `SECLVLMSG`        | `VARCHAR(4000)` | Second-level message text |

---

## Notes

- Use `CMD_CHECK` when you only need syntax-check diagnostics.
- This function is useful for automation where CL command feedback must be queryable.