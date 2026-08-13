// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 by R. Cozzi, Jr.


  // @author BobCozzi

    ////////////////////////////////////////////////////////////////////////
    // IBM i CL Command Processor for SQL
    // This is an SQL UDTF External Program
    // It uses the QCAPCMD API to run or syntax check CL commands
    // in various IBM i environments (OPM, ILE, CL Program, Command Line, etc.)
    ////////////////////////////////////////////////////////////////////////
    // This is part of the collection of open source SQL UDTFs that are
    // primarily built for the VS CODE and CODE for IBM i IDE, however
    // they can certainly be feely used in products such as:
    //  - IBM ACS RUNSQL Scripts
    //  - SQL iQuery
    //  Available on github at:  https://github.com/bobcozzi/open-UDTF
    ////////////////////////////////////////////////////////////////////////

#ifndef __STDC_WANT_DEC_FP__
#define __STDC_WANT_DEC_FP__
#endif

#ifndef __POSIX_LOCALE__
#define __POSIX_LOCALE__
#endif


#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#pragma datamodel(P128)

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <langinfo.h>
#include <except.h>
#include <signal.h>

#include <QP0LSTDI.h>
#include <QP0ZTRC.h>     // Qp0zLprintf

#include <qusec.h>
#include <qlg.h>
#include <qlgcase.h>
#include <letype.h>
#include <lecond.h>
#include <leenv.h>

#include <QMH.h>
#include <QMHSNDPM.h>
#include <QMHRCVPM.h>
#include <QMHRTVM.h>

#include <qcapcmd.h>

  // include syntax when compiling from an IFS stream file
#include <mih/cpybytes.h>
#include <mih/triml.h>
#include <mih/MATPGMNM.h>


#include <cstring>
#include <cctype>
#include <string>
#include <memory>

#include <vector>
#include <algorithm>

using namespace std;

#include <sql.h>
#include <sqludf.h>

const int coz_MAXUSRSPACESIZE = 16711568;
const int coz_MAXMEM15M = 15662992;
/* Case-conversion direction constants */
#ifndef _TOUPPER
#define _TOUPPER 0
#endif
#ifndef _TOLOWER
#define _TOLOWER 1
#endif

/* ============================================= */
/* coz_qusec - IBM i standard error code wrapper */
/* ============================================= */

typedef struct tag_QUS_EC
 {
    union { int  Bytes_Provided; int  length; };
    union { int  Bytes_Available; int  Bytes_Returned; };
    union { char Exception_Id[7]; char msgid[7]; };
    char Reserved;
    union { char Exception_Data[255]; char msgdta[255]; };
 } qusec_t;


class coz_qusec {
public:
    coz_qusec() { init(); }
    void init() {
        memset((char*)&ec, 0x00,
               sizeof(ec) + sizeof(xtra));
        ec.Bytes_Provided =
            (int)(sizeof(ec) + sizeof(xtra));
    }
    void clear() {
        init();
        ec.Bytes_Provided = 0;
    }
    void reset() { init(); }
    int isEmpty() {
        return ec.Bytes_Available == 0;
    }
    int isNotEmpty() {
        return ec.Bytes_Available != 0;
    }
    int hasError() {
        return ec.Bytes_Available > 0;
    }
    int isError() {
        return ec.Bytes_Available > 0;
    }
    int hasNoError() {
        return ec.Bytes_Available == 0;
    }
    /* Compare Exception_Id up to 7 chars */
    bool compare(const char* pMsgID) {
        if (ec.Bytes_Available == 0)
            return false;
        int n = (int)strlen(pMsgID);
        if (n > 7) n = 7;
        return (memcmp(ec.Exception_Id,
                       pMsgID, n) == 0);
    }
    char* msgid() {
        return ec.Exception_Id;
    }
    bool startsWith(const char* prefix) {
        return strncmp(ec.Exception_Id, prefix, std::min<int>(7,std::strlen(prefix))) == 0;
    }
    char* msgdata() {
        int avail = ec.Bytes_Available;
        if (avail > 16) {
            char* p = (char*)&ec + 16;
            int dlen = avail - 16;
            if (dlen < (int)sizeof(xtra))
                p[dlen] = 0x00;
            return p;
        }
        return NULL;
    }
    int getMsgDataLen() {
        int a = ec.Bytes_Available;
        return (a > 16) ? a - 16 : 0;
    }
    operator void*()     { return &ec; }
    operator Qus_EC_t*() {
        return (Qus_EC_t*)&ec;
    }
private:
    qusec_t ec;
    char    xtra[256];
};


