# open-UDTF

Open-source IBM i SQL User-Defined Table Functions (UDTFs)
by Robert Cozzi Jr.

I am releasing some of my SQLTOOLS SQL Table functions as open source. However they are being created as stand-alone UDTFs without any dependencies on the SQLTOOLS *SRVPGM and helper functions in the main product. The SQLTOOLS shall continue to be available in compiled form, for free on my SQLTOOLS repo.
The first function being released is SPOOLED_DATA. It reads SPOOLED files and returns their results. It reads directly into the SPOOLED FILE object; it does not copy the SPOOLED FILE to a database file. This function interprets the SCS data stream and passes DBCS data transparently through the resulting columns. There is also a 2nd column SPOOLED_DATA_BIN that contains the non-textualized SPOOLED FILE records/line data, which in most cases can be ignored. It is functionally equivalent to the SQL Tools READSPLF table function.

Each function's source lives under `/src/<function_name>/` and is self-contained. Shared components, such as the COZUTILS.H header file, are in the `/src/shared/h` folder and should be copied to the target IBM i source member in the same library where the other source code is uploaded. That is: `/src/shared/h/` currently contains `COZUTILS.H` and it should be uploaded to `SQLTOOLS/H(COZUTILS)`.

---

## Functions

### SPOOLED_DATA

Reads a spooled file (*SCS or AFP) and returns each print
line as a row.  A drop-in, standalone replacement for the
classic `READSPLF` UDTF.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.SPOOL_DATA`
**External program name:** `SQLTOOLS/SPOOLDATA`

#### Source files

| Folder | File | Target Src | Purpose |
|--------|------|---------|---------|
| `/src/shared/h/` | `COZUTILS.H` | SQLTOOLS/H(COZUTILS) | Standalone utility header |
| `/src/SPOOLED_DATA/` | `SPOOLDATA.CPP` | SQLTOOLS/QCSRC(SPOOLDATA) | ILE C++ UDTF program |
| `/src/SPOOLED_DATA/` | `SPOOLDATA.SQL` | SQLTOOLS/QUDFSRC(SPOOLDATA) | SQL UDTF source |

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

Using the BLDUDTF helper function provided, compile/build the SPOOL_DATA UDTF using the following CL command.
```clle
BLDUDTF UDTF(SPOOLDATA)
        EXTPGM(SPOOLDATA)
        EXTPGMSRC(SQLTOOLSRC/QCSRC)
        UDTFSRC(SQLTOOLSRC/QUDFSRC)
        OBJLIB(SQLTOOLS)
        DBGVIEW(*NONE)
        DROP(*NO)
```
Several assumptions are included in the above BLDUDTF command:
- The target (object) library is SQLTOOLS (SQLTOOLS is the recommended target library)
- The source code is in library SQLTOOLSRC (although it can be any library)
- No Debug info is stored. Use DBGVIEW(*SOURCE) if you need to debug.
- The DROP parameter is *NO since most of the UDTFs use `CREATE or REPLACE FUNCTION` and therefore do not strictly need to be dropped ahead of time. If you have issues building, specify `DROP(*YES)` and try the build again.
---

## License

Copyright (c) 1996-2026 Robert Cozzi Jr.
Licensed under the Apache License, Version 2.0 (Apache-2.0).
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
See [LICENSE](LICENSE) for the full license text and terms.
