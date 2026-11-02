**FREE

// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 by R. Cozzi, Jr.

// @author BobCozzi

    ////////////////////////////////////////////////////////////////////////
    // IBM i SQL Function: DELETE_OBJECT / DLTOBJ
    // This is an SQL UDF External Program
    // It uses the QLIDLTO API to delete the object
    ////////////////////////////////////////////////////////////////////////
    // This is part of the collection of open source SQL UDTFs that are
    // primarily built for the VS CODE and CODE for IBM i IDE, however
    // they can be feely used in other environments that permit running SQL:
    //  - IBM ACS RUNSQL Scripts
    //  - SQL iQuery
    //  Available on github at:  https://github.com/bobcozzi/open-UDTF
    ////////////////////////////////////////////////////////////////////////

ctl-opt   main(main) OPTION(*SRCSTMT);

/if defined(*CRTBNDRPG)
ctl-opt DFTACTGRP(*NO) ACTGRP(*CALLER);
/endif

         // Use the PSDS to obtain "this" Program's name
dcl-Ds psds psds qualified;
    Pgmname *proc;
end-Ds;

         // Standardized API exception/error struct for APIs
dcl-Ds QUSEC_t Qualified Template;
    Bytes_Provided int(10) inz(%size(QUSEC_T));
    Bytes_Available INT(10);
    Bytes_RTN int(10) overlay(bytes_available);
    Bytes_returned int(10) overlay(bytes_available);
    BytesReturned int(10) overlay(bytes_available);
    Exception_Id char(7);
    Msgid char(7) overlay(exception_id);
    Reserved char(1);
    Msgdata char(64);
end-Ds;

         // Delete Object API
dcl-pr QLIDLTO extpgm('QLIDLTO');
    qualObjName char(20) const;
    objType     char(10) const;
    aspName     char(10) const; // Use '*'
    rmvMsg      char(1)  const; // '1'=Remove "Object deleted" msg
    apiError    likeds(QUSEC_T) options(*varsize);
end-pr;

         // Retrieve System Name (long SQL name to system name)
dcl-pr QDBRTVSN extpgm('QDBRTVSN');
    sysName     char(20);
    sqlName     char(128) const options(*varsize);
    sqlNameLen  int(10)   const;
    libName     char(10)  const;
    apiError    likeds(QUSEC_T) options(*varsize);
end-pr;

         // Send Program Message API (used to resignal API errors)
dcl-pr QMHSNDPM extpgm('QMHSNDPM');
    msgID       char(7)   const;
    msgFile     char(20)  const;
    msgData     char(256) const options(*varsize);
    msgDataLen  int(10)   const;
    msgType     char(10)  const;
    callStkEnt  char(10)  const;
    callStkCnt  int(10)   const;
    msgKey      char(4);
    apiError    likeds(QUSEC_T) options(*varsize);
end-pr;

