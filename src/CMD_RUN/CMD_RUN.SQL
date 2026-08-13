
  -- CL Command Processor for IBM i
  -- This function runs or syntax checks any CL command.
  -- For specific CL environments, like CL programs or command entry,
  -- use the MODE parameter identify the format for syntax checking.
  -- Note the options for MODE for running a CL command are limited
  -- to RUN or LIMIT. RUN runs the command as if it were on a Command Entry
  -- display being run. LIMIT runs the command for a so called
  -- LIMITED User Profile--which restricts certain commands from running.
  -- The default MODE=>'*CLLE' is to Syntax Check the CL Command for
  -- use in a CL Program. This means CL variables, expressions, and
  -- built-in functions are permitted for syntax checking.
CREATE or REPLACE FUNCTION sqltools.CMD_RUN(
                                    CMD   VARCHAR(32700),
                                    MODE  VARCHAR(14) DEFAULT '*CLLE'
                                          )
            RETURNS table (
                    ORDINAL_POSITION smallint,
                    msgid   varchar(7),
                    msgsev  int,
                    msgtype varchar(10),
                    msgtext varchar(1024),
                    seclvlMsg varchar(2048)
              )
     LANGUAGE C++
     NO SQL
     NOT DETERMINISTIC
     NOT FENCED
     NO FINAL CALL
     DISALLOW PARALLEL
     CARDINALITY 5
     SCRATCHPAD 2000
     SPECIFIC sqlTools.CMD_RUN
     EXTERNAL NAME 'SQLTOOLS/CMD_RUN'
     PARAMETER STYLE DB2SQL;


LABEL on specific routine sqlTools.cmd_run IS
'${version} - Run or Check CL commands via QCAPCMD';

comment on specific FUNCTION sqlTools.cmd_run is
'${version} - CL Command Processor to Run and Syntax Checker CL commands<br />
The resultSet returns the MSGID, MSGSEV, MSGTYPE, 1st Level Message Text,
and 2nd Level Message text (sometimes called "Message Help"). <br />
When running a CL command, if no messages are generated the resultSet
is empty. When checking CL command sytnax, if no syntax errors are detected,
the resultSet is empty.';

comment on parameter specific function sqlTools.cmd_run
(
CMD is 'CL command to be processed. A command length under 32k bytes is
supported by this function. Command Prompting is not supported.',

MODE IS 'Processing mode. Use this parameter to control whether the
function syntax checks the CL command or runs it. All modes syntax check
the CL command. The *RUN, *LIMIT (or *RUNLIMIT) modes syntax check
the CL command and then immediately run the command.
<ul><li>*RUN - Runs command like Command Entry</li>
<li>*LIMIT - Runs command as "limited" User</li>
<li>*CHECK - Syntax Checks for Command Entry use</li>
<li>*CHECKLIMIT - Syntax Checks Command for "limited" Users</li>
<li><b>*CLLE</b>  - Syntax Checks for ILE CL Programs alt(ILE)</li>
<li>*CLP   - Syntax Checks for OPM (CLP) CL Programming </li>
<li>*CMD   - Syntax Checks a Command Definition statement</li>
<li>*BND   - Syntax Checks a Binder Source statement</li>
<li>*PDM   - Syntax Checks for PDM option use</li>
<li>*PROMPT - (ignored/notsupported)</li>
</ul>
The leading asterisk and upper/lower case is ignored.
The default is: <i>*CLLE - ILE CL Program Source file syntax checking</i>'
);