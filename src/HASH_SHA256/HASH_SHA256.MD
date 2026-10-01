# HASH_SHA256 - SHA-256 Hash Functions

Provides SHA-256 helper functions for CLOB input values.
Both functions return binary hash output.

**Schema:** `SQLTOOLS`
**Specific names:**
- `SQLTOOLS.HASH_SHA256`
- `SQLTOOLS.HASH_SHA256_UTF8`
**External program:** `SQLTOOLS/HASHSHA256`

---

## Source files

| File                              | Target member                  | Purpose                           |
| --------------------------------- | ------------------------------ | --------------------------------- |
| `/src/HASH_SHA256/HASHSHA256.CPP` | `SQLTOOLS/QCSRC(HASHSHA256)`   | ILE C++ external function program |
| `/src/HASH_SHA256/HASHSHA256.SQL` | `SQLTOOLS/QUDFSRC(HASHSHA256)` | SQL function definitions          |

---

## Functions

### HASH_SHA256

```sql
VALUES HEX(SQLTOOLS.HASH_SHA256('hello world'));
```

Signature:

```sql
SQLTOOLS.HASH_SHA256(input_data CLOB(2G)) RETURNS VARBINARY(64)
```

### HASH_SHA256_UTF8

```sql
VALUES HEX(SQLTOOLS.HASH_SHA256_UTF8(CAST('hello world' AS CLOB(2G) CCSID 1208)));
```

Signature:

```sql
SQLTOOLS.HASH_SHA256_UTF8(input_data CLOB(2G) CCSID 1208) RETURNS VARBINARY(64)
```

---

## Notes

- SHA-256 digest size is 32 bytes; output type is `VARBINARY(64)`.
- Use `HEX(...)` when you want printable hexadecimal output.
- `HASH_SHA256_UTF8` ensures UTF-8 input semantics before hashing.