dcl-proc main ;
    dcl-pi main EXTPGM('DLT_OBJ');

             // Input parameters
        inLIBNAME varchar(128) const;
        inOBJNAME varchar(128) const;
        inOBJTYPE varchar(10)  const;
        inASPNAME varchar(10)  const;
        inRMVMSG  varchar(10)  const;
        inPROTECT varchar(10)  const;
        inCVTCASE varchar(10)  const;

             // Output (return) value
        outSUCCESS int(10);

             // UDF interface checklist (DB2SQL parameter style):
             // 1) Null indicators must be 2-byte (SMALLINT) -> RPG INT(5).
             // 2) Set indicator to -1 for NULL and 0 for NOT NULL.
             // 3) Keep trailing DB2SQL parms order: SQLSTATE, function/specific,
             //    message. Scalar UDFs have no scratchpad/SQL opcode.

             // Input indicators
        indy_inLIBNAME int(5);
        indy_inOBJNAME int(5);
        indy_inOBJTYPE int(5);
        indy_inASPNAME int(5);
        indy_inRMVMSG  int(5);
        indy_inPROTECT int(5);
        indy_inCVTCASE int(5);

             // Output indicator
        indy_outSUCCESS int(5);

             // Standard DB2SQL diagnostic fields
        outSQLSTATE char(5);
        inFuncName  varchar(517) const; // Function name
        inSpecName  varchar(128) const; // Specific function name
        outSQLMSG   varchar(70);
    end-pi;

    dcl-s objName     char(10);
    dcl-s libName     char(10);
    dcl-s objType     char(10);
    dcl-s aspName     char(10) inz('*');
    dcl-s rmvMsg      char(1)  inz('0');
    dcl-s protect     ind      inz(*on);
    dcl-s cvtCase     ind      inz(*on);
    dcl-s qualObjName char(20);
    dcl-s wkOption    varchar(10);
    dcl-s wkObjName   varchar(128);
    dcl-s wkLibName   varchar(128);

    outSUCCESS = 0;      // Initialize return code to 0=Not deleted
    indy_outSUCCESS = 0;
    outSQLSTATE = '00000';
    clear outSQLMSG;

         // PROTECT option: *YES/1/Y/T/P = protect system libraries
    if (indy_inPROTECT >= 0 and inPROTECT <> '');
        wkOption = %upper(%trimL(inPROTECT : '* '));
        if (wkOption <> '');
            protect = (%SUBST(wkOption : 1 : 1) in
                       %list('1' : 'Y' : 'T' : 'P'));
        endif;
    endif;

         // CONVERT_NAME option: *YES = convert names to upper case
    if (indy_inCVTCASE >= 0 and inCVTCASE <> '');
        wkOption = %upper(%trimL(inCVTCASE : '* '));
        if (wkOption <> '');
            cvtCase = (%SUBST(wkOption : 1 : 1) in
                       %list('1' : 'Y' : 'T' : 'P'));
        endif;
    endif;

    wkObjName = inOBJNAME;
    wkLibName = inLIBNAME;

    if (cvtCase);  // "Double quoted" names are never converted
        if (wkObjName <> '' and %subst(wkObjName : 1 : 1) <> '"');
            wkObjName = %upper(wkObjName);
        endif;
        if (wkLibName <> '' and %subst(wkLibName : 1 : 1) <> '"');
            wkLibName = %upper(wkLibName);
        endif;
    endif;

         // Object type is always upper case; leading asterisk optional
    objType = '*' + %upper(%trimL(inOBJTYPE : '* '));

         // Convert potential long SQL names to 10-char system names
    if (NOT getObjSysName(objName : libName :
                          wkObjName : wkLibName));
        return;   // Error was resignaled by getObjSysName
    endif;

    if (indy_inASPNAME >= 0 and inASPNAME <> '');
        aspName = %upper(inASPNAME);
    endif;

         // RMVMSG option: *YES/1/Y/S/R = remove "Object deleted" msg
    if (indy_inRMVMSG >= 0 and inRMVMSG <> '');
        wkOption = %upper(%trimL(inRMVMSG : '* '));
        rmvMsg = '0';
        if (wkOption <> '');
            if (%SUBST(wkOption : 1 : 1) in %list('1' : 'Y' : 'S' : 'R'));
                rmvMsg = '1';
            endif;
        endif;
    endif;

    if (protect and %subst(libName : 1 : 5) <> 'QTEMP');
        if (%SUBST(libName : 1 : 4) = 'QSYS' or
            %SUBST(libName : 1 : 3) = 'SYS'  or
            %SUBST(libName : 1 : 3) = 'SQL'  or
            %SUBST(libName : 1 : 1) = 'Q');
            snd-msg %trimR(psds.Pgmname) + ': ' + %trimR(wkObjName) +
                    ' in ' + %trimR(wkLibName) + ' type ' +
                    %trimR(objType) + ' is in a protected library.' +
                    ' Delete request cancelled.';
            return;
        endif;
    endif;

    if (%SUBST(objName : 1 : 4) = '*ALL');
        snd-msg %trimR(psds.Pgmname) +
                ': OBJECT_NAME=>*ALL is not permitted by this' +
                ' routine. The delete request is cancelled.';
        return;
    endif;

    qualObjName = objName;
    %subst(qualObjName : 11 : 10) = libName;
    outSUCCESS = deleteObject(qualObjName : objType : aspName : rmvMsg);
    return;

end-proc;

    ////////////////////////////////////////////////////////////////////////
    // getObjSysName - Convert long SQL object/library names to the
    // corresponding 10-character system names via the QDBRTVSN API.
    // Returns *ON = success, *OFF = error (errors are resignaled).
    ////////////////////////////////////////////////////////////////////////
