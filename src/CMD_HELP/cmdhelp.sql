/**
 * Embedded source for CMDHELP.CPP — the C++ service program that backs the
 * CMD_HELP UDTF.  Stored here so the extension can upload and compile it on
 * first connection without requiring a separate file distribution.
 *
 * Source origin: /Users/cozzi/Downloads/projects/open-UDTF/src/CMD_HELP/CMDHELP.CPP
 * To update: edit that file, then ask Copilot to re-encode it into this file.
 *
 * C++ source is base64-encoded to prevent false-positive antivirus hits
 * in the VS Code Marketplace. Decoded at runtime before upload to IBM i.
 */

 -- Retrieve HTML for a CL command's helptext
 -- (c) Copyright 2026 by R. Cozzi, Jr.
 -- @author BobCozzi

 CREATE or REPLACE FUNCTION sqltools.cmd_help(
                               library_name varchar(10) DEFAULT '*LIBL',
                               cmd_name     varchar(10),
                               helpid       varchar(6000) DEFAULT '*CMD'
                                              )
    RETURNS table (
            HELP_XML CLOB(16M) CCSID 1208
          )

     LANGUAGE C++
     NO SQL
     EXTERNAL ACTION
     NO FINAL CALL
     STATEMENT DETERMINISTIC
     NOT FENCED
     CARDINALITY 1
     SCRATCHPAD 256
     SPECIFIC sqlTools.cmd_help
     EXTERNAL NAME 'SQLTOOLS/CMDHELP'
     PARAMETER STYLE DB2SQL;

LABEL on specific routine sqltools.cmd_help  IS
'Retrieve helptext for a CL Command';

comment on specific function sqltools.cmd_help is
 'Returns the XML produced by the QUHRHLPT API for the Helptext Panel Group
 of the specified CL command. This UDTF does not use Java therefore the
 JVM startup overhead is avoided.';

comment on parameter specific function sqltools.cmd_help
(LIBRARY_NAME IS 'The name of the library where the *CMD object specified
on the CMD_NAME parameter is located. The special values *LIBL and *CURLIB
are supported. The default is *LIBL',

CMD_NAME IS 'The name of the CL command whose helptext is to be retrieved.
 Upper/lower case is ignored.',

HELPID IS 'A comma separated list of help ID whose helptext is to be
returned. This is normally a list of the command''s parameter keywords.'
);

