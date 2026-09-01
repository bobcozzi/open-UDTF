
 -- SPDX-License-Identifier: Apache-2.0
 -- Copyright (c) 1996-2026 by R. Cozzi, Jr.

 -- @author BobCozzi

--
-- Source origin: /Users/cozzi/Downloads/projects/open-UDTF/src/HASH_SHA
--

 CREATE or REPLACE FUNCTION sqlTools.hash_SHA256(
                                     input_data CLOB(2G)
                                )
               RETURNS VARBINARY(64)
    LANGUAGE C++
    NO SQL
    NO EXTERNAL ACTION
    NO FINAL CALL
    NOT FENCED
    DETERMINISTIC
    SPECIFIC sqlTools.hash_sha256
    EXTERNAL NAME 'SQLTOOLS/HASHSHA256'
    PARAMETER STYLE DB2SQL;



LABEL on specific routine sqltools.hash_SHA256 IS
'Generate SHA-256 HASH for input data';

comment on specific function sqltools.hash_SHA256 IS
'Returns a SHA-256 hash for the input data. The result is a
VARBINARY value with the length set to the size of the hash (32-bytes).';

comment on parameter specific function sqltools.hash_SHA256
( input_data is 'The data used to produce the SHA-256 hash result.
  The input data is used to calculate the SHA-256 hash.'
);


 CREATE or REPLACE FUNCTION sqlTools.hash_SHA256_UTF8(
                                     input_data CLOB(2G) CCSID 1208
                                                      )
               RETURNS VARBINARY(64)
    LANGUAGE C++
    NO SQL
    NO EXTERNAL ACTION
    NO FINAL CALL
    NOT FENCED
    DETERMINISTIC
    SPECIFIC sqlTools.hash_SHA256_UTF8
    EXTERNAL NAME 'SQLTOOLS/HASHSHA256'
    PARAMETER STYLE DB2SQL;


LABEL on specific routine sqltools.hash_SHA256_UTF8 IS
'Generate SHA-256 HASH for the UTF-8 input data';

comment on specific function sqltools.hash_SHA256_UTF8 IS
'Returns a SHA-256 hash for the input data. The input data
is cast to UTF-8 and then the hash is calculated. The result is a
VARBINARY value with the length set to the size of the hash (32-bytes).';

comment on parameter specific function sqltools.hash_SHA256_UTF8
( input_data is 'The data used to produce the SHA-256 hash result.
  The input is defined as a CLOB(2G) with CCSID(1208). This causes the
  input data to be converted to UTF-8 (ASCII) before calculating the
  hash result.'
);
