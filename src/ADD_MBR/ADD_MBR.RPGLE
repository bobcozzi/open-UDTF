**FREE

// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 by R. Cozzi, Jr.


    ////////////////////////////////////////////////////////////////////////
    // IBM i Add a member to a soruce file
    // This is an SQL UDTF External Program
    ////////////////////////////////////////////////////////////////////////
    // This is part of the collection of open source SQL UDTFs that are
    // primarily built for the VS CODE and CODE for IBM i IDE, however
    // they can be feely used in production environments on IBM i.
    // Available from @BobCozzi at: https://github.com/bobcozzi/open-UDTF
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

            // Path case conversion option
dcl-c  TOUPPER 1;
dcl-c  TOLOWER 2;
dcl-c  NOCONVERT 0;


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


dcl-c IFS_FILE Const(1);
dcl-c IFS_FILES Const(1);
dcl-c IFS_DIR Const(2);
dcl-c IFS_DIRS Const(2);
dcl-c IFS_ALL Const(8);
dcl-c R_OK Const(4);
dcl-c W_OK Const(2);
dcl-c X_OK Const(1);
dcl-c F_OK Const(0);

    // *  seek() mode values
dcl-c SEEK_SET Const(0);
dcl-c SEEK_CUR Const(1);
dcl-c SEEK_END Const(2);

     // *  fOpen() read/write mode values
dcl-c f_READ_ONLY Const('r');
dcl-c f_WRITE_ONLY Const('w');
dcl-c f_APPEND Const('a');
dcl-c f_BINARY Const('b');
dcl-c f_READ_WRITE Const('rw');
dcl-c f_APPEND_BINARY Const('ab');

dcl-c O_RDONLY Const(1);
dcl-c O_WRONLY Const(2);
dcl-c O_RDWR Const(4);
dcl-c O_CREAT Const(8);
dcl-c O_EXCL Const(16);
dcl-c O_CCSID Const(32);
dcl-c O_TRUNC Const(64);
dcl-c O_APPEND Const(256);
dcl-c O_SYNC Const(1024);
dcl-c O_DSYNC Const(2048);
dcl-c O_RSYNC Const(4096);
dcl-c O_NOCTTY Const(32768);
dcl-c O_SHARE_RDONLY Const(65536);
dcl-c O_SHARE_WRONLY Const(131072);
dcl-c O_SHARE_RDWR Const(262144);
dcl-c O_SHARE_NONE Const(524288);
dcl-c O_CODEPAGE Const(8388608);
dcl-c O_TEXT_CREAT Const(33554432);
dcl-c O_INHERITMODE Const(134217728);
dcl-c O_TEXTDATA Const(16777216);
dcl-c O_LARGEFILE Const(536870912);

    // * Object Authority Flags

dcl-c S_IXOTH Const(1);
dcl-c S_IWOTH Const(2);
dcl-c S_IROTH Const(4);
dcl-c S_IRWXO Const(7);

dcl-c S_IXGRP Const(8);
dcl-c S_IWGRP Const(16);
dcl-c S_IRGRP Const(32);
dcl-c S_IRWXG Const(56);

dcl-c S_IXUSR Const(64);
dcl-c S_IWUSR Const(128);
dcl-c S_IRUSR Const(256);
dcl-c S_IRWXU Const(448);

dcl-c S_ISVTX Const(512);
dcl-c S_ISGID Const(1024);
dcl-c S_ISUID Const(2048);

    dcl-C QP0L_ATTR_TEXT 48;
    dcl-C QP0L_DONOT_FOLLOW_SYMLNK 0;
    dcl-C QP0L_FOLLOW_SYMLNK 1;

dcl-C QLG_CHAR_SINGLE 0; // path is char and delimiter is 1 char;
dcl-C QLG_PTR_SINGLE 1; // path is ptr and delimiter is 1 char;
dcl-C QLG_CHAR_DOUBLE 2; // path is char and delimiter is 2 chars;
dcl-C QLG_PTR_DOUBLE 3; // path is ptr and delimiter is 2 char2;