bool startsWith(const char* str, const char* prefix) {
    return strncmp(str, prefix, std::strlen(prefix)) == 0;
}

int makeUpper(char* szData, int inLen = 0, int ccsid = 0)
{

  Qlg_CCSID_ReqCtlBlk_T frcb;
  coz_qusec      ec;
  long       len = 0;
  char*      pIn = szData;
  char*      pOut= szData;

  memset((char*)&frcb,0x00,sizeof(frcb));
  frcb.Type_of_Request = 1;
  frcb.Case_Request = _TOUPPER;
  frcb.CCSID_of_Input_Data = ccsid;

  if (inLen <= 0)
  {
    len = strlen(pIn);
  }
  else
  {
    len = inLen;
  }

  ec.init();
  if (len > 0)
  {
    QlgConvertCase( (char*) &frcb, (char*)pIn,
                     pOut, &len ,
                    (char*) &ec);
  }
  return len;
}


int makeLower(char* szData, int inLen = 0, int ccsid = 0)
{

  Qlg_CCSID_ReqCtlBlk_T frcb;
  coz_qusec ec;
  long       len = 0;
  char*      pIn = szData;
  char*      pOut= szData;

  memset((char*)&frcb,0x00,sizeof(frcb));
  frcb.Type_of_Request = 1;
  frcb.Case_Request = _TOLOWER;  // 0=Upper 1=Lower
  frcb.CCSID_of_Input_Data = ccsid;

  if (inLen <= 0)
  {
    len = strlen(pIn);
  }
  else
  {
    len = inLen;
  }

  ec.init();
  if (len > 0)
  {
    QlgConvertCase( (char*) &frcb, (char*)pIn,
                     pOut, &len ,
                    (char*) &ec);
  }
  return len;
}



inline char* coz_getNextParmIf(int& pC, int& argc, char** argv, int ioFlag = 0);


#define inChar(_v) \
          char *in##_v = \
          (char*) coz_getNextParmIf(p,argc,argv)
#define inInt(_v)  \
          int   *in##_v = \
          (int *)coz_getNextParmIf(p, argc, argv)

#define outChar(_v) \
           char  *out##_v = \
           (char *)coz_getNextParmIf(p, argc, argv, 1)

#define outSmallInt(_v) \
           short *out##_v = \
           (short *)coz_getNextParmIf(p, argc, argv, 2)
#define outInt(_v)   \
           int  *out##_v = \
           (int *)coz_getNextParmIf(p, argc, argv)
#define outBigInt(_v) \
           long long *out##_v = \
           (long long *)coz_getNextParmIf(p, argc, argv, 8)

#define inIndy(_v) \
    short *indyIn##_v = \
        (short*) coz_getNextParmIf(p,argc,argv)
#define outIndy(_v) \
    short *indy##_v = \
        (short*) coz_getNextParmIf(p,argc,argv,2)

#define cpyFixedToStr(dest, src, srclen)        \
    do {                                        \
        size_t i_copy = 0;                      \
        for (; i_copy < (srclen); ++i_copy) {   \
            if ((src)[i_copy] == '\0') break;   \
            (dest)[i_copy] = (src)[i_copy];     \
        }                                       \
        (dest)[i_copy] = '\0';                  \
    } while (0)

    // copyPad copies a fixed src to a fixed length target with blank padding
inline void copyPad(char* dest, const char* src, int maxLen = 10)
{
    memset(dest, ' ', maxLen);
    _CPYBYTES(dest, src, maxLen);
}

    // copyFromStr copies a C-string to a fixed length target but
    // copies no more than maxLen bytes.
inline void copyFromStr(char* dest, const char* src, int maxLen = 10)
{
    memset(dest, ' ', maxLen);
    _CPYBYTES(dest, src, std::min<int>(strlen(src), maxLen));
}


#define makeAPIObjName(_out, _obj, _lib) \
 memset(_out, ' ', 10); \
 _CPYBYTES(_out, _obj, std::min<int>(10,strlen(_obj))); \
 memset(_out+10, ' ', 10); \
 _CPYBYTES(_out+10, _lib, std::min<int>(10,strlen(_lib)))


