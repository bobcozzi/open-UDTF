# CHECK_AUT — Check User Authority to Object

Checks whether a user profile has the specified authority to an IBM i object (such as a file, program, or *AUTL). Returns 1 if the user is authorized, 0 if unauthorized or an error occurred.

**Schema:** `SQLTOOLS`
**Specific name:** `SQLTOOLS.CHECK_AUT`
**External program:** `SQLTOOLS/CHECKAUT`

---

## Source files

| File | Target member | Purpose |
|------|---------------|---------|
| `/src/CHECK_AUT/checkaut.cpp` | `SQLTOOLS/QCSRC(CHECKAUT)` | ILE C++ function |
| `/src/CHECK_AUT/checkaut.sql` | `SQLTOOLS/QUDFSRC(CHECKAUT)` | SQL function definition |

---

## Signature

```sql
SELECT SQLTOOLS.CHECK_AUT(
    LIBRARY_NAME => '*LIBL',
    OBJECT_NAME  => 'MYFILE',  -- REQUIRED PARAMETER
    OBJTYPE      => '*FILE',
    AUTH         => '*USE',
    USER_NAME    => '*CURRENT',
    CALLLVL      => '*SAME'
) AS AUTHORIZED;
```

---

## Function
```sql
values sqltools.check_aut(object_name=>'IQUERY',objtype=>'lib', auth=>'change');
```

## Parameters

### LIBRARY_NAME
**Type:** `VARCHAR(10)`  **Default:** `*LIBL`

The name of the library where the object specified on `OBJECT_NAME` is located.
The special values `*LIBL` and `*CURLIB` are supported. The default is `*LIBL`.
Upper/lower case is ignored.

| Value | Meaning |
|-------|---------|
| `*LIBL` *(default)* | Job library list |
| `*CURLIB` | Job's current library |
| *library-name* | Specific library |

---

### OBJECT_NAME
**Type:** `VARCHAR(10)`  **REQUIRED**

The object name being checked for the user's authority. Upper/lower case is ignored. This is a required parameter.

---

### OBJTYPE
**Type:** `VARCHAR(10)`  **Default:** `*FILE`

The IBM i object type of the object specified on `OBJECT_NAME`. Upper/lower case is ignored, and the leading asterisk is optional.

---

### AUTH
**Type:** `VARCHAR(96)`  **Default:** `*USE`

The authorities to check. One or more authorities may be specified (see below). Each authority must be separated by one or more blanks or commas. Upper/lower case is ignored and the leading asterisk is optional.

Valid authorities:
- **`*USE`** — *OBJOPR, *READ, and *EXECUTE authority (this is the default)
- `*ALL` — All authority
- `*CHANGE` — *OBJOPR, *OBJREF, *OBJALTER, *READ, *ADD, *DLT, *UPD
- `*EXCLUDE` — Exclude authority
- `*AUTLMGT` — For *AUTL objects, Authorization List management authority
- `*OBJALTER` — Object alter authority
- `*OBJOPER` — Object operational authority
- `*OBJMGT` — Object management authority
- `*OBJEXIST` — Object existence authority
- `*OBJREF` — Object reference authority
- `*READ` — Read authority
- `*ADD` or `*WRITE` — Add authority
- `*UPD` or `*UPDATE` — Update authority
- `*DLT` or `*DELETE` — Delete authority
- `*EXECUTE` or `*RUN` — Run a program or find an object in a library

---

### USER_NAME
**Type:** `VARCHAR(10)`  **Default:** `*CURRENT`

The user profile whose authority to the object is checked. Upper/lower case is ignored.

---

### CALLLVL
**Type:** `VARCHAR(10)`  **Default:** `*SAME`

The call level for the authority check. Upper/lower case is ignored.
The valid choices are:
- `*SAME` - The call level isn't used to check authority.
- `*PRV` - The previous entry in the call stack is used to check author (same as `CALLLVL=>'1'`)
- `0` to `9` - The call level to use to check the authority.
It is recommended to avoid the call level (CALLLVL) parameter unless you are sure you want the function to check for adopted authority.

---

## Return value

Returns `1` if the user is authorized, `0` if not. When `0` is returned, check the joblog for any CPF message(s) that may have been generated, such as CPF9801 Object Not Found.

---

## Description

Checks a user profile's authority to the specified object. The authorization is determined if the user has the rights to the object specified on the `AUTH` parameter.
