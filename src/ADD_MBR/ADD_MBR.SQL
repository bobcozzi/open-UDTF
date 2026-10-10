
 -- Add Member to file using /QSYS.LIB path syntax
 -- @author BobCozzi

-- This function adds a member (source member) to an existing source file.
-- @param PATH_NAME Full path to the source file member to create or update.
-- the format is the '/QSYS.LIB/ style path name:
--      e.g., /QSYS.lib/QGPL.lib/QRPGLESRC.FILE/MYNEWMBR.MBR'
-- or using the seu/source type as the suffix (option):
--      e.g., /QSYS.lib/QGPL.lib/QRPGLESRC.FILE/MYNEWMBR.rpgle'
-- @param SRCTYPE Source type to assign to the member,
--      e.g.  SQL, RPGLE, CLLE, SQLRPGLE or other valid source types.
--        When SRCTYPE is specified, it is assigned to the member once added.
-- @param MBRTEXT Member text description to assign to the source member.

-- Notes: When the source type is used with the path name as its suffix,
--        such as '/QSYS.lib/DEVLIB.lib/QCLSRC.FILE/EODRUN.clle'
--        and the SRCTYPE parmaeter is not specified, the suffix
--        is used as the SRCTYPE.

CREATE or REPLACE FUNCTION sqltools.ADD_MBR(
                    PATH_NAME VARCHAR(4096) CCSID 1208 DEFAULT NULL,
                    SRCTYPE  VARCHAR(10) DEFAULT NULL,
                    MBR_TEXT VARCHAR(50) CCSID 1208 DEFAULT NULL
                                           )
       RETURNS table (
            LIBNAME VARCHAR(10),
            OBJNAME VARCHAR(10),
            OBJTYPE VARCHAR(10),
            MBRNAME VARCHAR(10),
            SRCTYPE VARCHAR(10),
            OBJTEXT VARCHAR(50) CCSID 1208,
            ASPNAME VARCHAR(10)
           )

  LANGUAGE RPGLE
  NO SQL
  NO FINAL CALL
  NOT DETERMINISTIC
  DISALLOW PARALLEL
  SCRATCHPAD 16
  SPECIFIC sqltools.ADD_MBR
  EXTERNAL NAME 'SQLTOOLS/ADD_MBR'
  CARDINALITY 1
  PARAMETER STYLE DB2SQL;

LABEL on specific routine sqltools.ADD_MBR IS
'${version} - Add database memeber';

COMMENT ON Specific FUNCTION sqltools.ADD_MBR  IS
 '${version} - Add database member @author BobCozzi';

comment on parameter SPECIFIC FUNCTION sqltools.ADD_MBR
 (
   PATH_NAME IS 'Specifies the /QSYS.LIB source file and member name.
   The member name is added to the source file. The source type and text
   of that new member can be specified on the SRCTYPE and MBR_TEXT parameters
   respectively. When this /QSYS.LIB path name ends with anything other than
   .MBR then that suffix value can be used as the Source Type by ommitting
   the SRCTYPE parmaeter. If a source type suffix and the SRCTYPE parameter
   are specified, the SRCTYPE parameter value is used as the source type.',

  SRCTYPE IS 'The Source Type (aka "SEU TYPE") for the newly added source
  member. When this parameter is not specified, the suffix of the PATH_NAME
  is used as the source type. If that suffix is also unspecified or is
  .MBR then no srctype (source type) is assigned to the newly added member.',

  MBR_TEXT IS 'When the member is added, the text description of that memeber
  is set to the specified on this parameter.'
 );