dcl-C UTF8 1208;
dcl-S EpochTM timestamp inz(Z'1970-01-01-00.00.00');

    /SET CCSID(*CHAR:*UTF8)
        dcl-s UTF8_DOT varchar(10) inz('.');
    /RESTORE CCSID(*CHAR)
    dcl-s suffix_Pos int(10);
    dcl-s srcType varchar(10);
    dcl-s path_srcType varchar(10);

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

dcl-DS Qp0l_AttrTypes_List_T Qualified Template Inz;
    Number_ofReqAttrs uns(10);
    Attrtype uns(10) Dim(31);
end-DS;

dcl-DS attr_type likedS(Qp0l_AttrTypes_List_T) Inz;
dcl-s fileSuffix varchar(20) inz('.FILE');
dcl-s utf8File varchar(20) ccsid(UTF8);
dcl-s utf8Path varchar(4096) ccsid(UTF8);
dcl-s utf8NullBlank CHAR(2) CCSID(UTF8) INZ(X'2000');
dcl-s cvtOpt char(1);
dcl-s isFile Ind inz(*OFF);
dcl-s textDesc varchar(80);

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

dcl-ds Qp0l_QSYS_Info_t Qualified Template Inz;
    Bytes_Returned uns(10);      // bytes actually returned to caller
    Bytes_Available uns(10);     // bytes total available
    CCSID_Out uns(10);           // CCSID of names and types returned
    Lib_Name char(28);           // Name of library
    Lib_Type char(20);           // Type of library
    Obj_Name char(28);           // Name of object
    Obj_Type char(20);           // Type of object
    Mbr_Name char(28);           // Name of member
    Mbr_Type char(20);           // Type of member
    Asp_Name char(28);           // Name of ASP
end-ds;

dcl-Ds chgAttr_t Qualified Inz TEMPLATE;
    offset    int(10);   // Offset to next attritube
    AttrId    int(10);   // Attribute ID
    Attrlen   int(10);  // Lenght of attribute data
    Reserved1 char(4)   inz(*ALLX'00');
    AttrData  char(256);
    attrText  char(100) CCSID(UTF8) Overlay(AttrData);  // UTF-8 CCSID(1208)
end-Ds ;

dcl-Ds chgAttr likeDS(chgAttr_t) Inz;

dcl-Pr Qp0lSetAttr int(10) extproc('Qp0lSetAttr');
    Path_Name            likeds(Qlg_Path_Name_T) OPTIONS(*VARSIZE);
    attrs                char(32765) OPTIONS(*VARSIZE); // Returned parm;
    Buffer_Size          uns(10) value;
    Follow_Symlnk        uns(10) value;
    moreParms char(10) options(*NOPASS:*VARSIZE);
end-Pr;

dcl-pr Qp0lCvtPathToQSYSObjName extProc('Qp0lCvtPathToQSYSObjName');
    Path_Name            likeds(Qlg_Path_Name_T) OPTIONS(*VARSIZE);
    rtnQSYSName          likeds(Qp0l_QSYS_Info_t) OPTIONS(*VARSIZE);
    apiFormat            char(8) const; // QSYS0100
    rtnBufferLen         uns(10) value;
    preferred_CCSID      uns(10) value;
    errorDs              LikeDS(QUSEC_T) OPTIONS(*VARSIZE);
end-pr;

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


dcl-pr openFile int(10) extProc(*CWIDEN :'QlgOpen64');
    Path_Name     likeds(Qlg_Path_Name_T) OPTIONS(*VARSIZE);
    openFlags     int(10) Value;
    authMode      uns(10) Value options(*nopass);
    oCCSID        uns(10) Value options(*nopass);
    xCCSID        uns(10) Value options(*nopass);
end-pr;

dcl-pr closeFile int(10) extProc(*CWIDEN :'close');
    hFile int(10) Value;
end-pr ;

// QUSRMBRD API Interfaces
dcl-ds Qdb_Mbrd0100_T  Qualified Inz TEMPLATE;
    Bytes_Returned INT(10);
    Bytes_Available INT(10);
    Db_File_Name CHAR(10);
    Db_File_Lib CHAR(10);
    Member_Name CHAR(10);
    File_Attr CHAR(10);
    Src_Type CHAR(10);
    Crt_Date CHAR(13);
    Src_Change_Date CHAR(13);
    Text_Desc CHAR(50);
    Src_File CHAR(1);
            // END of MBRD0100
