CREATE or REPLACE FUNCTION sqltools.CMD_EVENTF (
                                  LIBRARY_NAME VARCHAR(10) DEFAULT '*LIBL',
                                  EVENTF_NAME  VARCHAR(10) DEFAULT 'EVENTF',
                                  MBR_NAME     VARCHAR(10)
                                          )
      RETURNS TABLE (
          EVENT_TIMESTAMP   CHAR(14),
          SOURCE_ID         INTEGER,
          SOURCE_FILE       VARCHAR(256),
          SOURCE_TIMESTAMP  CHAR(14),
          SOURCE_FLAG       INTEGER,

          STMT_LINE         INTEGER,
          START_LINE        INTEGER,
          START_COLUMN      INTEGER,
          END_LINE          INTEGER,
          END_COLUMN        INTEGER,

          MESSAGE_ID        CHAR(7),
          SEVERITY_CHAR     CHAR(1),
          MSGSEV            INTEGER,
          MSGTEXT           VARCHAR(2048)
      )
      LANGUAGE C++
      NO SQL
      NO FINAL CALL
      NOT DETERMINISTIC
      NO EXTERNAL ACTION
      DISALLOW PARALLEL
      CARDINALITY 20
      SCRATCHPAD 8400
      SPECIFIC sqlTools.cmd_eventf
      EXTERNAL NAME 'SQLTOOLS/CMD_EVENTF'
      PARAMETER STYLE DB2SQL;


LABEL on specific routine sqlTools.cmd_check IS
'Syntax Check CL commands via QCAPCMD API';

comment on specific FUNCTION sqlTools.cmd_check is
'${version} - CL Command Syntax Checker (QCAPCMD Wrapper)';

comment on parameter specific function sqlTools.cmd_check
(
CMD is 'CL command to be checked. A command length of up to 6000
  bytes is supported by this function',

CHECKOPT IS 'Command syntax checking option(s). Use this parameter
to control what kind of syntax checking is performed by this function.
The leading asterisk and upper/lower case is ignored.
The default is: <i>ILE CL Program Source syntax checking<i> or <b>*CLLE</b>
The valid choices are:
<ul>
 <li>*RUN - 0. Run CL Cmd (similar to QCMDEXC API)</li>
 <li>*CHECK or *CL - 1. Syntax Check Cmd (similar to QCMDCHK API)</li>
 <li>*RUNLIMIT - 2. Run CL Cmd as limited user command</li>
 <li>*CHKLIMIT - 3. Cmd Syntax check for limited User</li>
 <li>*CLP - 4. Syntax check Cmd in OPM CL program source</li>
 <li>*STREAM - 5. System/38 CL Stream batch jobs</li>
 <li>*CMD - 6. Syntax check *CMD statements (e.g., PARM, ELEM, QUAL)</li>
 <li>*BINDER or *BND - 7. Syntax check Cmd for Binder Language</li>
 <li>*PDM - 8. Syntax check Cmd with embedded PDM user-defined options</li>
 <li>*CLLE or *ILE - 9. Syntax check Cmd in ILE CL program source</li>
 <li>*PMT or *PROMPT - 10. (Reserved)</li>
 </li>
</ul>'
);
