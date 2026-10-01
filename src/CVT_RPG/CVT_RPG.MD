# CVT_RPG - RPG III to RPG IV Conversion Function

Wraps the IBM i `CVTRPGSRC` command and returns a status code.
Use this function to convert RPG III source members into RPG IV source.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CVT_RPG`
**Implementation:** SQL function (`LANGUAGE SQL`)

---

## Source files

| File                       | Target member               | Purpose                           |
| -------------------------- | --------------------------- | --------------------------------- |
| `/src/CVT_RPG/CVT_RPG.SQL` | `SQLTOOLS/QUDFSRC(CVT_RPG)` | SQL function definition and logic |

---

## Signature

```sql
VALUES SQLTOOLS.CVT_RPG(
  rpg3SrcLib  => '*LIBL',
  rpg3SrcFile => 'QRPGSRC',
  rpg3SrcMbr  => 'OLDMBR',
  rpg4SrcLib  => '*LIBL',
  rpg4SrcFile => 'QRPGLESRC',
  rpg4SrcMbr  => '*FROMMBR'
);
```

---

## Parameters

- `rpg3SrcLib`, `rpg3SrcFile`, `rpg3SrcMbr`: input RPG III source location
- `rpg4SrcLib`, `rpg4SrcFile`, `rpg4SrcMbr`: output RPG IV source location
- `EXPCOPY`, `OUTPUT`, `SECLVL`, `RULER`: conversion options
- `LogLib`, `LogFile`, `LogMbr`: optional conversion report destination

The function normalizes most option values to valid `CVTRPGSRC` command options.

---

## Return value

Returns `INT` status from command execution:
- `1` typically indicates command success
- `0` indicates no-success/failed status

Use job log messages for detailed diagnostics when conversion fails.
