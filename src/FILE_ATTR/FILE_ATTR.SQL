
 -- Member Description Information

-- SPDX-License-Identifier: Apache-2.0
-- Copyright (c) 1996-2026 by R. Cozzi, Jr.

-- @author BobCozzi


CREATE or REPLACE FUNCTION sqltools.FILE_ATTR(
                    LIBRARY_NAME varchar(10) DEFAULT '*LIBL',
                    FILE_NAME varchar(10) DEFAULT NULL,
                    MBR_NAME varchar(10)  DEFAULT '*FIRST',
                    PATH_NAME VARCHAR(4096) CCSID 1208 DEFAULT NULL,
                    cvtOption varchar(10) DEFAULT 'NO'
                                           )
       RETURNS table (
            created_Date timeStamp(0),  -- Creation Date
            access_Date timestamp(0), -- Last accessed
            changed_Date timestamp(0), -- Last Changed
            data_changed_date timestamp(0), -- Last DataChanged
            last_used timestamp(0),
            last_used_days int,
            last_used_reset timestamp(0),
            Allocate_size bigint,
            data_size bigint,
            object_CCSID int,
            ASP_Nbr   smallInt,
            TEXT      VARCHAR(50) CCSID 1208,
            OBJTYPE   VARCHAR(10)
           )
   LANGUAGE RPGLE
  NO SQL
  NO FINAL CALL
  SCRATCHPAD 128
  NOT DETERMINISTIC
  DISALLOW PARALLEL
  CARDINALITY 1
  EXTERNAL NAME 'SQLTOOLS/FILE_ATTR'
  SPECIFIC sqlTools.FILE_ATTR
  PARAMETER STYLE DB2SQL;

LABEL on specific routine sqltools.FILE_ATTR IS
'${version} - Retrieve File Attributes';

COMMENT ON Specific FUNCTION sqltools.FILE_ATTR  IS
 '${version} - Retrieve File Attributes';

comment on parameter SPECIFIC FUNCTION sqltools.FILE_ATTR
 ( library_NAME IS 'The library name containing the file whose
  attributes are to be received. If the FILE_NAME parameter is
  null or empty, then this parameter is ignored.',

  FILE_NAME IS 'The name of the file whose attributes are returned.
  This parameter may be a valid IBM i Db2 File Name, or it may
  be NULL or blank/empty. When this parameter is empty, the
  LIBRARY_NAME, FILE_NAME and MBR_NAME parameters are ignored and
  the PATH_NAME parameter is used as the file name.
  Note upper/lower case is ignored, unless the name is enclosed in
  double-quotes.',

  MBR_NAME IS 'The name of the member whose attributes are returned.
  The special values *FIRST, *LAST may be specified in addition to
  a member name. If unspecified, *FIRST is used.
  Note upper/lower case is ignored, unless the name
  is enclosed in double-quotes.',

   PATH_NAME IS 'Specifies the file whose attributes are to be returned.
   The path_name must be a valid IFS or /QSYS.LIB file path such as<br />
   /QSYS.LIB/QGPL.LIB/QRPGLESRC.FILE<br />
   Note the member may also be included after the .FILE segment. When
   the path_name is specifie, the FILE_NAME, LIBRARY_NAME, and MBR_NAME
   parameters are ignored. When the FILE_NAME parameter is specified,
   then this parameter (PATH_NAME) is ignored.',

   CVTOPTION IS 'Controls whether to automatically convert the file name
   to all upper case. The default ''NO'' indicates that no case conversion
   will be preformed by the function. When ''YES'' or ''UPPER'' is specified,
   then the QlgConvertCase API is used to translate the file name to
   upper case. When ''LOWER'' is specified, the file name is translated to
   all lower case, again using the QlgConvertCase API.'
 );