#define MAX_MSGTEXT_LEN 1024  // Limit message text to the first 1k of data
#define MAX_SECLVL_LEN  2048  // Limit seclvl text to the first 2k of data
#define MAX_MSGKEY_COUNT 256  // Limit msgkeys saved and messages returned

typedef _Packed struct tagScratch
{
    int  len;  // scratch pad length as set by the SCRATCHPAD kwd in the UDTF
    char pgmName[11];
    char libName[11];
    int eof;
    int syntaxOption;  // Check or Run QCAPCMD runtype control option
    int msgcount;
    int current_msg;
    char msgkey[MAX_MSGKEY_COUNT][4];
} scratch_t;


int getNextCmdMsg(scratch_t* pScratch, char* msgid, int* msgsev, char* pMsgType, char* msgtext, char* seclvl);
inline int saveAllMsgKeys(scratch_t* pScratch);
int getMsgType(char* msgType, const char* msgtypeID);
void logCtrlBlock(const Qca_PCMD_CPOP0100_t& ctrlBlock);
int xlateCheckOption(const char* pProcessMode);

    // Prototype for retrieve Message text
void rtvMsgText(char* pMsgText, int bufLen, coz_qusec& ec);


int main(int argc, char *argv[])
{
     /**********************************************************/
     /* Syntax Check or run CL commands via QCAPCMD API        */
     /**********************************************************/

     int p = 0;

     //////////////////////////////////////////////
     //  PARMS Parameter declaration section
     //////////////////////////////////////////////

     //////////////////////////////////////////////
     //  INPUT Parameters
     //////////////////////////////////////////////
     inChar(CMD);
     inChar(PROCESSMODE);

     //////////////////////////////////////////////
     //  OUTPUT Parameters
     //////////////////////////////////////////////
     outSmallInt(ORDPOS);  // IBM's "ORDINAL_POSITION" column for sequencing help
     outChar(MSGID);
     outInt(MSGSEV);
     outChar(MSGTYPE);
     outChar(MSGTEXT);   // Varchar(1024)
     outChar(SECLVL);    // Varchar(2048)

     //////////////////////////////////////////////
     //  Input Indicator Fields
     //////////////////////////////////////////////
     inIndy(CMD);
     inIndy(PROCESSMODE);
        // Options may be any 1 of the following:
        // Note upper/lower case and leading asterisk are ignored.
        //   NULL or empty = Command Entry (non-program) CL (default)
        //   *CL   - Command Entry (non-program) CL
        //   *CLLE - ILE CL Program syntax
        //   *CLP  - OPM CL Program syntax
        //   *LIMIT - Limited User Commands
        //   *CMD  - Command Definition Commands
        //   *BND  - Binder Language Commands

     //////////////////////////////////////////////
     //  Output Indicator Fields
     //////////////////////////////////////////////
    outIndy(ORDPOS);
    outIndy(MSGID);      // set to 0 by the macro
    outIndy(MSGSEV);
    outIndy(MSGTYPE);
    outIndy(MSGTEXT);    // 1st Level message text
    outIndy(SECLVL);     // 2nd level message test

     ////////////////////////////////////////////////////////////
     //  SQL specific parameters
     ////////////////////////////////////////////////////////////
     char *sqlstate     = (char *)coz_getNextParmIf(p, argc, argv);
     char *funcName     = (char *)coz_getNextParmIf(p, argc, argv);
     char *specificName = (char *)coz_getNextParmIf(p, argc, argv);
     char *sqlmsgtext   = (char *)coz_getNextParmIf(p, argc, argv);
     char *scratchPad   = (char *)coz_getNextParmIf(p, argc, argv);
     int  *sqlOpCode    = (int  *)coz_getNextParmIf(p, argc, argv);

     ////////////////////////////////////////////////////////////
     //  BEGIN main() body (after parms starts here)
     ////////////////////////////////////////////////////////////

    scratch_t* pScratch = (scratch_t *)scratchPad;

    _MATPGMNM_Template_T  pgm;
    memset(&pgm,0x00,sizeof(pgm));

      const int            MSGTEXT_LEN = 1024;
      char                 APIFMT[] = "CPOP0100";
      int                  ctlType = 0;
      int                  ctlInv = 0;

      _FEEDBACK            fc;
      coz_qusec           ec;

       if (*sqlOpCode == SQLUDF_TF_OPEN)
       {
        if (pScratch != NULL) {
            const int len = pScratch->len;
            if (len < (int) sizeof(scratch_t)) {
              Qp0zLprintf("[%s] Warning Scratch Pad Length=%d expected length=%d\n",funcName,len, (int) sizeof(scratch_t));
              Qp0zLprintf("Change %s UDTF SCRATCHPAD keyword to at least %d bytes\n",funcName, (int) sizeof(scratch_t));
              strcpy(sqlstate,"02000");
              return 0;
            }
            // Clear entire scratch pad buffer passed in (even if longer than our typedef)
            memset(pScratch, 0x00, len);
            pScratch->len = len;          // re-assign the saved scratch pad length to the len member variable.
        }
        else {
            Qp0zLprintf("[%s] Warning Scratch Pad Length is 0 or NULL expected length=%d\n",funcName, (int) sizeof(scratch_t));
            Qp0zLprintf("Change %s UDTF SCRATCHPAD keyword to at least %d bytes\n",funcName, (int) sizeof(scratch_t));
            strcpy(sqlstate,"02000");
            return 0;
        }

        pgm.Template_Size = sizeof(pgm);
        _MATPGMNM( &pgm );
        copyPad(pScratch->pgmName,pgm.Format_0.Name.Name,10);
        copyPad(pScratch->libName,pgm.Format_0.Context.Name,10);

    Qca_PCMD_CPOP0100_t  ctrlBlock;
    int                  ctrlBlockLen = sizeof(ctrlBlock);

    memset((char*)&ctrlBlock,0x00,sizeof(ctrlBlock));

    ctrlBlock.Command_Process_Type = 0;     // DFT: Mimic QCMDEXC
    ctrlBlock.DBCS_Data_Handling = '0';     // 0 = Ignore DBCS, 1 = Handle DBCS
    ctrlBlock.Prompter_Action = '0';        // 0 = Never prompt the command
    ctrlBlock.Command_String_Syntax = '0';  // 0 = IBM i syntax, 1 = System/38
    ctrlBlock.CCSID_Command_String = 0;
    memset(ctrlBlock.Message_Key,' ', sizeof(ctrlBlock.Message_Key));


     int cmdLen = strlen(inCMD);   // Input CMD string length
     cmdLen = ::triml(inCMD, ' ');  // Get length of command (blank trimmed-right)
    // while (cmdLen > 0 && inCMD[cmdLen-1] == ' ') --cmdLen;

      // Calculate length for updated CMD string workspace
     int  rtnUpdatedCmdLen = 0;
     int  returnedCmdBufferLen = 0; // Not used/Deprecated: was = MAXCMD_LEN;
     char returnedCmdString[256];   // Not used/Deprecated was [MAXCMD_LEN]
     char* pRtnCmd = returnedCmdString;

     ctrlBlock.Command_Process_Type = 3;  // DFT(Command Entry Syntax checking)

     if (*indyInPROCESSMODE >= 0 && strlen(inPROCESSMODE) > 0)
     {
        while (*inPROCESSMODE == ' ' || *inPROCESSMODE == '*') {
               ++inPROCESSMODE;
        }
        char cmdCheckOption[64];
        strcpy(cmdCheckOption, inPROCESSMODE);
        makeUpper(cmdCheckOption);

        pScratch->syntaxOption = xlateCheckOption(cmdCheckOption);

        ctrlBlock.Command_Process_Type = pScratch->syntaxOption;

      }

      // logCtrlBlock(ctrlBlock);  // Used for development only


#pragma exception_handler(MONMSG, 0, 0, _C2_MH_ESCAPE | _C2_MH_FUNCTION_CHECK,\
                          _CTLA_HANDLE )

      ec.clear();  // runtime errors get swallowed by the ec parameter
                   // so we clear it (set Bytes_Provided = 0) so all
                   // errors are pushed to the joblog. Ironically,
                   // this does not cause a runtime error for the QCAPCMD API.
                   // Likely because we used an exception_handler monitor.
        // RUN CL Command using QCAPCMD
      QCAPCMD(inCMD,
              cmdLen,
              &ctrlBlock,
              ctrlBlockLen,
              APIFMT,
              returnedCmdString,
              returnedCmdBufferLen,
              &rtnUpdatedCmdLen,
              &ec);

MONMSG:
        // When an error occurs on the API call that is something like a parameter error,
        // then do your handler coding here and then "goto CONTINUE;"
        Qp0zLprintf("[CMD_RUN] Detected an Error. See low-level messages in joblog for details\n");

#pragma disable_handler

CONTINUE:

      // Fetch and save the first 200 message keys to the scratch pad
      if (saveAllMsgKeys(pScratch) == 0)
      {
        pScratch->eof = 1;
        strcpy(sqlstate,"02000");
      }

    }  // end SQLUDF_TF_OPEN

    if (pScratch->eof != 0) {
      strcpy(sqlstate,"02000");
    }


      // SQLUDF_TF_FETCH  - F E T C H
    if (*sqlOpCode == SQLUDF_TF_FETCH)
    {
        // Qp0zLprintf("[CMD_RUN] FETCH detects %d msgkeys\n", pScratch->msgcount);
        if (!getNextCmdMsg(pScratch, outMSGID, outMSGSEV, outMSGTYPE, outMSGTEXT, outSECLVL))
        {
            pScratch->eof = 1;
            strcpy(sqlstate,"02000");
            return 0;
        }
        *outORDPOS = pScratch->current_msg;  // Ordinal is a 1's based value
    }
    if (*sqlOpCode == SQLUDF_TF_CLOSE)
    {
        // Add normal cleanup code here.
    }
    if (*sqlOpCode == SQLUDF_TF_FINAL ||
        *sqlOpCode == SQLUDF_TF_FINAL_CRA)
    {
        // Release non-SQL resources.
        // On FINAL_CRA, do not issue SQL other than CLOSE cursor.

        // If this function later allocates resources that require cleanup after
        // an normal or abnormal termination, handle SQLUDF_TF_FINAL and SQLUDF_TF_FINAL_CRA,
        // then change the UDTF defintion from "NO FINAL CALL" to "FINAL CALL".
    }

}  // end main

