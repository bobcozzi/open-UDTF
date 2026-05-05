# open-UDTF

Open-source IBM i SQL User-Defined Table Functions (UDTFs)
by Robert Cozzi Jr.

Each function lives under `src/<function_name>/` and is
self-contained — no shared service programs are required.

---

## Functions

### SPOOLED_DATA

Reads a spooled file (*SCS or AFP) and returns each print
line as a row.  A drop-in, standalone replacement for the
classic `READSPLF` UDTF.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.ST_SPOOLDATA`
**External module:** `SQLTOOLS/ST_SPOOLDATA`

#### Source files

| Folder | File | Purpose |
|--------|------|---------|
| `src/spooled_data/c/` | `SPOOLDATA.CPP` | ILE C++ UDTF entry point |
| `src/spooled_data/h/` | `SPLUTILS.H` | Standalone utility header |
| `src/spooled_data/sql/` | `SPOOLDATA.SQL` | `CREATE FUNCTION` DDL + COMMENTs |

#### Signature

```sql
SELECT *
  FROM TABLE(
    sqltools.SPOOLED_DATA(
        SPLFNAME    => '*LAST',
        SPLNBR      => '*LAST',
        JOB_NAME    => '*',
        BLANK_LINES => '*DROP',
        EOF         => '*IGNORE',
        START_PAGE  => '1',
        START_LINE  => 1,
        OPTIONS     => NULL
    )
  ) AS t;
```

#### Result columns

| Column | Type | Description |
|--------|------|-------------|
| `SPLFNAME` | VARCHAR(10) | Spooled file name |
| `SPLNBR` | INT | Spooled file number |
| `ORDINAL_POSITION` | INT | Sequential row counter |
| `PAGENBR` | INT | Page number |
| `LINENBR` | INT | Line number on page |
| `SPOOLED_DATA` | VARCHAR(2048) FOR MIXED DATA | Print line text |
| `SPOOLED_DATA_BIN` | VARCHAR(2048) FOR BIT DATA | Raw binary copy |
| `PAGE_COUNT` | INT | Total pages in spooled file |
| `PAGE_LENGTH` | SMALLINT | Lines per page |
| `PAGE_WIDTH` | SMALLINT | Characters per line |
| `CRTSYSNAME` | VARCHAR(8) | System where job ran |
| `ORGSYSNAME` | VARCHAR(8) | System where spooled file originated |
| `JOB` | VARCHAR(28) | Qualified job name (nbr/user/name) |
| `INT_SPLID` | BINARY(16) | Internal spooled-file ID |
| `INT_JOBID` | BINARY(16) | Internal job ID |

#### Key OPTIONS values

| Option | Effect |
|--------|--------|
| `*UPPER` | Convert output to upper case |
| `*LOWER` | Convert output to lower case |
| `*LEFT` | Left-adjust (trim leading blanks) |
| `*TRIMRIGHT` | Trim trailing blanks |
| `*RULER` | Insert a column-position ruler row |
| `*KEEP` | Retain blank lines in result set |
| `*DELETE` | Delete the spooled file after reading |

#### Build

```
CRTCPPMOD SRCFILE(SQLTOOLSRC/QCSRC) SRCMBR(SPOOLDATA) +
          MODULE(SQLTOOLS/ST_SPOOLDATA) +
          INCDIR('/') OPTIMIZE(40)

CRTSRVPGM or CRTPGM as needed, then run SPOOLDATA.SQL
```

---

## License

Copyright (c) 1992-2025 Robert Cozzi Jr.
Released under the MIT License — see [LICENSE](LICENSE).
