  create or replace function sqltools.CVT_RPG(
                                rpg3SrcLib  varchar(10) DEFAULT '*LIBL',
                                rpg3SrcFile varchar(10) DEFAULT 'QRPGSRC',
                                rpg3SrcMbr  varchar(10),  -- REQUIRED PARM!
                                rpg4SrcLib  varchar(10) DEFAULT '*LIBL',
                                rpg4SrcFile varchar(10) DEFAULT 'QRPGLESRC',
                                rpg4SrcMbr  varchar(10) DEFAULT '*FROMMBR',
                                   EXPCOPY varchar(10) DEFAULT '*NO',
                                   OUTPUT  varchar(10) DEFAULT '*NONE',
                                   SECLVL  varchar(10) DEFAULT '*YES',
                                   RULER   varchar(10) DEFAULT '*YES',
                                   LogLib  varchar(10) DEFAULT '*NONE',
                                   LogFile varchar(10) DEFAULT '*NONE',
                                   LogMbr  varchar(10) DEFAULT '*FIRST'
                             )
            returns int


      LANGUAGE SQL
      MODIFIES SQL DATA
      DETERMINISTIC
      NOT FENCED
      NO EXTERNAL ACTION
      SPECIFIC sqlTools.CVT_RPG

    set option datfmt = *ISO, commit=*NONE

R: BEGIN

 DECLARE CVTRPGSRC VARCHAR(256) NOT NULL DEFAULT '';
 DECLARE SUCCESS   INT NOT NULL DEFAULT 0;
 DECLARE QRPG3FILE VARCHAR(21) NOT NULL DEFAULT '';
 DECLARE QRPG3MBR  VARCHAR(10) NOT NULL DEFAULT '';
 DECLARE QRPG4FILE VARCHAR(21) NOT NULL DEFAULT '';
 DECLARE QRPG4MBR  VARCHAR(10) NOT NULL DEFAULT '';
 DECLARE EXPCOPY_OPT  VARCHAR(10) NOT NULL DEFAULT '*NO';
 DECLARE CVTRPT_OPT   VARCHAR(10) NOT NULL DEFAULT '*NO';
 DECLARE SECLVL_OPT   VARCHAR(10) NOT NULL DEFAULT '*YES';
 DECLARE RULER_OPT    VARCHAR(10) NOT NULL DEFAULT '*YES';
 DECLARE LOGFILE_OPT  VARCHAR(21) NOT NULL DEFAULT '*NONE';
 DECLARE LOGMBR_OPT   VARCHAR(10) NOT NULL DEFAULT '*LAST';


    -- RPGIII Source File Member (input file)
 if (rpg3srclib is not NULL and rpg3srclib <> '') THEN
     set QRPG3FILE =  trim(upper(rpg3srclib)) concat '/';
 else
     set QRPG3FILE =  '*LIBL/';
 end if;

 if (rpg3srcfile is not NULL and rpg3srcfile <> '') THEN
     set QRPG3FILE =  trim(qrpg3file) concat trim(upper(rpg3srcfile));
 else
     set QRPG3FILE =  trim(qrpg3file) concat 'QRPGSRC';
 end if;

 if (rpg3srcMbr is not NULL and rpg3srcMbr <> '') THEN
     set QRPG3MBR =  trim(upper(rpg3srcMbr));
 else
     set QRPG3MBR =  '*FIRST';
 end if;

    -- RPG IV Source File Member (output file)
 if (rpg4srclib is not NULL and rpg4srclib <> '') THEN
     set Qrpg4FILE =  trim(upper(rpg4srclib)) concat '/';
 else
     set Qrpg4FILE =  '*LIBL/';
 end if;

 if (rpg4srcfile is not NULL and rpg4srcfile <> '') THEN
     set Qrpg4FILE =  trim(qrpg4file) concat trim(upper(rpg4srcfile));
 else
     set Qrpg4FILE =  trim(qrpg4file) concat 'QRPGLESRC';
 end if;

 if (rpg4srcMbr is not NULL and rpg4srcMbr <> '') THEN
     set Qrpg4MBR =  trim(upper(rpg4srcMbr));
 else
     set Qrpg4MBR =  '*FROMMBR';
 end if;

 if (expCopy is not NULL and expCopy <> '') THEN
    if (ltrim(UPPER(expcopy), '*') = 'NO') THEN
      set EXPCOPY_OPT = '*NO';
    else
      set EXPCOPY_OPT = '*YES';
    end if;
 end if;

 if (output is not NULL and output <> '') THEN
    if (ltrim(UPPER(output), '*') IN ('NO','NONE')) THEN
      set CVTRPT_OPT = '*NO';
    else
      set CVTRPT_OPT = '*YES';
    end if;
 end if;

 if (seclvl is not NULL and secLvl <> '') THEN
    if (ltrim(UPPER(seclvl), '*') = 'NO') THEN
      set SECLVL_OPT = '*NO';
    else
      set SECLVL_OPT = '*YES';
    end if;
 end if;

 if (ruler is not NULL and ruler <> '') THEN
    if (ltrim(UPPER(ruler), '*') = 'NO') THEN
      set ruler_OPT = '*NO';
    else
      set ruler_opt = '*YES';
    end if;
 end if;

   --  Conversion Log File


 if (logfile is not NULL and logfile <> '') THEN
   if (ltrim(upper(logFile), '*') NOT IN ('NO','NONE')) THEN
     if (loglib is not NULL and loglib <> '') THEN
         set LOGFILE_OPT =  trim(upper(loglib)) concat '/';
     else
         set LOGFILE_OPT =  '*LIBL/';
     end if;
     set logFILE_OPT =  trim(logfile) concat trim(upper(logfile));
   end if;
 end if;

 if (logMbr is not NULL and logMbr <> '') THEN
     set logMBR_OPT =  trim(upper(logMbr));
 end if;

   set CVTRPGSRC = ' FROMFILE(' concat QRPG3FILE concat ')' concat
                   ' FROMMBR(' concat QRPG3MBR concat ')';
   set CVTRPGSRC = CVTRPGSRC CONCAT
                   ' TOFILE(' concat QRPG4FILE concat ')' concat
                   ' TOMBR(' concat QRPG4MBR concat ')';

    -- Command Options:
   set CVTRPGSRC = CVTRPGSRC CONCAT
                   ' EXPCPY(' concat EXPCOPY_OPT concat ') ' CONCAT
                   ' CVTRPT(' concat CVTRPT_OPT concat ') '  CONCAT
                   ' SECLVL(' concat SECLVL_OPT concat ') '  CONCAT
                   ' INSRTPL(' concat RULER_OPT concat ') '  CONCAT
                   ' LOGFILE(' concat LOGFILE concat ')' concat
                   ' LOGMBR(' concat LOGMBR concat ')';
   set CVTRPGSRC = 'CVTRPGSRC ' concat cvtrpgsrc;
    values qsys2.qcmdexc( cvtRPGSrc ) into Success;
    return SUCCESS;
end;
