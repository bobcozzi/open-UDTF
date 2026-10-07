**FREE

// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 by R. Cozzi, Jr.


         // @author BobCozzi

    ////////////////////////////////////////////////////////////////////////
         // IBM i Retrieve File Attributes
         // This is an SQL UDTF External Program
         // It uses the Qp0lGetAttr API to retreive a Files attributes
           // This information is currently not available via QSYS2 SQL functions
    ////////////////////////////////////////////////////////////////////////
         // This is part of the collection of open source SQL UDTFs that are
         // primarily built for the VS CODE and CODE for IBM i IDE, however
           // they can certainly be feely used in production environments on IBM i
         // Available on github at: https://github.com/bobcozzi/open-UDTF
    ////////////////////////////////////////////////////////////////////////

ctl-opt   main(main) OPTION(*SRCSTMT);

/if defined(*CRTBNDRPG)
ctl-opt DFTACTGRP(*NO) ACTGRP(*CALLER);
/endif

         // Use the PSDS to obtain "this" Program's name
dcl-Ds psds psds qualified;
    Pgmname *proc;
end-Ds;

         // SQL UDF Stuff
dcl-C SQL_FETCH 0;
dcl-C SQL_OPEN -1;
dcl-C SQL_CLOSE 1;


         // Convert to hex from char MI inst.
dcl-Pr cvthc extProc('cvthc');
    Outhexval char(65534) OPTIONS(*VARSIZE);
    inCharVal char(32766) OPTIONS(*VARSIZE)  CONST;
    Hexlen int(10) Value;
end-Pr;
         // Convert to char from hex MI inst.
dcl-Pr cvtch extProc('cvtch');
    Outcharval char(32766) OPTIONS(*VARSIZE);
    inHexVal  char(65534) OPTIONS(*VARSIZE) CONST;
    Hexlen int(10) Value;
end-Pr;

     // SQL UDTF Scratch pad passed to this program
         // Note all Scratch pads have their defined length
         // as the first entry (first 4-byte int)
dcl-Ds scratch_t Qualified Template;
    Length int(10); // Length of this Scratch Pad;
    eof    int(10);  // 1=EOF has been reached
    Msgid char(7);
end-Ds;
dcl-S scratch_len int(10);

         // Qp0lGetAttr() API attribute selection constants
