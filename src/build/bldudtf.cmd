 BLDUDTF:    CMD        PROMPT('Create SQL Tools OSS UDTF')
             PARM       KWD(UDTF) TYPE(*NAME) LEN(10) MIN(1) +
                          EXPR(*YES) PROMPT('UDTF Source member')
             PARM       KWD(EXTPGM) TYPE(*CHAR) LEN(10) MIN(1) +
                          EXPR(*YES) PROMPT('External program name')

             PARM       KWD(EXTPGMSRC) TYPE(EXTPS) +
                          PROMPT('External program source file')
 EXTPS:      QUAL       TYPE(*NAME) LEN(10) DFT(QCSRC) +
                          SPCVAL((QCSRC)) EXPR(*YES)
             QUAL       TYPE(*NAME) LEN(10) DFT(SQLTOOLS) EXPR(*YES) +
                          PROMPT('Source library')
             PARM       KWD(EXTPGMMBR) TYPE(*NAME) LEN(10) +
                          DFT(*EXTPGM) SPCVAL((*EXTPGM)) +
                          PROMPT('External program source member')

             PARM       KWD(UDTFSRC) TYPE(UDFSRC) +
                          PROMPT('UDTF Source file')
 UDFSRC:     QUAL       TYPE(*NAME) LEN(10) DFT(QUDFSRC) +
                          SPCVAL((QUDFSRC) (QSQLSRC)) EXPR(*YES)
             QUAL       TYPE(*NAME) LEN(10) DFT(SQLTOOLS) EXPR(*YES) +
                          PROMPT('Source library')
             PARM       KWD(OBJLIB) TYPE(*CHAR) LEN(10) +
                          DFT(SQLTOOLS) PROMPT('Target Library')

                    /* If dependent objects do not exist always build them.  */
                    /*    1=Bind only*                             */
                    /*    2=Build/Refresh the dependent objects    */
                    /* *If DEP objects don't exist, they are always be built */
             PARM       KWD(BLDDEP) TYPE(*CHAR) LEN(1) DFT(*BIND) +
                          SPCVAL((*BIND '1') (*REFRESH '2')) +
                          EXPR(*YES) PROMPT('DEP OBJs Bind or refresh+Bind')
             PARM       KWD(DBGVIEW) TYPE(*CHAR) LEN(8) RSTD(*YES) +
                          DFT(*SOURCE) SPCVAL((*NONE) +
                          (*STMT) (*SOURCE) (*LIST)) PROMPT('Debug +
                          view')
             PARM       KWD(DROP) TYPE(*LGL) RSTD(*YES) DFT(*NO) +
                          SPCVAL((*YES '1') (*NO '0') (*DROP '1')) +
                          EXPR(*YES) PROMPT('DROP FUNCTOIN before +
                          create')
             PARM       KWD(SPECNAME) TYPE(*PNAME) LEN(128) +
                          DFT(*UDTF) SPCVAL((*UDTF)) EXPR(*YES) +
                          PMTCTL(DROPNAME) PROMPT('Specific name +
                          for DROP')
 DROPNAME:   PMTCTL     CTL(DROP) COND((*EQ '1')) NBRTRUE(*EQ 1)