void logCtrlBlock(const Qca_PCMD_CPOP0100_t& ctrlBlock)
{
    const char lf = 0x25;

    Qp0zLprintf("[CMD_RUN] ctrlBlock.Command_Process_Type = %d%c",
                ctrlBlock.Command_Process_Type, lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.DBCS_Data_Handling = '%c' (0x%02X)%c",
                ctrlBlock.DBCS_Data_Handling,
                (unsigned int)(unsigned char)ctrlBlock.DBCS_Data_Handling,
                lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.Prompter_Action = '%c' (0x%02X)%c",
                ctrlBlock.Prompter_Action,
                (unsigned int)(unsigned char)ctrlBlock.Prompter_Action,
                lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.Command_String_Syntax = '%c' (0x%02X)%c",
                ctrlBlock.Command_String_Syntax,
                (unsigned int)(unsigned char)ctrlBlock.Command_String_Syntax,
                lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.Message_Key = [%02X %02X %02X %02X]%c",
                (unsigned int)(unsigned char)ctrlBlock.Message_Key[0],
                (unsigned int)(unsigned char)ctrlBlock.Message_Key[1],
                (unsigned int)(unsigned char)ctrlBlock.Message_Key[2],
                (unsigned int)(unsigned char)ctrlBlock.Message_Key[3],
                lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.CCSID_Command_String = %d%c",
                ctrlBlock.CCSID_Command_String, lf);
    Qp0zLprintf("[CMD_RUN] ctrlBlock.Reserved = [%02X %02X %02X %02X %02X]%c",
                (unsigned int)(unsigned char)ctrlBlock.Reserved[0],
                (unsigned int)(unsigned char)ctrlBlock.Reserved[1],
                (unsigned int)(unsigned char)ctrlBlock.Reserved[2],
                (unsigned int)(unsigned char)ctrlBlock.Reserved[3],
                (unsigned int)(unsigned char)ctrlBlock.Reserved[4],
                lf);
}