end-ds;

dcl-pr QUSRMBRD EXTPGM('QUSRMBRD');
    rtnMBRINFO char(1024) OPTIONS(*VARSIZE);
    rtnMbrInfoSize int(10) Const;
    APIFORMAT  char(8) Const;
    FileLib    char(20) Const;
    MbrName    char(10) Const;
    OVR        char(1)  Const;
    apiError   LikeDS(QUSEC_T) OPTIONS(*VARSIZE:*NOPASS);
    FindMbr    char(1)  Const OPTIONS(*NOPASS);
end-pr;

 dcl-pr qcmdexc extpgm('QCMDEXC');
   cmdString  char(32702)  Const OPTIONS(*VARSIZE);
   cmdLength  packed(15:5) Const;
   cmdIGC     char(3)      Const OPTIONS(*NOPASS);
 end-pr;

     // SQL UDTF Scratch pad passed to this program
         // Note all Scratch pads have their defined length
         // as the first entry (first 4-byte int)
dcl-Ds scratch_t Qualified Template;
    Length int(10);  // Length of this Scratch Pad;
    eof    int(10);  // 1=EOF has been reached
    cvtCase int(5);  // 0=No, 1=Uppercase, 2=Lowercase
    reserved char(6);
end-Ds;

dcl-S scratch_len int(10);

  // NOTE: Need to use the SQL preprocessor just to support the LOB parmaeter