dcl-C QP0L_ATTR_OBJTYPE 0;
dcl-C QP0L_ATTR_DATA_SIZE 1;
dcl-C QP0L_ATTR_ALLOC_SIZE 2;
dcl-C QP0L_ATTR_EXTENDED_ATTR_SIZE 3;
dcl-C QP0L_ATTR_CREATE_TIME 4;
dcl-C QP0L_ATTR_ACCESS_TIME 5;
dcl-C QP0L_ATTR_CHANGE_TIME 6;
dcl-C QP0L_ATTR_MODIFY_TIME 7;
dcl-C QP0L_ATTR_STG_FREE 8;
dcl-C QP0L_ATTR_CHECKED_OUT 9;
dcl-C QP0L_ATTR_LOCAL_REMOTE 10;
dcl-C QP0L_ATTR_AUTH 11;
dcl-C QP0L_ATTR_FILE_ID 12;
dcl-C QP0L_ATTR_ASP 13;
dcl-C QP0L_ATTR_DATA_SIZE_64 14;
dcl-C QP0L_ATTR_ALLOC_SIZE_64 15;
dcl-C QP0L_ATTR_USAGE_INFORMATION 16;
dcl-C QP0L_ATTR_PC_READ_ONLY 17;
dcl-C QP0L_ATTR_PC_HIDDEN 18;
dcl-C QP0L_ATTR_PC_SYSTEM 19;
dcl-C QP0L_ATTR_PC_ARCHIVE 20;
dcl-C QP0L_ATTR_SYSTEM_ARCHIVE 21;
dcl-C QP0L_ATTR_CODEPAGE 22;
dcl-C QP0L_ATTR_FILE_FORMAT 23;
dcl-C QP0L_ATTR_UDFS_DEFAULT_FORMAT 24;
dcl-C QP0L_ATTR_JOURNAL_INFORMATION 25;
dcl-C QP0L_ATTR_ALWCKPWRT 26;
dcl-C QP0L_ATTR_CCSID 27;
dcl-C QP0L_ATTR_SIGNED 28;
dcl-C QP0L_ATTR_SYS_SIGNED 29;
dcl-C QP0L_ATTR_MULT_SIGS 30;
dcl-C QP0L_ATTR_DISK_STG_OPT 31;
dcl-C QP0L_ATTR_MAIN_STG_OPT 32;
dcl-C QP0L_ATTR_DIR_FORMAT 33;
dcl-C QP0L_ATTR_AUDIT 34;
dcl-C QP0L_ATTR_CRTOBJSCAN 35;
dcl-C QP0L_ATTR_SCAN 36;
dcl-C QP0L_ATTR_SCAN_INFO 37;
dcl-C QP0L_ATTR_ALWSAV 38;
dcl-C QP0L_ATTR_RSTDRNMUNL 39;
dcl-C QP0L_ATTR_JOURNAL_EXTENDED_INFORMATION 40;
dcl-C QP0L_ATTR_CRTOBJAUD 41;
dcl-C QP0L_ATTR_SYSTEM_USE 42;
dcl-C QP0L_ATTR_TEMPORARY 43;
dcl-C QP0L_ATTR_UDFS_TEMPORARY 44;
dcl-C QP0L_ATTR_UDFS_PREFERRED_STORAGE_UNIT 45;
dcl-C QP0L_ATTR_INHERIT_ALWCKPWRT 46;
dcl-C QP0L_ATTR_SYS_RESTRICTS_SAVE 47;
dcl-C QP0L_ATTR_TEXT  48;
dcl-C QP0L_ATTR_RESET_DATE 200;
dcl-C QP0L_ATTR_SUID 300;
dcl-C QP0L_ATTR_SGID 301;

         // Qp0lGetAttr()/Qp0lSetAttr() follow-symlink indicators
dcl-C QP0L_DONOT_FOLLOW_SYMLNK 0;
dcl-C QP0L_FOLLOW_SYMLNK 1;

         // Qp0lGetAttr() yes/no indicators
dcl-C QP0L_NO 0;
dcl-C QP0L_YES 1;

         // Qp0lGetAttr() storage-free indicators
dcl-C QP0L_SYS_NOT_STG_FREE 0;
dcl-C QP0L_SYS_STG_FREE 1;

         // Qp0lGetAttr() checked-out indicators
dcl-C QP0L_NOT_CHECKED_OUT 0;
dcl-C QP0L_CHECKED_OUT 1;

         // Qp0lGetAttr()/Qp0lSetAttr() PC attribute indicators
dcl-C QP0L_PC_NOT_READONLY 0;
dcl-C QP0L_PC_READONLY 1;
dcl-C QP0L_PC_NOT_HIDDEN 0;
dcl-C QP0L_PC_HIDDEN 1;
dcl-C QP0L_PC_NOT_SYSTEM 0;
dcl-C QP0L_PC_SYSTEM 1;
dcl-C QP0L_PC_NOT_CHANGED 0;
dcl-C QP0L_PC_CHANGED 1;

         // Qp0lGetAttr()/Qp0lSetAttr() system attribute indicators
dcl-C QP0L_SYSTEM_NOT_CHANGED 0;
dcl-C QP0L_SYSTEM_CHANGED 1;

dcl-C QLG_CHAR_SINGLE 0; // path is char and delimiter is 1 char;
dcl-C QLG_PTR_SINGLE 1; // path is ptr and delimiter is 1 char;
dcl-C QLG_CHAR_DOUBLE 2; // path is char and delimiter is 2 chars;
dcl-C QLG_PTR_DOUBLE 3; // path is ptr and delimiter is 2 char2;

