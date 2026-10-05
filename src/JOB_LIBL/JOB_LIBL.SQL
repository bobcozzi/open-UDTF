
    -- Retrieve Job Attributes

CREATE or REPLACE FUNCTION sqltools.job_libl(
           JOB          varchar(28) default '*',
           PORTION      varchar(10) default '*ALL'
                                  )
       RETURNS table (

          ORDINAL_POSITION int,  -- Sequence of library on library list
          LIBRARY_NAME      VARCHAR(10),  -- Library Name
              -- Library List Portion (i.e., SYSTEM, PRODUCT, CURRENT, USER)
          TYPE              VARCHAR(10),
              -- Library Type (i.e., *PROD, *TEST)
          LIBRARY_TYPE      VARCHAR(5),
          LIBRARY_TEXT      VARCHAR(50),
          IASP_NUMBER       smallint,
          IASP_NAME         VARCHAR(10),
          IASP_GROUP        VARCHAR(10),
          OBJECT_COUNT      INT,
              -- Same as TYPE
          PORITION          VARCHAR(10),
              -- Qualified 3-part job name of job whose library_List is returned
          Job               VARCHAR(28)

       )
    LANGUAGE RPGLE
    NO SQL
    NOT DETERMINISTIC
    NOT FENCED
    CALLED ON NULL INPUT
    DISALLOW PARALLEL
    SCRATCHPAD 256
    SPECIFIC sqlTools.JOB_LIBL
    EXTERNAL NAME 'SQLTOOLS/JOB_LIBL'
    PARAMETER STYLE DB2SQL;


LABEL on specific routine sqltools.job_libl IS
'${version} Library list for the specified job';

comment on specific function sqltools.job_libl IS
'${version} Library list for the specified job
retrieves the libary list for the specified job or the current job
when no JOB parameter is specified. The library name, the portion of
the library list where the library occurs along with the library object''s
Text description and Type (*PROD or *TEST) are returned.
The iASP name, number and group of the library is also included.';

comment on parameter specific function sqltools.job_libl
( JOB is 'The fully qualified job name whose library list is returned.
 The default * returns the library list for the job running the UDTF.
 The job name must be in the <i>nnnnnn/userid/jobname</i> format.'
);