dcl-Proc main ;
    dcl-pi main EXTPGM('ADD_MBR');

             // Input parameters
        inPATH_NAME VARCHAR(4096) CCSID(UTF8) OPTIONS(*VARSIZE);
        inSRCTYPE  VARCHAR(10) const;
        inMBRTEXT  VARCHAR(50) const CCSID(UTF8);


        outLIBNAME VARCHAR(10);
        outOBJNAME VARCHAR(10);
        outOBJTYPE VARCHAR(10);
        outMBRNAME VARCHAR(10);
        outSRCTYPE VARCHAR(10);
        outMbrText VARCHAR(50) CCSID(UTF8);
        outASPNAME VARCHAR(10);

               // UDTF interface checklist (DB2SQL parameter style):
               // 1) Null indicators must be 2-byte (SMALLINT) -> RPG INT(5).
               // 2) Set indicator to -1 for NULL and 0 for NOT NULL.
               // 3) SQL SCRATCHPAD nnn and RPG scratch DS layout must stay aligned.
               // 4) Keep trailing DB2SQL parms order: SQLSTATE, function/specific,
               //    message, scratch, SQL opcode.

             // Input indicators
        indy_PATH_NAME int(5);
        indy_SRCTYPE   int(5);
        indy_MBRTEXT   int(5);

        indy_outLIBNAME INT(5);
        indy_outOBJNAME INT(5);
        indy_outOBJTYPE INT(5);
        indy_outMBRNAME INT(5);
        indy_outSRCTYPE INT(5);
        indy_outMbrText INT(5);
        indy_outASPNAME INT(5);


             // Standard DB2SQL scratchpad/diagnostic fields
        outSQLSTATE CHAR(5);
        funcname VARCHAR(517) CONST; // Function name;
        specname VARCHAR(128) CONST; // Specific function name;
        outSQLMSG VARCHAR(70);
        scratch likeDS(scratch_t) OPTIONS(*VARSIZE);
        SQLOpCode INT(10) CONST; // -1=Open, 0=Fetch, 1=Close;

    end-pi;

     // Local API error info and returned member buffer
    dcl-ds ec likeDS(qusec_t) inz(*LIKEDS);
    dcl-s  fd int(10);
    dcl-s  flags int(10);
    dcl-ds MBRDESC likeDS(Qdb_Mbrd0100_T) Inz;
    dcl-ds PATH likeDS(Qlg_Path_Name_T) inz(*LIKEDS);
    dcl-ds savedPATH likeDS(Qlg_Path_Name_T) inz(*LIKEDS);
    dcl-ds qsys_Name likeDS(Qp0l_QSYS_Info_t) Inz;
    dcl-s  fileName char(20);
    dcl-s  mbrName  char(10);
    dcl-s bytesReturned uns(10);
    dcl-s bytesAvail    uns(10);
    dcl-s rtnBuffer CHAR(4096);

    if (sqlOpCode = SQL_OPEN);
        Scratch_Len = scratch.length;
        clear scratch;
        Scratch.length = scratch_len;
        return;
    endif;

    if (sqlOpCode = SQL_CLOSE);
        Return;
    endif;

    if (scratch.eof = 1);
        outSQLState = '02000';
        Return;
    endif;


    if (sqlOpCode = SQL_FETCH);
        Scratch.eof = 1;
        clear outSQLMSG;
        outSQLSTATE = '00000';

        Path.ptr_Path = %Addr(inPATH_NAME : *DATA) ;
        Path.Path_Length = %len(inPATH_NAME);

        flags = O_APPEND + O_CREAT + O_INHERITMODE + O_RDWR;

            // retrieve the /QSYS.LIB path name's suffix
            // if not .MBR, then use the suffix as the source type
            // when the SRCTYPE parameter is not specified.

        suffix_Pos = %ScanR(UTF8_DOT : inPATH_NAME);
        path_srcType = %SUBST(inPATH_NAME : suffix_Pos);
        path_srcType = %UPPER(%trimR(path_SrcType));
            // Swap the source type suffix for a valid .MBR suffix
        if (%UPPER(path_srcType) <> '.MBR');
          %subst(inPATH_NAME : suffix_Pos : %len(path_srcType)) = '.MBR';
        endif;

            // Add Member
        fd = openFile( path : flags );

        If (fd < 0);
            pErrNo = errNo();
            snd-msg %TRIMR(psds.Pgmname) + ' openFile ' + %Char(nErrNo) + ' - ' +
                        %str(strerror( nErrNo ));
            snd-msg 'Path_name=>''' + %trim(inPATH_NAME) + '''';

            outSQLState = '02000';
            scratch.eof = 1;
            return;
        endIf;

        if (closeFile( fd ) <> 0);
            pErrNo = errNo();
            snd-msg %TRIMR(psds.Pgmname) + ' closeFile(' + %char(fd) + ') ' +
                        %Char(nErrNo) + ' - ' +%str(strerror( nErrNo ));
        endif;
        reset ec;


            // Change Member Text
        chgAttr.AttrId = QP0L_ATTR_TEXT;
        chgAttr.Attrlen  = %len(inMBRTEXT);
        chgAttr.attrText = %TrimR(inMBRTEXT);

        if (Qp0lSetAttr( path : chgAttr :
                         %size( chgAttr) : QP0L_FOLLOW_SYMLNK ) <> 0);
            pErrNo = errNo();
            snd-msg %TRIMR(psds.Pgmname) + ' Qp0lSetAttr(' + %char(fd) + ') ' +
                        %Char(nErrNo) + ' - ' +%str(strerror( nErrNo ));

        endif;

            // Get member attributes
        Qp0lCvtPathToQSYSObjName( Path : QSYS_Name : 'QSYS0100' :
                                  %size(QSYS_Name) : 0 : ec);

        // The lib, file, mbr, type, text and ASP are returned null-terminated,
        // I use %TRIMR to remove those X'00' values when copied to the result.
        if (ec.Bytes_returned = 0);
            if (QSYS_NAME.Lib_Name = *ALLX'00');
                indy_outLIBNAME = -1;
                clear outLIBNAME;
            else;
                outLIBNAME = %trimR(QSYS_Name.Lib_Name : X'00');
                indy_outLIBNAME = 0;
            endif;

            if (QSYS_NAME.Obj_Name = *ALLX'00');
                indy_outOBJNAME = -1;
                clear outOBJNAME;
            else;
                outOBJNAME = %trimR(QSYS_Name.Obj_Name : X'00');
                indy_outOBJNAME = 0;
            endif;

            if (QSYS_NAME.Obj_Type = *ALLX'00');
                indy_outOBJTYPE = -1;
                clear outOBJTYPE;
            else;
                outOBJTYPE = %trimR(QSYS_Name.Obj_Type : X'00');
                indy_outOBJTYPE = 0;
            endif;

            if (QSYS_NAME.Mbr_Name = *ALLX'00');
                indy_outMBRNAME = -1;
                clear outMBRNAME;
                indy_outSRCTYPE = -1;
                clear outSRCTYPE;
            else;
                outMBRNAME = %trimR(QSYS_Name.Mbr_Name : X'00');
                indy_outMBRNAME = 0;
                    // Set (change) the Member's SEU/Source Type
                path_srcType = %TRIML(path_srcType : '.');

                if (indy_SRCTYPE < 0 or inSRCTYPE = '' or
                    %upper(inSRCTPE) in %LIST('*PATH'));
                    if (%upper(path_srcType) <> 'MBR' and %len(path_srcType) > 0);
                      srcType = path_srcType;
                    else;
                        clear srcType;
                    endif;
                else;
                    srcType = inSrcType;
                endif;
                    // Set source type only when SRCTYPE is not empty
                if (srcType <> '');
                    setSrcType( outLIBNAME : outOBJNAME : outMBRNAME : srcType );
                endif;
                reset ec;
                fileName = %trimR(outOBJNAME);
                %SUBST(fileName : 11 : 10) = %TrimR(outLIBNAME);
                mbrName  = %trimR(outMBRNAME);
                // Retrieve member source type
                QUSRMBRD( MBRDESC : %size(MBRDESC) : 'MBRD0100' :
                        fileName : mbrName : '1' : ec : '1');
                if (mbrDesc.Bytes_returned > 8);
                    outSRCTYPE = mbrDesc.Src_Type;
                    outMbrText = mbrDesc.Text_Desc;
                endif;

            endif;

            if (QSYS_NAME.ASP_Name = *ALLX'00');
                indy_outASPNAME = -1;
                clear outASPNAME;
            else;
                outASPNAME = %trimR(QSYS_Name.ASP_Name : X'00');
                indy_outASPNAME = 0;
            endif;
        endif;

        outSQLSTATE = '00000';
        return;
    endif;
end-proc;

    dcl-proc setSrcType;
        dcl-pi setSrcType;
          srcFile varchar(10) const;
          srcLib  varchar(10) const;
          srcMbr  varchar(10) const;
          srcType varchar(10) const;
          srcText varchar(50) const OPTIONS(*NOPASS:*OMIT);
          expDate date(*ISO)  const OPTIONS(*NOPASS:*OMIT);
        end-pi;

        dcl-s chgpfm varchar(256);
        dcl-c quote const('''');
        dcl-c doubleQuote const( '''''' );
        dcl-s src_type varchar(10);
        if (%upper(srcType) in %LIST('*BLANKS':'*BLANK'));
            src_Type = '*NONE';
        else;
            src_Type = %UPPER(srcType);
        endif;

        chgpfm = 'CHGPFM FILE(' + %trimR(srcFile) + '/' +
                                  %trimR(SrcLib) + ') ' +
                        ' MBR(' + %trimR(srcMbr) + ') ' +
                        ' SRCTYPE(' + %trimR(src_Type) + ') ';

        if (%PARMS() >= %ParmNum(SrcText) and %addr(srcText) <> *NULL);
            chgpfm += ' TEXT(''' +
                            %scanRpl(quote : doubleQuote : srcText) +
                        ''')';
        endif;
        if (%PARMS() >= %ParmNum(expDate) and %addr(expDate) <> *NULL);
            chgpfm += ' EXPDATE(' + %Char(expDate : *JOBRUN) + ')';
        endif;
        snd-msg 'Changing MbrDesc: ' + %trimR(chgPFM);
            // There is no API or other interface to set/change the SEU Type
            // Therefore I use QCMDEXC to invoke the CHGPFM CL Command
        monitor;
         qcmdexc( chgpfm : %len(chgpfm) );
        on-error;
            snd-msg 'Failed to run: ' + %trimR(chgpfm);
        endmon;
        return;
    end-proc;