int xlateCheckOption(const char* pProcessMode)
{
    int typeCheck = 9;  // Default to ILE CL Program CL Commands

      // Setup up type of syntax checking to perform

  if (strcmp(pProcessMode,"RUN")==0 ||  // Run CL command
      startsWith(pProcessMode,"EXEC"))  // or EXECute CL command
  {
      typeCheck = 0; // Runs a CL Command similar to QCMDEXC
  }
  else if (  // Syntax-check a CL command in a non-CL program environment
    // this is mostly a legacy feature: it mimics the QCMDCHK API syntax checking
    // which only checks commands that would be run via QCMDEXC. Therefore,
    // no embedded CL Variables or string expressions are permitted.
    // The recommended CL programming syntax checking option is *CLLE
        strcmp(pProcessMode,"CHECK")==0    ||
        strcmp(pProcessMode,"CLCHECK")==0  ||
        startsWith(pProcessMode,"CMDCH")   ||   // *CMDCHECK or *CMDCHK
        startsWith(pProcessMode,"CMDENT")  ||   // *CMDENTRY
        startsWith(pProcessMode,"CHK")     ||
        startsWith(pProcessMode,"SYNTAX")  ||
        strcmp(pProcessMode,"QCMDCHK")==0)
  {
      typeCheck = 1;     // 1=Command Entry CL Syntax check (syntax check any CL
  }
  else if (startsWith(pProcessMode,"RUNLIMIT") ||  // Run Limited User Command
           startsWith(pProcessMode,"LIMIT")    ||
           startsWith(pProcessMode,"CMDLINE"))
  {
      typeCheck = 2; // Command Line environment: RUN CL COMMAND/Limited User
  }
         // Check Limited User CL command for syntax errors only
  else if (startsWith(pProcessMode,"CHKL") ||      // *Check Limited user CL command
           startsWith(pProcessMode,"CHECKLIMIT"))

  {
      typeCheck = 3; // Command Line environment: Syntax Check Only
  }
  else if (startsWith(pProcessMode,"CLP")  ||  // *CLP
           startsWith(pProcessMode,"OPM"))     // *CLPGM
  {
      typeCheck = 4; // Syntax Check original ("OPM") CL Program statement
  }
  else if (startsWith(pProcessMode,"CMD"))   // *CMD
  {          // Command Definition statements
      typeCheck = 6;  //  (CMD, PARM, ELEM QUAL, DEP, PMTCTL)
  }
  else if (startsWith(pProcessMode,"BND")   ||  // *BND or *BIND or *BNDSRC
           startsWith(pProcessMode,"BIND"))
  {       // Binder Language CL command Syntax Check
      typeCheck = 7; // STRPGMEXP, EXPORT, ENDPGMEXP
  }
  else if (startsWith(pProcessMode,"PDM")   ||
            startsWith(pProcessMode,"USRDFN") ||
            startsWith(pProcessMode,"USERDEFN"))
  {        // Syntax Check command with PDM user-defined options
      typeCheck = 8; // e.g., CRTBNDPGM PGM(&l/&n) ...
  }
  else if (startsWith(pProcessMode,"CLLE")  ||  // *CLP
           startsWith(pProcessMode,"PGM")   ||  // *PGM
           startsWith(pProcessMode,"ILE"))      // *ILE (e.g., *ILECL)
  {
      typeCheck = 9; // Syntax Check ILE CL Program Statement (this is the default)
  }
  else if (startsWith(pProcessMode,"PMT")   ||  // *PMT
           startsWith(pProcessMode,"PROMPT"))  // *PROMPTER
  {   // 10 is prep CL command for Prompter and should not be used.
      typeCheck = 10; // it is her for completeness purposes only.
  }
  return typeCheck;
}

