
-- SPDX-License-Identifier: Apache-2.0
-- Copyright (c) 1996-2026 by R. Cozzi, Jr.
-- @author BobCozzi

   -- SQL Tools UDF - (DLTOBJ) Delete Object

CREATE or REPLACE FUNCTION sqltools.DLTOBJ (
                                 LIBRARY_NAME varchar(128) DEFAULT '*LIBL',
                                 OBJECT_NAME  varchar(128),
                                 OBJTYPE      varchar(10),
                                 ASP_NAME     varchar(10) DEFAULT '*SYSBAS',
                                 RMVMSG       varchar(10) DEFAULT '*NO',
                                 PROTECT      varchar(10) DEFAULT '*YES',
                                 CONVERT_NAME  varchar(10) DEFAULT '*YES'
                                   )
        RETURNS INT

     LANGUAGE C++
     NO SQL
     NO FINAL CALL
     EXTERNAL ACTION
     NOT DETERMINISTIC
     NOT FENCED
     SPECIFIC sqlTools.DELETE_OBJECT
     EXTERNAL NAME 'SQLTOOLS/DLTOBJ'
     PARAMETER STYLE DB2SQL;


LABEL on specific routine sqltools.DELETE_OBJECT  IS
'Delete an object';

COMMENT ON SPECIFIC FUNCTION sqltools.DELETE_OBJECT  IS
'Delete and IBM i object. This routine will delete an IBM i object.
The Object name and Schema (library) may be the standard "short"
system object name (up to 10 characters) or a long SQL name.';

COMMENT on parameter SPECIFIC FUNCTION sqltools.DELETE_OBJECT
(
library_name is 'The library name containing the Object
to be deleted. If no library name is specified, then *LIBL is used.
This parameter may be specified using either the standard "short"
system object name (up to 10 characters) or it may be a long SQL name.
The symbolic library names *LIBL or *CURLIB may be used.',


OBJECT_NAME IS 'The name of the object that is to be deleted.
The object name may be a full object name or a generic object name,
or a long SQL name.
Note *ALL is blocked by this function and may not be used.',

OBJTYPE  IS 'The standard IBM i Object Type for the object that is
to be deleted. The leading asterisk is optional.
Upper/lower case is ignored.',

ASP_NAME  IS 'Auxiliary Storage Pool (ASP) Device Name where the library
is located. Valid choices are: <ul><li>*</li>
 <li>*SYSBAS</li>
 <li>*CURASPGRP</li>
 <li>*ALLAVL</li></ul>
 Upper/lower case is ignored.',

RMVMSG IS 'Controls whether the CPC2191 Object Deleted message is generated
or suppressed. The valid choices are *YES or *NO, ''1'', or ''0''.
The default *NO causes the CPC2191 message to be written to the joblog.
Other messages, such as not found, invalid name, etc.
are always written to the joblog.',

PROTECT is 'Limits whether to permit objects in QSYS or any library
that starts with ''Q'', ''SYS'', ''IBM'', or ''SQL'' from being deleted.
The valid choices are:<ul><li><i>*YES</i> - Protect system objects</li>
<li>*NO - Do not protect system objects</li></ul>Note that "TRUE" and
"FALSE" may be used instead of "*YES" and "*NO" respectively.',

CONVERT_NAME is 'Controls whether or not to automatically convert the
value of the OBJECT_NAME and LIBRARY_NAME parameters to upper case.
In rare cases, IBM i interfaces can create object names that contain
lower case characters. Use this parameter to force the function to
automatic convertion of the object and library names to all upper case.
The valid choices are:<ul><li><i>*YES</i> - Convert to upper case</li>
<li>*NO - Do not convert to upper case</li></ul>Note that "TRUE" and
"FALSE" may be used instead of "*YES" and "*NO" respectively.
<p>Note: Object names encloded in double quotes (e.g., "Old.file")
are never capitalized by this function regardless of the value of
this parameter.</p>'

 );

