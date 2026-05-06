// COZUTILS.H - Standalone spool-read utility helpers.
// Replaces coz:: namespace for SPOOLDATA standalone UDTF.
// All functions are static inline, using prefix coz_.
// Line length: each line is <=76 visible characters.
// (c) 2025 Robert Cozzi Jr. All rights reserved.

#ifndef _COZUTILS_H_
#define _COZUTILS_H_

#pragma datamodel(P128)

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>
#include <string>

#include <qusec.h>
#include <QUSCRTUS.h>
#include <QUSPTRUS.h>
#include <QLIDLTO.h>
#include <qgyolspl.h>
#include <qgygtle.h>
#include <qgyclst.h>
#include <Qp0ztrc.h>
#include <sqludf.h>
#include <qlgcase.h>
#include <QSPRILSP.h>
#include "QMHSNDPM.H"
#include <cpybytes.mih>
#include <triml.mih>
#include <matpgmnm.mih>

/* Case-conversion direction constants */
#ifndef _TOUPPER
#define _TOUPPER 0
#endif
#ifndef _TOLOWER
#define _TOLOWER 1
#endif

#ifndef ST_QUSEC_T
#define ST_QUSEC_T
typedef struct tag_QUS_EC
 {
    union { int  Bytes_Provided; int  length; };
    union { int  Bytes_Available; int  Bytes_Returned; };
    union { char Exception_Id[7]; char msgid[7]; };
    char Reserved;
    union { char Exception_Data[255]; char msgdta[255]; };
 } qusec_t;
#endif

/* ============================================= */
/* spBufferStatus_t - SCS/AFP buffer I/O status  */
/* ============================================= */
typedef _Packed struct tagsplfbufferCtrl {
    unsigned char cmd;
    unsigned char parm;
    int lineLen;
    int lineFeed;
    int newPage;
} spBufferStatus_t;

/* ============================================= */
/* SCS control code arrays (coz_ namespace)      */
/* ============================================= */
static const unsigned char coz_wCtrl    = 0x34;
static const unsigned char coz_wTRN     = 0x35;
static const unsigned char coz_wTRNEPSON= 0x03;
static const unsigned char coz_cCR      = 0x0D;

static unsigned char coz_wFF[] = {
    0x0C,
    0x3A
};
static unsigned char coz_wLF[] = {
    0x25,
    0x15,
    0x06
};
static unsigned char coz_wSCS[] = {
    0x15, 0x25, 0x0C, 0x0D, 0x16,
    0x2F, 0x1E, 0x28, 0x00, 0x05,
    0x0B, 0x2B, 0x04, 0x14, 0x24,
    0x08, 0x0E, 0x0F, 0x06, 0x09,
    0x23, 0x33, 0x34, 0x35, 0x36,
    0x38, 0x39, 0x41, 0xE1, 0xCA
};
static int coz_s_wFF  =
    (int)(sizeof(coz_wFF)/sizeof(coz_wFF[0]));
static int coz_s_wLF  =
    (int)(sizeof(coz_wLF)/sizeof(coz_wLF[0]));
static int coz_s_wSCS =
    (int)(sizeof(coz_wSCS)/sizeof(coz_wSCS[0]));