void rtvMsgText(char* pMsgText, int bufLen, coz_qusec& ec)
{
   char msgfile[21];
   char msgPrefix[3];

   Qus_EC_t  fc;
   memset((char*)&fc, 0x00, sizeof(fc));
   fc.Bytes_Provided = sizeof(fc);
   fc.Bytes_Available = sizeof(fc);


   if (ec.isEmpty())
   {
       return;
   }

   _CPYBYTES(msgPrefix, ec.msgid(), 2);
   msgPrefix[2] = 0x00;

   if (strcmp(msgPrefix,"CP")==0)
   {
      makeAPIObjName(msgfile,"QCPFMSG","*LIBL");
   }
   else if (strcmp(msgPrefix,"RN")==0)
   {
      makeAPIObjName(msgfile,"QRPGLEMSG","QDEVTOOLS");
   }
   else if (strcmp(msgPrefix,"HT")==0)
   {
      makeAPIObjName(msgfile,"QHTTPMSG","QHTTPSVR");
   }
   else if (strcmp(msgPrefix,"CE")==0)
   {
      makeAPIObjName(msgfile,"QCEEMSG","QSYS");
   }
   else if (strcmp(msgPrefix,"GU")==0)
   {
      makeAPIObjName(msgfile,"QGUIMSG","QSYS");
   }
   else if (strcmp(msgPrefix,"IW")==0)
   {
      makeAPIObjName(msgfile,"QIWSMSG","QSYS");
   }
      char APIFMT[9] = "RTVM0100";
      char includeMSGDATA[11];
      char fmtCTL[11];
      char msgid[8];
      char* pMsgData = ec.msgdata();
      int  msgDataLen = ec.getMsgDataLen();
      char buffer[4096];

      memset(pMsgText,0x00, bufLen);
      memset(buffer,0x00, sizeof(buffer));
      copyFromStr(includeMSGDATA,"*YES");
      copyFromStr(fmtCTL,"*NO");
      copyPad(msgid,ec.msgid(),7);

     QMHRTVM(buffer,
             sizeof(buffer),
             APIFMT,
             msgid,
             msgfile,
             pMsgData,
             msgDataLen,
             includeMSGDATA,
             fmtCTL,
             &fc);
     if (fc.Bytes_Available == 0) {
      Qmh_Rtvm_RTVM0100_t* pRtnMsg = (Qmh_Rtvm_RTVM0100_t*) buffer;
      _CPYBYTES(pMsgText, buffer + sizeof(Qmh_Rtvm_RTVM0100_t), pRtnMsg->Length_Message_Returned);
     }
     return;
}