dcl-C UTF8 1208;
dcl-S EpochTM timestamp inz(Z'1970-01-01-00.00.00');

dcl-pr errNo pointer extProc('__errno');
end-pr errNo;
dcl-pr strerror pointer extProc('strerror');
    errno int(10) Value;
end-pr;
dcl-s nErrNo int(10)  BASED(pErrNo);

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

dcl-Ds Qp0l_AttrTypes_List_T Qualified Template Inz;
    Number_ofReqAttrs uns(10);
    Attrtype uns(10) Dim(31);
end-Ds;
dcl-Ds attr_type likedS(Qp0l_AttrTypes_List_T) Inz;
dcl-s  fileSuffix varchar(20) inz('.FILE');
dcl-s  utf8File varchar(20) ccsid(UTF8);

dcl-Ds Qlg_Path_Name_T Qualified Inz TEMPLATE;
    CCSID INT(10) Inz(UTF8);            // Use Unicode (UTF-16)
    Country_ID  char(2) inz(*allX'00');  // Use job country/region
    Language_ID char(3) inz(*allX'00'); // Use job language;
    Reserved CHAR(3) Inz(*ALLX'00');
    Path_Type UNS(10) Inz(QLG_PTR_SINGLE);
    Path_Length INT(10);
    Path_Name_Delimiter CHAR(2) inz(X'2F');
    Reserved2 CHAR(10) Inz(*ALLX'00');
    Ptr_PATH pointer inz(*NULL); // recommend using pointer to the path;
             // char_PATH_NAME char(4096); // 4k max char path name .
end-Ds; // Qlg_Path_Name_T;

dcl-ds Qp0l_Usage_t Qualified Inz TEMPLATE;
    dcl-subf resetDate uns(10);
    dcl-subf usedDate uns(10);
    dcl-subf usedDays uns(5);
    dcl-subf reserved1 char(6);
end-ds;

dcl-Pr Qp0lGetAttr int(10) extproc('Qp0lGetAttr');
    Path_Name            likeds(Qlg_Path_Name_T) OPTIONS(*VARSIZE);
    Attr_Array           likeds(Qp0l_AttrTypes_List_t) OPTIONS(*VARSIZE);
    Buffer_ptr           char(32765) OPTIONS(*VARSIZE); // Returned parm;
    Buffer_Size          uns(10) value;
    Buffer_Size_Avail    uns(10); // Returned parm;
    Buffer_Size_Returned uns(10); // Returned parm;
    Follow_Symlnk        uns(10) value;
    moreParms char(10) options(*NOPASS:*VARSIZE);
end-Pr;

dcl-s  pRtnAttr pointer;
dcl-Ds rtnAttr likeDS(rtnAttr_T) based(pRtnAttr);
dcl-Ds rtnAttr_t Qualified Inz TEMPLATE;
    offset int(10);
    Attrid int(10);
    Attrlen int(10);  // Lenght of attribute data
    Reserved1 char(4);
    AttrData char(256);
    attrUTF8 char(100) CCSID(UTF8) Overlay(AttrData);
    Attrchar50 char(50) Overlay(AttrData);
    attrChar10 char(10) Overlay(AttrData);
    Attrchar1 char(1) Overlay(AttrData);
    attrTime uns(10) Overlay(AttrData);
    Attrint4 int(10) Overlay(AttrData);
    Attrint2 int(5) Overlay(AttrData);
    Attruint4 uns(10) Overlay(AttrData);
    Attruint2 uns(5) Overlay(AttrData);
    last_used Likeds(Qp0l_Usage_t) overlay(attrData);
end-Ds ;


  // NOTE: Need to use the SQL preprocessor just to support the LOB parmaeter