dcl-proc getObjSysName;
    dcl-pi getObjSysName ind;
        outObject    char(10);
        outLibrary   char(10);
        inLongObject varchar(128) const;
        inLongLibrary varchar(128) const;
    end-pi;

    dcl-ds ec likeds(QUSEC_T) inz(*LIKEDS);
    dcl-s rtnName  char(20);
    dcl-s shortObj char(10);
    dcl-s shortLib char(10);

         // Resolve a long library name first (skip if it contains '*')
    if (%len(inLongLibrary) > 10 and %scan('*' : inLongLibrary) = 0);
        reset ec;
        QDBRTVSN(rtnName : inLongLibrary :
                 %len(inLongLibrary) : ' ' : ec);
        if (ec.bytes_returned > 0);
            resignalMsg(ec);
            return *off;
        endif;
             // Library name is in positions 1 to 10 of the result
        shortLib = %subst(rtnName : 1 : 10);
    else;
        shortLib = inLongLibrary;
    endif;

         // Resolve a long object name (skip if it contains '*')
    if (%len(inLongObject) > 10 and %scan('*' : inLongObject) = 0);
        reset ec;
        QDBRTVSN(rtnName : inLongObject :
                 %len(inLongObject) : shortLib : ec);
        if (ec.bytes_returned > 0);
            resignalMsg(ec);
            return *off;
        endif;
             // Object in pos 1-10, library in pos 11-20; copy the
             // library too in case *CURLIB/*LIBL were resolved.
        shortObj = %subst(rtnName : 1 : 10);
        shortLib = %subst(rtnName : 11 : 10);
    else;
        shortObj = inLongObject;
    endif;
    if (%SUBST(shortObj : 1 : 1) <> '"');
        outObject = %upper(shortObj);
    else;
        outObject  = shortObj;
    endif;
    if (%SUBST(shortLib : 1 : 1) <> '"');
        outLibrary = %upper(shortLib);
    else;
        outLibrary = shortLib;
    endif;

    return *on;
end-proc;

    ////////////////////////////////////////////////////////////////////////
    // deleteObject - Deletes an IBM i object via the QLIDLTO API.
    // Returns 1 if deleted, 0 otherwise (API errors are resignaled).
    ////////////////////////////////////////////////////////////////////////
dcl-proc deleteObject;
    dcl-pi deleteObject int(10);
        qualObjName char(20) const;
        objType     char(10) const;
        aspName     char(10) const;
        rmvMsg      char(1)  const;
    end-pi;

    dcl-ds ec likeds(QUSEC_T) inz(*LIKEDS);

    if (qualObjName = *blanks);
        return 0;
    endif;

    QLIDLTO(qualObjName : objType : aspName : rmvMsg : ec);
    if (ec.bytes_returned > 0);
        resignalMsg(ec);
        return 0;
    endif;
    return 1;
end-proc;

    ////////////////////////////////////////////////////////////////////////
    // resignalMsg - Re-send an IBM i API error as a program message.
    ////////////////////////////////////////////////////////////////////////
dcl-proc resignalMsg;
    dcl-pi resignalMsg;
        apiError likeds(QUSEC_T) const;
    end-pi;

    dcl-ds fc likeds(QUSEC_T) inz(*LIKEDS);
    dcl-s msgFile    char(20);
    dcl-s msgType    char(10);
    dcl-s msgKey     char(4) inz(*blanks);
    dcl-s msgPrefix  char(2);
    dcl-s msgDataLen int(10);

    if (apiError.bytes_returned = 0);
        return;
    endif;

    msgPrefix = %subst(apiError.msgid : 1 : 2);

    select;  // Guess the MSGF since QUSEC does not contain it
        when msgPrefix = 'RN';
            msgFile = 'QRPGLEMSG';
            %subst(msgFile : 11 : 10) = 'QDEVTOOLS';
        when msgPrefix = 'HT';
            msgFile = 'QHTTPMSG';
            %subst(msgFile : 11 : 10) = 'QHTTPSVR';
        when msgPrefix = 'CE';
            msgFile = 'QCEEMSG';
            %subst(msgFile : 11 : 10) = 'QSYS';
        when msgPrefix = 'GU';
            msgFile = 'QGUIMSG';
            %subst(msgFile : 11 : 10) = 'QSYS';
        when msgPrefix = 'IW';
            msgFile = 'QIWSMSG';
            %subst(msgFile : 11 : 10) = 'QSYS';
        other;      // CP and everything else use QCPFMSG
            msgFile = 'QCPFMSG';
            %subst(msgFile : 11 : 10) = '*LIBL';
    endsl;

    select;
        when %subst(apiError.msgid : 3 : 1) = 'E';
            msgType = '*ESCAPE';
        when %subst(apiError.msgid : 3 : 1) = 'D';
            msgType = '*DIAG';
        when %subst(apiError.msgid : 3 : 1) = 'C';
            msgType = '*COMP';
        other;      // F, I and everything else
            msgType = '*INFO';
    endsl;

    msgDataLen = apiError.bytes_returned - 16;
    if (msgDataLen < 0);
        msgDataLen = 0;
    endif;
    if (msgDataLen > %size(apiError.msgdata));
        msgDataLen = %size(apiError.msgdata);
    endif;

    QMHSNDPM(apiError.msgid : msgFile : apiError.msgdata :
             msgDataLen : msgType : '*' : 1 : msgKey : fc);
end-proc;