// This function grabs each command generated by the syntax checker
// Note that normally, only the first one (which is usually the last
// syntax error, is returned.
int getNextCmdMsg(scratch_t* pScratch, char* msgid, int* msgsev, char* msgtype, char* msgtext, char* seclvl)
{
  char APIFMT[]  = "RCVM0200";
  char MSGTYPE[] = "*ANY      ";
  char MSGQ[11]  = "*         ";
  char MSGKEY[4] = {0};   // initialized to all x'00'
  const char blanks7[] = "       ";

  int  CALLSTACK = 0;
  int  WAIT = 0;

  char msgBuffer[4096];
  Qmh_Rcvpm_RCVM0200_t* pMsgInfo = (Qmh_Rcvpm_RCVM0200_t*)msgBuffer;
  coz_qusec ec;

  memset(msgBuffer, 0x00, sizeof(msgBuffer));

    if (pScratch->msgcount <= 0) return 0;
    if (pScratch->current_msg >= pScratch->msgcount) return 0;

    int m = pScratch->current_msg++;
    if (m >= MAX_MSGKEY_COUNT)  // MAX_MSGKEY_COUNT - 1 is the array 'index' max to use
    {
      return 0;
    }

    _CPYBYTES(MSGKEY,pScratch->msgkey[m], 4);  // Update to this msgkey

  QMHRCVPM(msgBuffer, sizeof(msgBuffer), QMH_FMT_RCVM0200,
           MSGQ, CALLSTACK,
           MSGTYPE,
           MSGKEY,
           WAIT,
           QMH_MSGACT_OLD, &ec);


  if (ec.hasNoError() && pMsgInfo->Bytes_Returned > 8)
  {
      _CPYBYTES(msgid, pMsgInfo->Message_Id,7);
      *msgsev = pMsgInfo->Message_Severity;
      getMsgType(msgtype, pMsgInfo->Message_Type);

      int off_msgdata = sizeof(Qmh_Rcvpm_RCVM0200_t);  // msgdata and/or impromptu message text
      int off_msgtext = off_msgdata + pMsgInfo->Length_Data_Returned;
      int off_seclvl  = off_msgtext + pMsgInfo->Length_Message_Returned;

      if (memcmp(msgid,blanks7,7) == 0) // ImpromptU message
      {
         _CPYBYTES(msgtext, msgBuffer + off_msgdata, std::min<int>(MAX_MSGTEXT_LEN,pMsgInfo->Length_Data_Returned));
         msgtext[std::min<int>(MAX_MSGTEXT_LEN,pMsgInfo->Length_Data_Returned)] = 0x00;
      }
      if (pMsgInfo->Length_Message_Returned > 0)  // Regular message
      {
         _CPYBYTES(msgtext, msgBuffer + off_msgtext, std::min<int>(MAX_MSGTEXT_LEN,pMsgInfo->Length_Message_Returned));
         msgtext[std::min<int>(MAX_MSGTEXT_LEN,pMsgInfo->Length_Message_Returned)] = 0x00;
      }
      if (pMsgInfo->Length_Help_Returned > 0)  // Second Level Help text
      {
        // 2nd Level (SECLVL) message text. Also referred to as "Message Help"
        _CPYBYTES(seclvl, msgBuffer + off_seclvl, std::min<int>(MAX_SECLVL_LEN,pMsgInfo->Length_Help_Returned));
        seclvl[std::min<int>(MAX_SECLVL_LEN,pMsgInfo->Length_Help_Returned)] = 0x00;
      }
      return 1;
  }
  return 0;
}

