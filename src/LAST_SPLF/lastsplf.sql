

CREATE or REPLACE FUNCTION sqltools.LAST_SPLF(
                             SCOPE VARCHAR(10) DEFAULT 'JOB'
                                             )
       RETURNS table (
          splfName  varchar(10),
          splnbr    int,
          job       varchar(28),
          jobName   varchar(10),
          jobUser   varchar(10),
          jobNbr    varchar(6),
          sysname   varchar(8), -- System name where SPOOLED file was created
          CREATION_TIMESTAMP TIMESTAMP(0)

)   LANGUAGE C++
     NO SQL
     NOT DETERMINISTIC
     NOT FENCED
     NO FINAL CALL
     DISALLOW PARALLEL
     CARDINALITY 1
     SCRATCHPAD 255
     SPECIFIC sqlTools.LAST_SPLF
     EXTERNAL NAME 'SQLTOOLS/LASTSPLF'
     PARAMETER STYLE DB2SQL;

LABEL on specific routine SQLTOOLS.LAST_SPLF IS
'Retrieve Last SPOOLED File Created by User or Job';

comment on SPECIFIC FUNCTION SQLTOOLS.LAST_SPLF is
 'Retrieve the Last SPOOLED File Identity (RTVLASTSPLF) UDTF.
  This UDTF returns the name, number and other properties of
  the most recently created SPOOLED file for the Job. In addition, the
  SCOPE parameter may be used to retrieve the last SPOOLED file created
  by the User profile (regardless of job) or
  the last SPOOLED file created on the system (regardless of user or job).
  Users of tools such as IBM ACS RUN SQL SCRIPTS interface will find it
  useful with SCOPE=>''USER'' which returns the last SPOOLED file created
  for the User running this UDTF.';

comment on parameter SPECIFIC Function SQLTOOLS.LAST_SPLF (
  SCOPE is 'Controls how the last SPOOLED File identity is located.
The valid choices are:
<ul><li><u>*JOB</u> - Last SPOOLED File created in "this" Job</li>
<li>*USER - Last SPOOLED File the User created</li></ul>
<p>*JOB returns faster that *USER.</p>'
);

