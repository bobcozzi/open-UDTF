

 CREATE or REPLACE FUNCTION sqltools.FIELD_LIST(
                              library_name  varchar(128) DEFAULT '*LIBL',
                              file_name     CLOB(1M),
                              file_type     varchar(32)  default '*ALL'
                                    )
     RETURNS TABLE (
        OBJLIB    varchar(10),    -- Library containing the file
        FILE_NAME varchar(10),    -- File name whose columns are returned
        FILE_ATTR varchar(10),    -- File attribute: PF, LF, DDMF
        FILE_TYPE varchar(10),    -- File type: *SRC or *DATA
        SQL_OBJECT_TYPE  varchar(10), -- SQL object type: TABLE, VIEW, INDEX
        CRTDATE   timestamp(0),   -- File creation timestamp

        FIELD_COUNT      int,     -- Number of columns in the record format
        RCDFMT_COUNT     int,     -- Number of record formats in the file

        LVLCHK           varchar(10),  -- Level checking: *YES or *NO
        LVLCHK_ID        varchar(13),  -- 13-byte Level Check ID
        rcdFmt_TEXT      varchar(50),
        RCDFMT_ID        varchar(13),
        RCDFMT_ORDINAL   int,
        RCDFMT           varchar(10),  -- Record format name
        RCDLEN           smallint,     -- Record length

        ORDINAL_POSITION int,          -- Field ordinal pos in the record
        FIELD_NAME    varchar(10),     -- Short system field name
        DATA_TYPE     varchar(10),     -- SQL data type
        BUFFER_LENGTH int,             -- Bytes the field occupies in buffer
        FIELD_LENGTH  int,             -- Declared field length
        DECPOS        smallInt,        -- Decimal positions (numeric fields)

        RPG_FIELD_DEFN varchar(32),    -- RPG IV field declaration
        DEFN_KWDS      varchar(64),    -- Definition keywords: DATFMT, CCSID
        CCSID          int,
        ALWNULL        varchar(10),

        FROM_POS       int,            -- Field start position in buffer
        TO_POS         int,            -- Field end position in buffer

        FIELD_ALIAS    varchar(128),   -- Field ALIAS or SQL long field name
        FIELD_USAGE    varchar(10),    -- Field usage: Input, Output, or Both
        COLUMN_HEADINGS varchar(60),   -- 3-part heading, 20 bytes each
        FIELD_TEXT     varchar(50),    -- Field text description

        FIELD_PROC     varchar(10),    -- RCAC exit program name
        FIELD_PROCLIB  varchar(10),    -- Library of the RCAC exit program
        LONG_FILE_NAME varchar(128),   -- Long SQL file name
        FILE_TEXT      varchar(50)     -- File text description
                  )

     LANGUAGE C++
     NO SQL
     NOT DETERMINISTIC
     NOT FENCED
     CALLED ON NULL INPUT
     SCRATCHPAD 512
     SPECIFIC sqlTools.FIELD_LIST
     EXTERNAL NAME 'SQLTOOLS/FIELDLIST'
     PARAMETER STYLE DB2SQL;



LABEL on specific routine sqlTools.FIELD_LIST IS
'List File Field definitions (similar to DSPFFD)';

comment on specific function sqlTools.FIELD_LIST IS
'List of definitions for the fileds (columns) in the file(s) specified.
Typically one FILE_NAME entry is specified however any number of files
may be included to returns all the field formats at once.<p>Both the SQL
definition/keyword is returned along with the RPG IV definition.';

comment on parameter Specific Function sqlTools.FIELD_LIST
( LIBRARY_NAME is 'The library name where the file is located.
If this parameter is NULL or unspecified, then *LIBL is used.
Any valid library (schema) name may be specified or one of following special
values:<ul><li><u>*LIBL</u> - The library list is used</li>
<li>*CURLIB - The job''s current library is used</li>
<li>*USRLIBL - The user portion of the job''s library list is used.</li>
<li>*ALLUSR - All user libraries</li>
<li>*ALL - All libraries</li></ul>
<p>The LIBRARY_NAME parameter is used as the library name for unqualified
file names specified on the FILE_NAME parameter.</p>
Upper/lower case is ignored unless the name is enclosed in double-quotes.',

FILE_NAME is '<p>A list of File names whose field definitions
are to be returned. The file name may be a full name, generic name,
This is a required parameter and it supports both Long and short
file names. When a Long SQL Name is specified, it may <i>not</i> be
a generic name. If more than one file name is specified, each file name
must be separated by one or more blanks and/or commas.
Upper/lower case is ignored. The file names may include a library qualifier.
When an unqualified file is specified, the library name specified on the
LIBRARY_NAME parameter is used to locate the file.
Here are some examples:</p>
<pre>FILE_NAME=>''DATALIB/CUSTOMER DATALIB/ORDDATA''</pre>
<pre>FILE_NAME=>''DATALIB/CUSTOMER orddata testlib/ORDDATA''</pre>
<pre>FILE_NAME=>''customers, orddata, qiws/qcustcdt''</pre>
This parameter can accept a large number of file names,
qualified or unqualified.',

        -- File_Type may be *ALL, *PF, *LF, *DDMx, *SRC, *DATA
FILE_TYPE IS
  'The File Type to be retrieved. Only file''s whose type
matches this entry are returned. The default is *ALL database file types.
The valid choices are:<ul><li><u>*ALL</u> - All database file types</li>
<li>PF - Physical Files</li>
<li>LF - Logical Views</li>
<li>DDMF - DDM Files</li>
<li>*DATA - Database (non source) files (PF, LF, DDMF)</li>
<li>*SRC - Source (non data) physical files (PF-SRC)</li></ul>
If multiple types are specified, they must be separated by at least
one blank and/or comma. For example: ''PF LF'' or ''PF,LF''
<p>If FILE_TYPE=>''PF'' is specified, both *DATA and *SRC files
are returned. To return only Source Files, specify FILE_TYPE=>''*SRC''
</p><p>Normally explicit file names are specified on the FILE_NAME parameter
so this parameter is often left at its default of *ALL.</p>
<p>Upper/lower case and the leading asterisk are ignored.</p>'
);



