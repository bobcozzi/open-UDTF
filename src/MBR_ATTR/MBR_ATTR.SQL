
 -- Member Description Information

-- SPDX-License-Identifier: Apache-2.0
-- Copyright (c) 1996-2026 by R. Cozzi, Jr.

-- @author BobCozzi


CREATE or REPLACE FUNCTION sqltools.mbr_attr(
                                  LIBRARY_NAME varchar(10) DEFAULT '*LIBL',
                                  FILE_NAME varchar(10),
                                  MBR_NAME varchar(10)  default '*FIRST',

                                  OVR_MBR varchar(10)  default 'YES',
                                  FIND_MBR  varchar(10)  default 'YES',
                                  DETAILED_INFO varchar(10)  default 'YES'
                                           )
       RETURNS table (
          -- Begin detailed_info=>'BASIC' | 'NO' columns
            OBJLIB    varchar(10),
            OBJNAME   varchar(10),
            MBRNAME   varchar(10),
            SRCTYPE   varchar(10),
            FILEATTR  varchar(10),   -- PF, LF, DDMF
            TEXT      varchar(50),   -- Mbr Text description
            mbr_created timeStamp(0),  -- Date/Time member was added
            Last_Source_changed timestamp(0), -- Last Changed

          -- Begin detailed_info=>'FULL' | 'YES' columns
            Record_count BIGINT,         -- Current record count
            Deleted_Record_Count BIGINT, -- Deleted Records count
            EXPDATE   date,          -- Member Expiration Date
            Last_used_Days int,      -- Days since last used
            Last_used_Date date,     -- Last Used Date/Time
            Last_used_Reset_date date, -- Last used: Reset Date
            Object_Changed timestamp(0)   -- Object Changed timestamp
           )
       LANGUAGE RPGLE
       NO SQL
       NOT FENCED
       NOT DETERMINISTIC
       DISALLOW PARALLEL
       SCRATCHPAD 320
       SPECIFIC sqlTools.mbr_attr
       EXTERNAL NAME 'SQLTOOLS/MBR_ATTR'
       CARDINALITY 1
       PARAMETER STYLE DB2SQL;


LABEL on specific routine sqltools.mbr_attr IS
'${version} - Retrieve Source Member Description';

COMMENT ON Specific FUNCTION sqltools.mbr_attr  IS
 '${version} - Retrieve Source Member Description returns the basic or
 advanced attributes for the specified Member name.';

comment on parameter SPECIFIC FUNCTION sqltools.mbr_attr
 ( library_NAME IS 'The library name containing the file that contains the
  member whose description is retrieved. The special values *LIBL or
  *CURLIB may be specified. If unspecified, *LIBL is used.
  Note upper/lower case is ignored, unless the name
  is enclosed in double-quotes.',


  FILE_NAME IS 'The name of the source file whose member attributes
  are returned.   Note upper/lower case is ignored, unless the name
  is enclosed in double-quotes.',

  MBR_NAME IS 'The name of the member whose attributes are returned.
  The special values *FIRST, *LAST may be specified in addition to
  a member name. If unspecified, *FIRST is used.
  Note upper/lower case is ignored, unless the name
  is enclosed in double-quotes.',

   OVR_MBR IS 'Specifies whether to respect or ignore overrides placed
   on the file. *YES or *NO or ''1'' or ''0'' may be specified.
   If unspecified, then *YES is used. Upper/lower case and the leading
   asterisk are ignored.',

   FIND_MBR IS 'Specifies whether to locate the member in the FILE_NAME
   on the library list when LIBRARY_NAME=>''*LIBL'' is specified.
   FIND_MBR=>''*YES'' or ''*NO'' or ''1'' or ''0'' may be specified.
   If unspecified, then *YES is used. Upper/lower case and the leading
   asterisk are ignored.'
 );