/* ============================================= */
/* coz_qusec - IBM i standard error code wrapper */
/* ============================================= */
class coz_qusec {
public:
    coz_qusec() { init(); }
    void init() {
        memset((char*)&ec, 0x00,
               sizeof(ec) + sizeof(xtra));
        ec.Bytes_Provided =
            (int)(sizeof(ec) + sizeof(xtra));
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

/* ============================================= */
/* coz_makeAPIObjName                            */
/* Build 20-char IBM i API object name from      */
/* qualified name (LIB/OBJ or OBJ).              */
/* ============================================= */
static inline void coz_makeAPIObjName(
    char*       objName,
    const char* qualObj,
    const char* dftLib = "*LIBL")
{
    char OBJNAME[11];
    char LIBNAME[11];
    memset(objName,  ' ', 20);
    memset(OBJNAME,  ' ', sizeof(OBJNAME));
    memset(LIBNAME,  ' ', sizeof(LIBNAME));
    OBJNAME[10] = 0x00;
    LIBNAME[10] = 0x00;
    const char* slashPos = strchr(qualObj, '/');
    if (slashPos != NULL) {
        int len = (int)(slashPos - qualObj);
        int olen = (int)strlen(slashPos + 1);
        if (olen > 10) olen = 10;
        if (len  > 10) len  = 10;
        _CPYBYTES(OBJNAME, slashPos + 1, olen);
        _CPYBYTES(LIBNAME, qualObj,      len);
    } else {
        int olen = (int)strlen(qualObj);
        if (olen > 10) olen = 10;
        _CPYBYTES(OBJNAME, qualObj, olen);
        if (dftLib != NULL) {
            int llen = (int)strlen(dftLib);
            if (llen > 10) llen = 10;
            _CPYBYTES(LIBNAME, dftLib, llen);
        } else {
            _CPYBYTES(LIBNAME, "*LIBL", 5);
        }
    }
    _CPYBYTES(objName,      OBJNAME, 10);
    _CPYBYTES(objName + 10, LIBNAME, 10);
}

/* ============================================= */
/* coz_nameUpper                                 */
/* Converts non-quoted IBM i object name to      */
/* upper case in-place.                          */
/* ============================================= */
static inline int coz_nameUpper(
    char* szData,
    int   inLen = -1)
{
    const char q = '"';
    int len = (inLen <= 0)
                  ? (int)strlen(szData) : inLen;
    if (szData[0] == q) return inLen;
    Qlg_CCSID_ReqCtlBlk_T frcb;
    coz_qusec ec;
    long llen = len;
    memset((char*)&frcb, 0x00, sizeof(frcb));
    frcb.Type_of_Request  = 1;
    frcb.Case_Request     = _TOUPPER;
    frcb.CCSID_of_Input_Data = 0;
    if (llen > 0)
        QlgConvertCase((char*)&frcb,
                       szData, szData,
                       &llen, (char*)&ec);
    return len;
}

/* ============================================= */
/* coz_copyPad                                   */
/* Copy s to t, blank-padding to padLen bytes.   */
/* ============================================= */
static inline void coz_copyPad(
    char*       t,
    const char* s,
    int         padLen = 10,
    char        padChar = ' ')
{
    int slen = (s != NULL) ? (int)strlen(s) : 0;
    memset(t, padChar, padLen);
    if (slen > 0)
        _CPYBYTES(t, s,
                  (slen < padLen) ? slen : padLen);
}

/* ============================================= */
/* coz_copyUntil                                 */
/* Copy s to t (null-terminated), stopping at   */
/* maxLen, NUL, or any char in stopAt.           */
/* ============================================= */
static inline int coz_copyUntil(
    char*       t,
    const char* s,
    int         maxLen = 10,
    const char* stopAt = NULL,
    bool        bTrim  = false)
{
    int bStop  = 0;
    int rtnLen = 0;
    int i      = 0;
    if (t == NULL || s == NULL) return 0;
#pragma exception_handler(coz_cu_exc,\
    0, 0,\
    _C2_MH_ESCAPE | _C2_MH_FUNCTION_CHECK,\
    _CTLA_HANDLE)
    memset(t, ' ', maxLen);
    while (s[i] != 0x00 && i < maxLen) {
        if (stopAt != NULL) {
            size_t j;
            for (j = 0; j < strlen(stopAt); j++) {
                if (s[i] == stopAt[j]) {
                    bStop = 1;
                }
            }
        }
        if (bStop) break;
        t[i] = s[i];
        i++;
    }
    if (bTrim) {
        t[i] = 0x00;
        if (i > 0)
            t[::triml(t, ' ')] = 0x00;
        rtnLen = (int)strlen(t);
    } else {
        rtnLen = i;
    }
#pragma disable_handler
    return rtnLen;
coz_cu_exc:;
    return rtnLen;
}

/* ============================================= */
/* coz_copyUntilVar                              */
/* Copy s to VARCHAR struct (length + data).     */
/* ============================================= */
static inline int coz_copyUntilVar(
    struct sqludf_vc_fbd* t,
    const char*           s,
    int                   maxLen,
    bool                  stopAtNull,
    bool                  bTrimBlanks)
{
    int rtnLen = 0;
    int i      = 0;
    memset(t, ' ', maxLen);
    for (i = 0; i < maxLen; i++) {
        if (stopAtNull && s[i] == 0x00) break;
        t->data[rtnLen] = s[i];
        rtnLen++;
    }
    if (bTrimBlanks) {
        while (rtnLen > 0 &&
               t->data[rtnLen - 1] == ' ')
            rtnLen--;
    }
    t->length = rtnLen;
    return rtnLen;
}

/* ============================================= */
/* coz_joblog                                    */
/* Write a formatted message to the IBM i joblog */
/* via Qp0zLprintf.                              */
/* ============================================= */
static inline void coz_joblog(
    const char* fmt, ...)
{
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = 0x00;
    Qp0zLprintf("%s", buf);
}

/* ============================================= */
/* coz_toUpper / coz_toLower                     */
/* In-place EBCDIC case conversion.              */
/* ============================================= */
static inline int coz_toUpper(
    char* szData,
    int   inLen = -1,
    int   ccsid = 0)
{
    Qlg_CCSID_ReqCtlBlk_T frcb;
    coz_qusec ec;
    long len = (inLen <= 0)
               ? (long)strlen(szData) : inLen;
    memset((char*)&frcb, 0x00, sizeof(frcb));
    frcb.Type_of_Request     = 1;
    frcb.Case_Request        = _TOUPPER;
    frcb.CCSID_of_Input_Data = ccsid;
    if (len > 0)
        QlgConvertCase((char*)&frcb,
                       szData, szData,
                       &len, (char*)&ec);
    return (int)len;
}

static inline int coz_toLower(
    char* szData,
    int   inLen = -1,
    int   ccsid = 0)
{
    Qlg_CCSID_ReqCtlBlk_T frcb;
    coz_qusec ec;
    long len = (inLen <= 0)
               ? (long)strlen(szData) : inLen;
    memset((char*)&frcb, 0x00, sizeof(frcb));
    frcb.Type_of_Request     = 1;
    frcb.Case_Request        = _TOLOWER;
    frcb.CCSID_of_Input_Data = ccsid;
    if (len > 0)
        QlgConvertCase((char*)&frcb,
                       szData, szData,
                       &len, (char*)&ec);
    return (int)len;
}

/* ============================================= */
/* coz_getNextParmIf                             */
/* Advance parameter counter and return argv[n]. */
/* If ioFlag>0, memset output parm to 0x00.      */
/* ============================================= */
static inline char* coz_getNextParmIf(
    int&  pC,
    int&  argc,
    char** argv,
    int   ioFlag = 0)
{
    char* pRtn = NULL;
    if (argc > pC + 1) {
        pRtn = argv[++pC];
        if (ioFlag > 0)
            memset(pRtn, 0x00, ioFlag);
    }
    return pRtn;
}

/* ============================================= */
/* coz_getNextParmIntIf                          */
/* Like getNextParmIf but returns int*.          */
/* If ioFlag>0, sets *pRtn = ioFlag.             */
/* ============================================= */
static inline int* coz_getNextParmIntIf(
    int&  pC,
    int&  argc,
    char** argv,
    int   ioFlag = 0)
{
    int* pRtn = NULL;
    if (argc > pC + 1) {
        pRtn = (int*)argv[++pC];
        if (ioFlag > 0)
            *pRtn = ioFlag;
    }
    return pRtn;
}

/* ============================================= */
/* coz_getNextParmIndy                           */
/* Advance counter and return short* (indicator) */
/* initialised to initVal.                       */
/* ============================================= */
static inline short* coz_getNextParmIndy(
    int&  pC,
    int&  argc,
    char** argv,
    short initVal = -1)
{
    short* pRtn = NULL;
    if (argc > pC + 1) {
        pRtn = (short*)argv[++pC];
        *pRtn = initVal;
    }
    return pRtn;
}

/* ============================================= */
/* coz_resignalMsg                               */
/* Re-send an IBM i API error as a program msg.  */
/* ============================================= */
static inline void coz_resignalMsg(coz_qusec& ec)
{
    char msgfile[21];
    char msgtype[11];
    char msgkey[4];
    char topgmq[11];
    char msgPrefix[3];
    char msgType[3];
    qusec_t  fc;
    memset((char*)&fc, 0x00, sizeof(fc));
    fc.Bytes_Provided = sizeof(fc);
    memset(msgkey,  ' ', sizeof(msgkey));
    memset(msgfile, ' ', sizeof(msgfile));
    memset(topgmq,  ' ', sizeof(topgmq));
    coz_copyPad(topgmq, "*", 10);
    if (ec.isEmpty()) return;
    _CPYBYTES(msgPrefix, ec.msgid(), 2);
    _CPYBYTES(msgType,   ec.msgid() + 2, 1);
    msgType[1]   = 0x00;
    msgPrefix[2] = 0x00;
    if (strcmp(msgPrefix, "CP") == 0)
        coz_makeAPIObjName(msgfile,
                           "QCPFMSG", "*LIBL");
    else if (strcmp(msgPrefix, "RN") == 0)
        coz_makeAPIObjName(msgfile,
                           "QRPGLEMSG","QDEVTOOLS");
    else if (strcmp(msgPrefix, "HT") == 0)
        coz_makeAPIObjName(msgfile,
                           "QHTTPMSG", "QHTTPSVR");
    else if (strcmp(msgPrefix, "CE") == 0)
        coz_makeAPIObjName(msgfile,
                           "QCEEMSG", "QSYS");
    else if (strcmp(msgPrefix, "GU") == 0)
        coz_makeAPIObjName(msgfile,
                           "QGUIMSG",  "QSYS");
    else if (strcmp(msgPrefix, "IW") == 0)
        coz_makeAPIObjName(msgfile,
                           "QIWSMSG",  "QSYS");
    else
        coz_makeAPIObjName(msgfile,
                           "QCPFMSG",  "*LIBL");
    switch (msgType[0]) {
        case 'F': case 'I':
            coz_copyPad(msgtype, "*INFO", 10);
            break;
        case 'E':
            coz_copyPad(msgtype, "*ESCAPE", 10);
            break;
        case 'D':
            coz_copyPad(msgtype, "*DIAG", 10);
            break;
        case 'C':
            coz_copyPad(msgtype, "*COMP", 10);
            break;
        default:
            coz_copyPad(msgtype, "*INFO", 10);
            break;
    }
    QMHSNDPM(ec.msgid(), msgfile,
             ec.msgdata(),
             ec.getMsgDataLen(),
             msgtype, topgmq, 1,
             msgkey, &fc);
}

/* ============================================= */
/* coz_getProdLib                                */
/* Return the runtime library of this module.   */
/* Uses _MATPGMNM MI instruction.               */
/* ============================================= */
static inline char* coz_getProdLib(char* p = NULL)
{
    static char prodLib[11];
    _MATPGMNM_Template_T pgm;
    memset(prodLib, ' ', sizeof(prodLib));
    prodLib[10] = 0x00;
    memset((char*)&pgm, 0x00, sizeof(pgm));
    pgm.Template_Size = sizeof(pgm);
    _MATPGMNM(&pgm);
    _CPYBYTES(prodLib,
              pgm.Format_0.Context.Name, 10);
    prodLib[::triml(prodLib, ' ')] = 0x00;
    if (p != NULL)
        strcpy(p, prodLib);
    return prodLib;
}

/* ============================================= */
/* coz_getPtrUsrSpace                            */
/* Return pointer to user space data area.       */
/* ============================================= */
static inline void* coz_getPtrUsrSpace(
    const char* p2PartUsrSpaceName)
{
    void*     pUS = NULL;
    coz_qusec ec;
    ec.init();
    QUSPTRUS((char*)p2PartUsrSpaceName,
             &pUS, &ec);
    if (ec.isEmpty()) return pUS;
    return NULL;
}

/* ============================================= */
/* coz_TEMP_USRSPACE                             */
/* Create a temporary *USRSPC in QTEMP.          */
/* Returns pointer to user-space data area;      */
/* stores 20-char API name in rtnUSName[20].     */
/* ============================================= */
static inline char* coz_TEMP_USRSPACE(
    char*       rtnUSName,
    const char* objAttr = "SPLDATA")
{
    char qualName[21];
    char extAttr[11];
    char pubAut[11];
    char text[51];
    char repl[11];
    char domain[11];
    char initVal = 0x00;
    char* pUS   = NULL;
    int   initSize = 4096;
    coz_qusec ec;
    std::string nm;
    size_t pos = 0;
    memset(qualName, ' ', 20);
    qualName[20] = 0x00;
    nm  = tmpnam(NULL);
    pos = nm.find_last_of("/\\");
    if (pos != std::string::npos)
        nm.erase(0, pos + 1);
    if (nm.length() > 10)
        nm.resize(10);
    coz_makeAPIObjName(
        qualName, nm.c_str(), "QTEMP");
    coz_nameUpper(qualName, 20);
    coz_copyPad(extAttr, objAttr,       10);
    coz_copyPad(pubAut,  "*LIBCRTAUT",  10);
    coz_copyPad(text,
                "SPOOLDATA UDTF Temp Space", 50);
    coz_copyPad(repl,   "*NO",          10);
    coz_copyPad(domain, "*DEFAULT",     10);
    ec.init();
    QUSCRTUS(qualName, extAttr, initSize,
             &initVal, pubAut, text,
             repl, &ec, domain);
    if (ec.isEmpty()) {
        ec.init();
        QUSPTRUS(qualName, (void**)&pUS, &ec);
    }
    if (pUS != NULL && rtnUSName != NULL)
        _CPYBYTES(rtnUSName, qualName, 20);
    return pUS;
}

/* ============================================= */
/* coz_DLTOBJ                                    */
/* Delete an IBM i object via QLIDLTO API.       */
/* ============================================= */
static inline void coz_DLTOBJ(
    const char* qualObjName,
    const char* objType,
    const char* ASPName = "*")
{
    char szObjType[11];
    char szASP[11];
    char rmvMsg[1] = {'1'};
    coz_qusec ec;
    if (qualObjName == NULL  ||
        qualObjName[0] == ' ' ||
        qualObjName[0] == 0x00)
        return;
    coz_copyPad(szObjType, objType,  10);
    coz_copyPad(szASP,     ASPName,  10);
    ec.init();
    QLIDLTO((char*)qualObjName,
            szObjType, szASP,
            rmvMsg, &ec);
}

/* ============================================= */
/* coz_getLastSPLF                               */
/* Return last spooled file in the current job.  */
/* If pSPLFName is not NULL and not *LAST,       */
/* find the most-recent matching file by name.   */
/* Returns total spooled-file record count.      */
/* ============================================= */
static inline int coz_getLastSPLF(
    Qgy_Olspl_RecVar_0300_t* pLastSPLF,
    const char* pSPLFName = NULL)
{
typedef _Packed struct Qgy_Olspl_FI {
    int Num_User_Names;
    Qgy_Olspl_UserInfo_t  Users;
    int Num_OutQ_Names;
    Qgy_Olspl_Qinfo_t     OutQ;
    char Form_Type[10];
    char User_Data[10];
    int Num_Statuses;
    Qgy_Olspl_SFInfo_t    Status;
    int Num_Dev_Names;
    Qgy_Olspl_DevInfo_t   Dev;
} Qgy_Olspl_FI_t;

    coz_qusec ec;
    char APIFMT[]     = "OSPL0300";
    char FILTERFMT[]  = "OSPF0100";
    char sortBuf[80];
    char jobName[26];
    int  rowsToReturn = 1;
    int  splfCount    = 0;
    int  sk = 0;

    Qgy_Olspl_SData_t      SortKey;
    Qgy_Olspl_SortInfo_t   *pSortInfo = NULL;
    Qgy_Olspl_ListInfo_t   openListInfo;
    Qgy_Olspl_ListInfo_t   listInfo;
    Qgy_Olspl_RecVar_0300_t splfInfo;
    Qgy_Olspl_SortInfo_t   SortInfo;
    Qgy_Olspl_FI_t         filterInfo;

    memset(jobName, ' ', sizeof(jobName));
    jobName[0] = '*';
    memset((char*)&openListInfo,0,
           sizeof(openListInfo));
    memset((char*)&listInfo,    0,
           sizeof(listInfo));
    memset((char*)&splfInfo,    0,
           sizeof(splfInfo));
    memset((char*)&sortBuf,     0,
           sizeof(sortBuf));
    memset((char*)&filterInfo,  0,
           sizeof(filterInfo));

    filterInfo.Num_User_Names = 1;
    filterInfo.Num_OutQ_Names = 1;
    filterInfo.Num_Statuses   = 1;
    filterInfo.Num_Dev_Names  = 1;

    {
        int unLen =
            (int)sizeof(filterInfo.Users.User_Name);
        coz_copyPad(
            filterInfo.Users.User_Name,
            "*ALL", unLen);
    }
    {
        int oqLen = 20;
        coz_copyPad(
            filterInfo.OutQ.Output_Q_Name,
            "*ALL", oqLen);
    }
    {
        int stLen =
            (int)sizeof(filterInfo.Status.SF_Status);
        coz_copyPad(
            filterInfo.Status.SF_Status,
            "*ALL", stLen);
    }
    {
        int dnLen =
            (int)sizeof(filterInfo.Dev.Dev_Name);
        coz_copyPad(
            filterInfo.Dev.Dev_Name,
            "*ALL", dnLen);
    }
    coz_copyPad(filterInfo.Form_Type, "*ALL", 10);
    coz_copyPad(filterInfo.User_Data, "*ALL", 10);

    pSortInfo =
        (Qgy_Olspl_SortInfo_t *)sortBuf;
    pSortInfo->Num_Keys = 2;

    Qgy_Olspl_SData_t *pSK[2];

    /* Sort key 1: Open Date DESC */
    pSK[sk] = (Qgy_Olspl_SData_t*)(
        sortBuf + sizeof(SortInfo.Num_Keys));
    pSK[sk]->Start_Pos =
        offsetof(Qgy_Olspl_RecVar_0300_t,
                 Splf_Open_Date) + 1;
    pSK[sk]->Field_Length =
        sizeof(splfInfo.Splf_Open_Date);
    pSK[sk]->Data_Type  = 6;   /* CHAR */
    pSK[sk]->Sort_Order = 0xf2;/* Descend */

    /* Sort key 2: Open Time DESC */
    sk++;
    pSK[sk] = (Qgy_Olspl_SData_t*)(
        sortBuf + sizeof(SortInfo.Num_Keys)
        + sizeof(SortKey) * sk);
    pSK[sk]->Start_Pos =
        offsetof(Qgy_Olspl_RecVar_0300_t,
                 Splf_Open_Time) + 1;
    pSK[sk]->Field_Length =
        sizeof(splfInfo.Splf_Open_Time);
    pSK[sk]->Data_Type  = 6;   /* CHAR */
    pSK[sk]->Sort_Order = 0xf2;/* Descend */

    ec.init();
    QGYOLSPL(&splfInfo,
             sizeof(splfInfo),
             (void*)&openListInfo,
             rowsToReturn,
             (void*)pSortInfo,
             (void*)&filterInfo,
             &jobName,
             APIFMT,
             (void*)&ec,
             (void*)FILTERFMT);

    if (ec.isEmpty()) {
        _CPYBYTES(pLastSPLF, &splfInfo,
                  sizeof(splfInfo));
        splfCount = openListInfo.Total_Records;
        if (pSPLFName != NULL &&
            !(stricmp(pSPLFName,"*LAST")==0 ||
              pSPLFName[0] == '*'))
        {
            char hList[4];
            int  splfRec = 0;
            int  readNbr = 1;
            const int rtnSz = sizeof(splfInfo);
            _CPYBYTES(hList,
                      openListInfo.Request_Handle,
                      sizeof(
                       openListInfo.Request_Handle));
            int splfNameLen =
                (int)strlen(pSPLFName);
            if (splfNameLen > 10)
                splfNameLen = 10;
            while (
                memcmp(splfInfo.Spooled_File_Name,
                       pSPLFName,
                       splfNameLen) != 0)
            {
                splfRec++;
                if (splfRec > splfCount) break;
                memset(&listInfo, 0x00,
                       sizeof(listInfo));
                ec.init();
                QGYGTLE(&splfInfo, rtnSz,
                        hList, &listInfo,
                        readNbr, splfRec, &ec);
                if (ec.isError() ||
                    listInfo.Records_Retd == 0)
                    break;
            }
        }
    }
    ec.init();
    QGYCLST(openListInfo.Request_Handle, &ec);
    return splfCount;
}

/* ============================================= */
/* coz_readSPLine                                */
/* Parse one SCS print line from a spool buffer. */
/* Replacement for st_readSPLine (S_SPBUFFER).   */
/* ============================================= */
typedef _Packed struct tagPrtCtrl_coz {
    unsigned char ctrl;
    unsigned char cmd;
    unsigned char parm;
} coz_prtCtrl_t;

typedef _Packed struct tagTRNMODE_coz {
    unsigned char TRN;
    unsigned char len;
} coz_TRNMODE_t;

static inline int coz_readSPLine(
    char*             pSpoolBuf,
    int               bufSize,
    int&              bufOffset,
    char*             outLine,
    int               outLen,
    spBufferStatus_t& spStat)
{
    int i    = 0;
    int c    = 0;
    int bEOL = 0;
    int bEOP = 0;
    char* pEOL = NULL;
    char* pEOP = NULL;
    coz_prtCtrl_t prt;
    coz_TRNMODE_t TRNMODE;
    unsigned char* pBuf = NULL;

    spStat.cmd      = 0;
    spStat.parm     = 0;
    spStat.lineFeed = 0;
    spStat.lineLen  = 0;
    spStat.newPage  = 0;

    c    = (int)strlen(outLine);
    pBuf = (unsigned char*)(pSpoolBuf
                            + bufOffset);
    int len = bufSize - bufOffset;

    for (i = 0; i < len; i++) {
        if (pBuf[i] == 0x00) continue;
        if (pBuf[i] == 0x0D) {
            c = 0;
            continue;
        }
        if (pBuf[i] == coz_wTRN ||
            pBuf[i] == coz_wTRNEPSON)
        {
            _CPYBYTES((char*)&TRNMODE,
                      pBuf + i,
                      sizeof(TRNMODE));
            i += sizeof(TRNMODE);
            i += TRNMODE.len;
            i--;
            continue;
        }
        if (pBuf[i] == coz_wCtrl) {
            _CPYBYTES((char*)&prt,
                      pBuf + i,
                      sizeof(prt));
            spStat.cmd  = prt.cmd;
            spStat.parm = prt.parm;
            i += sizeof(prt);
            i--;
            switch (prt.cmd) {
                case 0xC0: case 0xC1:
                case 0xC2: case 0xC3:
                case 0xC4: case 0xC5:
                case 0xC6: case 0xC8:
                case 0xC9: case 0x4A:
                case 0x4C: case 0x4D:
                case 0x4E:
                    if (c < prt.parm)
                        memset(outLine + c,
                               ' ',
                               prt.parm);
                    c = prt.parm - 1;
                    break;
                case coz_wTRN:
                    _CPYBYTES(outLine + c,
                              pBuf + i,
                              prt.parm);
                    c += prt.parm;
                    i += prt.parm;
                    break;
                default:
                    break;
            }
            continue;
        }
        if ((pEOP = (char*)memchr(coz_wFF,
                                  pBuf[i],
                                  coz_s_wFF))
             != NULL)
        {
            i++;
            if (c > 0) bEOL = 1;
            spStat.newPage = 1;
            break;
        }
        if ((pEOL = (char*)memchr(coz_wLF,
                                  pBuf[i],
                                  coz_s_wLF))
             != NULL)
        {
            i++;
            bEOL = 1;
            while ((pBuf[i] == 0x00 ||
                    pBuf[i] == coz_cCR) &&
                   i++ < len);
            break;
        }
        if (pBuf[i] == 0x0E) {
            *(outLine + c++) = pBuf[i];
            for (i++; i < len; i++) {
                if (pBuf[i] == 0x0F) {
                    *(outLine + c++) = pBuf[i];
                    break;
                }
                *(outLine + c++) = pBuf[i];
            }
            if (i >= len) i--;
            spStat.lineLen =
                std::max<int>(c, spStat.lineLen);
            continue;
        }
        if (pBuf[i] == 0x0F) continue;
        if (memchr(coz_wSCS,
                   pBuf[i],
                   coz_s_wSCS) != NULL)
            continue;
        *(outLine + c) = pBuf[i];
        c++;
        spStat.lineLen =
            std::max<int>(c, spStat.lineLen);
        if (bEOL) break;
    }
    bufOffset += i;
    spStat.lineLen =
        std::max<int>(c, spStat.lineLen);
    bEOP = spStat.newPage;
    return (bEOL == 0 && bEOP == 0) ? -1 : c;
}

/* Compatibility macro */
#define READSPLINE(_sfb,_bsz,_boff,_ob,_obL,_sps)\
    coz_readSPLine(_sfb,_bsz,_boff,_ob,_obL,_sps)

/* ============================================= */
/* Parameter-walking macros (coz_ versions)      */
/* ============================================= */
#ifndef _COZ_PARM_MACROS_
#define _COZ_PARM_MACROS_

#define inParm(_v) \
    char *in##_v = \
        (char*)coz_getNextParmIf(p,argc,argv)
#define inChar(_v) \
    char *in##_v = \
        (char*)coz_getNextParmIf(p,argc,argv)
#define inVarChar(_v) \
    char *in##_v = \
        (char*)coz_getNextParmIf(p,argc,argv)
#define inInt(_v) \
    int *in##_v = \
        (int*)coz_getNextParmIf(p,argc,argv)
#define inSmallInt(_v) \
    short *in##_v = \
        (short*)coz_getNextParmIf(p,argc,argv)
#define inBigInt(_v) \
    long long *in##_v = \
        (long long*)coz_getNextParmIf(\
            p,argc,argv)
#define inBIN(_v) \
    SQLUDF_BINARY *in##_v = \
        (SQLUDF_BINARY*)coz_getNextParmIf(\
            p,argc,argv)
#define inBINARY(_v) \
    SQLUDF_BINARY *in##_v = \
        (SQLUDF_BINARY*)coz_getNextParmIf(\
            p,argc,argv)
#define inVARBIN(_v) \
    SQLUDF_VARBINARY *in##_v = \
        (SQLUDF_VARBINARY*)coz_getNextParmIf(\
            p,argc,argv)

#define inIndy(_v) \
    short *indyIn##_v = \
        (short*)coz_getNextParmIf(p,argc,argv)
#define outIndy(_v) \
    short *indy##_v = \
        (short*)coz_getNextParmIf(\
            p,argc,argv,2)
#define outNullIndy(_v) \
    short *indy##_v = \
        (short*)coz_getNextParmIndy(\
            p,argc,argv,-1)
#define outIndyNull(_v) \
    short *indy##_v = \
        (short*)coz_getNextParmIndy(\
            p,argc,argv,-1)
#define inOutIndy(_v) \
    short *indy##_v = \
        (short*)coz_getNextParmIf(\
            p,argc,argv,2)

#define outParm(_v) \
    char *out##_v = \
        (char*)coz_getNextParmIf(\
            p,argc,argv,1)
#define outChar(_v) \
    char *out##_v = \
        (char*)coz_getNextParmIf(\
            p,argc,argv,1)
#define outVarChar(_v) \
    char *out##_v = \
        (char*)coz_getNextParmIf(\
            p,argc,argv,1)
#define outVARCHAR(_v) \
    struct sqludf_vc_fbd *out##_v = \
        (struct sqludf_vc_fbd*) \
        coz_getNextParmIf(p,argc,argv,2)
#define outSmallInt(_v) \
    short *out##_v = \
        (short*)coz_getNextParmIf(\
            p,argc,argv,2)
#define outSMALLINT(_v) \
    short *out##_v = \
        (short*)coz_getNextParmIf(\
            p,argc,argv,2)
#define outInt(_v) \
    int *out##_v = \
        (int*)coz_getNextParmIntIf(\
            p,argc,argv)
#define outINT(_v) \
    int *out##_v = \
        (int*)coz_getNextParmIntIf(\
            p,argc,argv)
#define outBigInt(_v) \
    long long *out##_v = \
        (long long*)coz_getNextParmIf(\
            p,argc,argv,8)
#define outBIGINT(_v) \
    long long *out##_v = \
        (long long*)coz_getNextParmIf(\
            p,argc,argv,8)
#define outBIN(_v) \
    SQLUDF_BINARY *out##_v = \
        (SQLUDF_BINARY*)coz_getNextParmIf(\
            p,argc,argv)
#define outBINARY(_v) \
    SQLUDF_BINARY *out##_v = \
        (SQLUDF_BINARY*)coz_getNextParmIf(\
            p,argc,argv)
#define outVARBIN(_v) \
    SQLUDF_VARBINARY *out##_v = \
        (SQLUDF_VARBINARY*)coz_getNextParmIf(\
            p,argc,argv,2)
#define outVARBINARY(_v) \
    SQLUDF_VARBINARY *out##_v = \
        (SQLUDF_VARBINARY*)coz_getNextParmIf(\
            p,argc,argv,2)

#endif /* _COZ_PARM_MACROS_ */

#pragma datamodel(pop)

#endif /* _COZUTILS_H_ */