// If errors are returned, then save all the message keys.
inline int saveAllMsgKeys(scratch_t* pScratch)
{
  char APIFMT[] = "RCVM0200";
  char MSGKEY[4];
  char MSGTYPE[11] = "*NEXT     ";
  char MSGQ[11]    = "*         ";
  int  CALLSTACK   = 1;   // Must be 1 or it doesn't work (i.e., 0 is wrong)
  int  WAIT = 0;

  char msgBuffer[4096];

  Qmh_Rcvpm_RCVM0200_t* pMsgInfo = (Qmh_Rcvpm_RCVM0200_t*)msgBuffer;
  coz_qusec ec;


  pScratch->msgcount = 0;
  memset(pScratch->msgkey[0],0x00, sizeof(MSGKEY));
  _CPYBYTES(MSGKEY,pScratch->msgkey[0],sizeof(MSGKEY));

  do {
    int m = (pScratch->msgcount > 0) ? pScratch->msgcount-1 : 0;
    if (pScratch->msgcount > 0)
    {
        _CPYBYTES(MSGKEY,pScratch->msgkey[m], 4);  // Update to next msgkey
    }

    memset(msgBuffer, 0x00, sizeof(msgBuffer));

    QMHRCVPM(msgBuffer, sizeof(msgBuffer), QMH_FMT_RCVM0200,
             MSGQ, CALLSTACK,
             MSGTYPE,
             MSGKEY,
             WAIT,
             QMH_MSGACT_OLD, &ec);
    if (ec.hasNoError() && pMsgInfo->Bytes_Returned > 8)
    {   // Save the msgkey
      _CPYBYTES(pScratch->msgkey[pScratch->msgcount], pMsgInfo->Message_Key, 4);
      pScratch->msgcount++;
    }
  } while (ec.hasNoError() && pMsgInfo->Bytes_Returned > 8 && pScratch->msgcount < MAX_MSGKEY_COUNT);
  // Qp0zLprintf("[CMD_RUN] Added %d msgkeys\n", pScratch->msgcount);
  return pScratch->msgcount;
}



/* ============================================= */
/* coz_getNextParmIf                             */
/* Advance parameter counter and return argv[n]. */
/* If ioFlag>0, memset output parm to 0x00.      */
/* ============================================= */
inline char* coz_getNextParmIf(
    int&  pC,
    int&  argc,
    char** argv,
    int    ioFlag)
{
    char* pRtn = NULL;
    if (argc > pC + 1) {
        pRtn = argv[++pC];
        if (ioFlag > 0)
            memset(pRtn, 0x00, ioFlag);
    }
    return pRtn;
}

int getMsgType(char* msgType, const char* msgtypeID)
{
    int typeCode = 0;

    // API returns a 2-byte character code (char[2]); convert it to an int.
    if (msgtypeID != NULL &&
        std::isdigit((unsigned char)msgtypeID[0]) &&
        std::isdigit((unsigned char)msgtypeID[1]))
    {
        typeCode = ((msgtypeID[0] - '0') * 10) + (msgtypeID[1] - '0');
    }

    // Default for unknown/future message types.
    strcpy(msgType,"*UNKNOWN");

    if (typeCode == 1)
    {
        strcpy(msgType,"*COMP");
    }
    else if (typeCode == 2)
    {
        strcpy(msgType,"*DIAG");
    }
    else if (typeCode == 4)
    {
        strcpy(msgType,"*INFO");
    }
    else if (typeCode == 5)
    {
        strcpy(msgType,"*INQ");
    }
    else if (typeCode == 6)
    {
        strcpy(msgType,"*SENDER");
    }
    else if (typeCode == 8)
    {
        strcpy(msgType,"*RQS");
    }
    else if (typeCode == 14)
    {
        strcpy(msgType,"*NOTIFY");
    }
    else if (typeCode == 15)
    {
        strcpy(msgType,"*ESCAPE");
    }

    return typeCode;
}


#pragma datamodel(pop)