dcl-Proc main ;
    dcl-pi main EXTPGM('FILE_ATTR');

             // Input parameters
        inLibname VARCHAR(10) const;
        inSrcfile VARCHAR(10) const;
        inSrcmbr VARCHAR(10) const;
        inPATH_NAME VARCHAR(4096) CCSID(UTF8) OPTIONS(*VARSIZE);
        inCvtopt VARCHAR(10) const;

        outCreated  TIMESTAMP(0);
        outAccessed TIMESTAMP(0);
        outCHANGED  TIMESTAMP(0);
        outData_CHANGED TIMESTAMP(0);
        outLast_USED    TIMESTAMP(0);
        outLast_USED_DAYS  INT(10);
        outLast_USED_RESET TIMESTAMP(0);
        outAllocated_SIZE  INT(20);
        outData_SIZE       INT(20);
        outccsid  INT(10);
        outASPNbr int(5);
        outText   VARCHAR(50) CCSID(UTF8);
        outOBJTYPE VARCHAR(10);

             // Input indicators
        Indy_LIBNAME int(5);
        Indy_SRCFILE int(5);
        Indy_SRCMBR int(5);

        Indy_PATH int(5);
        Indy_CVTOPT int(5);

        Indy_outCREATED INT(3);
        Indy_outACCESSED INT(3);
        Indy_outCHANGED INT(3);
        Indy_outDATA_CHANGED INT(3);
        Indy_outLAST_USED INT(3);
        Indy_outLAST_USED_DAYS INT(3);
        Indy_outLAST_USED_RESET INT(3);
        Indy_outALLOCATED_SIZE INT(3);
        Indy_outDATA_SIZE INT(3);
        Indy_outCCSID  INT(3);
        indy_outASPNbr INT(3);
        Indy_outTEXT   INT(3);
        indy_outOBJTYPE  INT(3);

             // Standard DB2SQL scratchpad/diagnostic fields
        outSQLSTATE CHAR(5);
        funcname VARCHAR(517) CONST; // Function name;
        specname VARCHAR(128) CONST; // Specific function name;
        outSQLMSG VARCHAR(70);
        Scratch likeDS(scratch_t) OPTIONS(*VARSIZE);
        SQLOpCode INT(10) CONST; // -1=Open, 0=Fetch, 1=Close;

    end-pi;

     // Local API error info and returned member buffer
    dcl-ds ec likeDS(qusec_t) inz(*LIKEDS);
    dcl-s  rc int(10);
    dcl-ds PATH likeDS(Qlg_Path_Name_T) inz(*LIKEDS);
    dcl-s bytesReturned uns(10);
    dcl-s bytesAvail    uns(10);
    dcl-s rtnBuffer CHAR(4096);

    snd-msg 'Starting ' + %trimR(psds.pgmname) +
            ' with SQL Opcode=' + %char(sqlOpcode);
    if (sqlOpCode = SQL_OPEN);
        Scratch_Len = scratch.length;
        Clear scratch;
        Scratch.length = scratch_len;
        return;
    endif;

    if (scratch.eof = 1);
        snd-msg 'Leaving FILE_ATTR with eof=1';
        outSQLState = '02000';
        Clear scratch;
        Return;
    endif;

    if (sqlOpCode = SQL_FETCH);
        Scratch.eof = 1;
        clear Attr_Type;
        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_CREATE_TIME;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_ACCESS_TIME;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_CHANGE_TIME;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_MODIFY_TIME;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
                QP0L_ATTR_USAGE_INFORMATION;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_ASP;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_CCSID;

        Attr_type.Number_ofReqAttrs += 1;
        Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_TEXT;

        utf8File = fileSuffix;
        if (%scan(utf8File : %upper(inPATH_NAME)) = 0);
           Attr_type.Number_ofReqAttrs += 1;
           Attr_type.AttrType(attr_type.Number_ofReqAttrs) =
               QP0L_ATTR_OBJTYPE;
        else;
          outOBJTYPE = '*FILE';
        endif;

        Path.ptr_Path = %Addr(inPATH_NAME : *DATA) ;
        Path.Path_Length = %len(inPATH_NAME);
        snd-msg 'Path Length=' + %char(Path.Path_Length);

        rc = Qp0lGetAttr( path : attr_type :
                    rtnBuffer : %size(rtnBuffer) :
                    bytesAvail :
                    bytesReturned : QP0L_FOLLOW_SYMLNK);

        If (bytesReturned > 0);
            snd-msg 'Received ' + %char(bytesReturned) + ' from Qp0lGetAttr())';
            pRtnAttr = %addr(rtnBuffer);
        else;
            pErrNo = errNo();
            snd-msg 'Qp0lGetAttr returned ' + %Char(nErrNo) + ' - ' +
                        %str(strerror( nErrNo ));

            snd-msg 'Nothing received from Qp0lGetAttr()). Setting SQLState=02000';
            outSQLState = '02000';
            scratch.eof = 1;
            return;
        endIf;

        Dow rtnAttr.AttrLen > 0 or rtnAttr.Offset > 0;
            If rtnAttr.AttrLen <= 0;
                outSQLMSG = 'Attr ' + %char(rtnAttr.attrID) +
                   ' is not supported by file.';
                snd-msg outSQLMSG;
            Else;
                select;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_CREATE_TIME;
                        snd-msg 'QP0L_ATTR_CREATE_TIME';
                        monitor;
                            outCREATED = EpochTM + %SECONDS(rtnAttr.Attrtime);
                        on-error;
                            indy_outCREATED = -1;
                        endmon;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_CHANGE_TIME;
                        snd-msg 'QP0L_ATTR_CHANGE_TIME';
                        monitor;
                            outCHANGED = EpochTM + %SECONDS(rtnAttr.Attrtime);
                        on-error;
                            indy_outCHANGED = -1;
                        endmon;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_MODIFY_TIME;
                        snd-msg 'QP0L_ATTR_MODIFY_TIME';
                        monitor;
                            outCHANGED = EpochTM + %SECONDS(rtnAttr.Attrtime);
                        on-error;
                            indy_outCHANGED = -1;
                        endmon;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_ACCESS_TIME;
                        snd-msg 'QP0L_ATTR_ACCESS_TIME';
                        monitor;
                            outACCESSED = EpochTM + %SECONDS(rtnAttr.Attrtime);
                        on-error;
                            indy_outACCESSED = -1;
                        endmon;

                    WHEN rtnAttr.AttrID = QP0L_ATTR_USAGE_INFORMATION;
                        snd-msg 'QP0L_ATTR_USAGE_INFORMATION';
                        outLAST_USED_DAYS = rtnAttr.last_used.usedDays;
                        monitor;
                            outLAST_USED = EpochTM +
                                    %SECONDS(rtnAttr.last_used.usedDate);
                        on-error;
                            indy_outLAST_USED = -1;
                        endmon;
                        monitor;
                            outLAST_USED_RESET = EpochTM +
                                    %SECONDS(rtnAttr.last_used.resetDate);
                        on-error;
                            indy_outLAST_USED_RESET = -1;
                        endmon;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_ASP;
                        snd-msg 'QP0L_ATTR_ASP';
                        outASPNbr = rtnAttr.Attruint2;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_CCSID;
                        snd-msg 'QP0L_ATTR_CCSID';
                        outCCSID = rtnAttr.attruint4;
                    WHEN rtnAttr.AttrID = QP0L_ATTR_OBJTYPE;
                        snd-msg 'QP0L_ATTR_OBJTYPE';
                        outOBJTYPE =
                        %subst(rtnAttr.attrChar10 : 1 : rtnAttr.attrLen);
                    WHEN rtnAttr.AttrID = QP0L_ATTR_TEXT;
                        snd-msg 'QP0L_ATTR_TEXT length(' +
                                %char(rtnAttr.Attrlen) + ')';
                        outText = rtnAttr.attrUTF8 ;
                        outText = %TrimR(outText);
                endSL;
            endif;
            if (rtnAttr.Offset = 0);
                leave;  // end of attributes
            endif;
            pRtnAttr = %addr(rtnBuffer) + rtnAttr.offset;
        endDo;
    endif;
end-proc;
