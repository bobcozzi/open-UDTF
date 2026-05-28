
 -- SPDX-License-Identifier: Apache-2.0
 -- Copyright (c) 1996-2026 by R. Cozzi, Jr.
 -- @author BobCozzi

-- Check User Authority to an Object
--
-- Source origin: /Users/cozzi/Downloads/projects/open-UDTF/src/CHECK_AUT/CHECKA
--

CREATE or REPLACE FUNCTION SQLTOOLS.check_aut(
                                   library_name varchar(10) DEFAULT '*LIBL',
                                   object_NAME  VARCHAR(10),
                                   objtype      VARCHAR(10) default '*FILE',
                                   auth         VARCHAR(96) default '*USE',
                                   user_name    VARCHAR(10) default '*CURRENT',
                                   callLvl      VARCHAR(10) default '*SAME'
                                  )
        returns int

    LANGUAGE C++
    NO SQL
    NO EXTERNAL ACTION
    NO FINAL CALL
    STATEMENT DETERMINISTIC
    NOT FENCED
    SPECIFIC sqlTools.check_aut
    EXTERNAL NAME 'SQLTOOLS/CHECKAUT'
    PARAMETER STYLE DB2SQL;


LABEL on specific routine sqltools.CHECK_AUT IS
'Check a user Profile authority to an object';

COMMENT on SPECIFIC FUNCTION sqltools.CHECK_AUT IS
'Checks a User Profile authority to the specified Object.
 The authorization is determined if the user has all rights to the object
 specified on the AUTH parameter.
 Return value 1 = Authorized.
 Return value 0 = not authorized.';

Comment on Parameter Specific FUNCTION sqltools.CHECK_AUT
(
 LIBRARY_NAME IS 'The name of the library where the object specified on
the OBJECT_NAME parameter is located.
The special values *LIBL and *CURLIB are supported. The default is *LIBL',

OBJECT_NAME is 'The object name being checked for the  User''s authority.
Upper/lower case is ignored.',

OBJTYPE is 'The IBM i object type of the object specified on the OBJECT_NAME
  parameter. Upper/lower case is ignored, and the leading asterisk is option.',


AUTH is 'A list of one or more authorities being checked for. Up to 10
 authorities may be specified (listed below). Each authority must be
seperated by one or more blanks or commas. Upper/lower case is ignored
and the leading asterisk is optional. The valid authorities are:<ul>
 <li><u>*USE</u> - *OBJOPR *READ and *EXECUTE authority</li>
 <li>*ALL - All authority</li>
 <li>*CHANGE -  *OBJOPR *OBJREF *OBJALTER *READ *ADD *DLT *UPD</li>
 <li>*EXCLUDE - Exclude authority</li>
 <li>*AUTLMGT- For *AUTL objects, Authorization List mgt authority</li>
 <li>*OBJALTER - Object alter authority</li>
 <li>*OBJOPER  - Object operational authority</li>
 <li>*OBJMGT   - Object management authority</li>
 <li>*OBJEXIST - Object existence authority</li>
 <li>*OBJREF   - Object reference authority</li>
 <li>*READ     - Read authority</li>
 <li>*ADD or *WRITE - Add authority</li>
 <li>*UPD or *UPDATE - Update authority</li>
 <li>*DLT or *DELETE - Delete authority</li>
 <li>*EXECUTE or *RUN - Run a program or find an object in a library</li>
</ul>',

USER_NAME is 'The user profile whose authority to the object is checked.'

);

