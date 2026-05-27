// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 1996-2026 by R. Cozzi, Jr.
// @author BobCozzi

// COZUTILS.H - Standalone spool-read utility helpers.
// Replaces coz:: namespace for SPOOLDATA standalone UDTF.
// All functions are static inline, using prefix coz_.
// Line length: each line is <=76 visible characters.
// (c) 2025 Robert Cozzi Jr. All rights reserved.

#ifndef _COZUTILS_H_
#define _COZUTILS_H_

#include <sys/time.h>
#include <sys/types.h>

#pragma datamodel(P128)

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>

#include <qusec.h>
#include <QUSCRTUS.h>
#include <QUSCUSAT.h>
#include <QUSPTRUS.h>
#include <QLIDLTO.h>
#include <QWCCVTDT.h>
#include <lecond.h>
#include <except.h>
#include <qdbrtvsn.h>

#include <qgyolspl.h>
#include <qgygtle.h>
#include <qgyclst.h>
#include <Qp0ztrc.h>
#include <qlgcase.h>
#include <QSPRILSP.h>
#include <qmhsndpm.H>
#include <QMHRCVPM.h>
#include <QMHRMVPM.h>
#include <qusrmbrd.h>
#include <quslmbr.h>

#include <micommon.mih>
#include <cpybytes.mih>
#include <triml.mih>
#include <matpgmnm.mih>

#include <sql.h>
#include <sqludf.h>

/* Forward declarations (defined below)         */
static inline int coz_toUpper(char* szData,
    int inLen = -1, int ccsid = 0);
static inline int coz_rmvmsg(
    const char* msgtype,
    const char* msgkey = NULL);


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
/* IBM i Open List handle types                  */
/* (used by file-fields and other UDTFs)         */
/* ============================================= */
typedef char coz_listHANDLE[4];

typedef _Packed struct coz_openHandles_tag {
    int            count;
    coz_listHANDLE handle[];
} coz_openListHandles_t;

/* Size constants (mirrors coz::MAX_SIZES enum)  */
enum {
    coz_MAXUSRSPACESIZE       = 16711568,
    coz_MAXMEM15M             = 15662992,
    coz_MAX2PARTNAME_LEN      = 20,
    coz_MAXLONGNAME_LEN       = 128,
    coz_MAXQUALLONGNAME_LEN   = 257,
    coz_MAXSQLNAME_LEN        = 128,
    coz_MAXQUALOBJNAME_LEN    = 21,
    coz_MAXSYSOBJNAME_LEN     = 10,
    coz_MAX2PARTSYSOBJNAME_LEN = 20
};

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
    return coz_toUpper(szData, len);
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
    int   inLen,
    int   ccsid)
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
static inline char* coz_getPtrUsrSpace(
    const char* p2PartUsrSpaceName)
{
    void*     pUS = NULL;
    coz_qusec ec;
    ec.init();
    QUSPTRUS((char*)p2PartUsrSpaceName,
             &pUS, &ec);
    if (ec.isEmpty()) return (char*)pUS;
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
                                        const char* objAttr = "SQLTOOLS",
                                        int         initSize = 4096)
{
    char qualName[21];
    char extAttr[11];
    char pubAut[11];
    char text[51];
    char repl[11];
    char domain[11];
    char initVal = 0x00;
    char* pUS   = NULL;
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
        coz_rmvmsg("*COMP",NULL);  // Remove "Changed owner" message
        /* Enable auto-extend so the space grows   */
        /* automatically when APIs write past the  */
        /* initial size (e.g. QDBRTVFD).           */
        /* key=3 is AutoExtend; key=1=size,        */
        /* key=2=InitChar, key=4=TransferSize.     */
        typedef _Packed struct {
            int  key;
            int  dataLen;
            union {
                int  size;
                int  transferSize;
                char autoExtend;
                char initValue;
            };
        } coz_spaceKey_t;
        typedef _Packed struct {
            int           count;
            coz_spaceKey_t attr[1];
        } coz_spaceAttr_t;
        coz_spaceAttr_t spaceAttr;
        spaceAttr.count          = 1;
        spaceAttr.attr[0].key     = 3;
        spaceAttr.attr[0].dataLen = 1;
        spaceAttr.attr[0].autoExtend = '1';
        char rtnLib[10];
        memset(rtnLib, ' ', sizeof(rtnLib));
        ec.init();
        QUSCUSAT(rtnLib, qualName, &spaceAttr, &ec);
        /* Non-fatal if QUSCUSAT fails — space     */
        /* will still work, just won't auto-grow.  */
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
/* coz_isAllHexZeros                             */
/* Returns true when all sz bytes at ptr = 0x00 */
/* ============================================= */
static inline bool coz_isAllHexZeros(
    const char* ptr, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        if ((unsigned char)ptr[i] != 0x00)
            return false;
    return true;
}

/* ============================================= */
/* coz_isAllBlanks                               */
/* Returns true when all bytes are 0x40 or 0x20 */
/* ============================================= */
static inline bool coz_isAllBlanks(
    const char* ptr, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        if (ptr[i] != ' ' &&
            (unsigned char)ptr[i] != 0x40)
            return false;
    return true;
}

/* coz_isAllBinZeros \Z alias for coz_isAllHexZeros */
static inline bool coz_isAllBinZeros(
    const char* ptr, size_t sz)
{ return coz_isAllHexZeros(ptr, sz); }

/* ============================================= */
/* coz_isNotEmpty (2-arg free function)           */
/* Returns true when buffer has any non-blank     */
/* byte (not 0x20 or 0x40)                        */
/* ============================================= */
static inline bool coz_isNotEmpty(
    const char* ptr, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        if (ptr[i] != ' ' &&
            (unsigned char)ptr[i] != 0x40)
            return true;
    return false;
}

/* ============================================= */
/* coz_makeQual                                  */
/* Build 20-char qualified name from separate    */
/* object (10) and library (10) strings.         */
/* ============================================= */
static inline void coz_makeQual(
    char*       dest,
    const char* objName,
    const char* libName)
{
    memset(dest, ' ', 20);
    if (objName != NULL) {
        int n = (int)strlen(objName);
        if (n > 10) n = 10;
        memcpy(dest,      objName, n);
    }
    if (libName != NULL) {
        int n = (int)strlen(libName);
        if (n > 10) n = 10;
        memcpy(dest + 10, libName, n);
    }
}

/* ============================================= */
/* coz_makeAPIJobName                            */
/* Parse JOB/USER/NBR string and populate the    */
/* first 26 bytes of a Qwc_JIDF0100_t struct:    */
/* Job_Name[10] User_Name[10] Job_Number[6]      */
/* ============================================= */
static inline void coz_makeAPIJobName(
    char*       pJobStruct,
    const char* jobName)
{
    char *pJob = pJobStruct;
    char *pUsr = pJobStruct + 10;
    char *pNbr = pJobStruct + 20;
    memset(pJob, ' ', 10);
    memset(pUsr, ' ', 10);
    memset(pNbr, ' ',  6);
    if (jobName == NULL || jobName[0] == '\0')
        return;
    const char* s1 = strchr(jobName, '/');
    if (s1 == NULL) {
        int n = (int)strlen(jobName);
        if (n > 10) n = 10;
        memcpy(pJob, jobName, n);
    } else {
        int n = (int)(s1 - jobName);
        if (n > 10) n = 10;
        memcpy(pJob, jobName, n);
        const char* s2 = strchr(s1 + 1, '/');
        if (s2 == NULL) {
            int u = (int)strlen(s1 + 1);
            if (u > 10) u = 10;
            memcpy(pUsr, s1 + 1, u);
        } else {
            int u = (int)(s2 - (s1 + 1));
            if (u > 10) u = 10;
            memcpy(pUsr, s1 + 1, u);
            int nb = (int)strlen(s2 + 1);
            if (nb > 6) nb = 6;
            memcpy(pNbr, s2 + 1, nb);
        }
    }
}

/* ============================================= */
/* coz_getOSVer                                  */
/* Returns IBM i OS version as integer           */
/* e.g., 740 for V7R4M0.                         */
/* ============================================= */
static inline int coz_getOSVer()
{
    _INT4      osVer    = 0;
    _INT4      platform = 0;
    _FEEDBACK  fc;
    CEEGPID(&osVer, &platform, &fc);
    return (int)osVer;
}

/* ============================================= */
/* coz_BCAT                                      */
/* Blank-concatenate: appends " " + src to dest  */
/* if dest is non-empty; otherwise just copies.  */
/* Appends optional src2 the same way.           */
/* ============================================= */
static inline void coz_BCAT(
    char*       dest,
    const char* src1,
    const char* src2 = NULL)
{
    if (src1 != NULL && strlen(src1) > 0) {
        if (strlen(dest) > 0)
            strcat(dest, " ");
        strcat(dest, src1);
    }
    if (src2 != NULL && strlen(src2) > 0) {
        if (strlen(dest) > 0)
            strcat(dest, " ");
        strcat(dest, src2);
    }
}

/* ============================================= */
/* coz_cvtSYSDTStoDTS                            */
/* Convert IBM i system DTS (8 bytes) to an     */
/* ISO-style timestamp string via QWCCVTDT.      */
/* ============================================= */
static inline char* coz_cvtSYSDTStoDTS(
    char*       t,
    const char* pDts,
    bool        includeMicroSecs = false,
    bool        includeTM        = true)
{
    static char dts[32];
    if (t) memset(t, 0x00, 8);
    memset(dts, 0x00, sizeof(dts));
    if (pDts == NULL ||
        coz_isAllBinZeros(pDts, 8) ||
        coz_isAllBlanks(pDts, 8))
    {
        if (t) *t = 0x00;
        return NULL;
    }
    char inFmt[11];
    char outFmt[11];
    char mSecsFlag[1] = {'1'};
    Qwc_Time_Zone_Info_t rtnTZInfo;
    memset(inFmt,  ' ', sizeof(inFmt));
    _CPYBYTES(inFmt,  "*DTS",  4);
    memset(outFmt, ' ', sizeof(outFmt));
    _CPYBYTES(outFmt, "*YYMD", 5);
    const char TZ_SYS[] = "*SYS      ";
    const char TZ_JOB[] = "*JOB      ";
    memset((char*)&rtnTZInfo, 0x00,
           sizeof(rtnTZInfo));
    coz_qusec ec;
    ec.init();
    QWCCVTDT(inFmt,  (char*)pDts,
             outFmt, dts,
             &ec,
             TZ_SYS, TZ_JOB,
             &rtnTZInfo, sizeof(rtnTZInfo),
             mSecsFlag);
    if (ec.isEmpty()) {
        int yr=0, mo=0, dy=0;
        int hr=0, mn=0, sc=0, ms=0;
        int n = sscanf(dts,
            "%4d%2d%2d%2d%2d%2d%6d",
            &yr,&mo,&dy,&hr,&mn,&sc,&ms);
        if (n < 6) return NULL;
        if (includeMicroSecs)
            sprintf(dts,
                "%04d-%02d-%02d-%02d."
                "%02d.%02d.%06d",
                yr,mo,dy,hr,mn,sc,ms);
        else if (includeTM)
            sprintf(dts,
                "%04d-%02d-%02d-%02d."
                "%02d.%02d",
                yr,mo,dy,hr,mn,sc);
        else
            sprintf(dts,
                "%04d-%02d-%02d",
                yr,mo,dy);
        if (t != NULL && strlen(dts) > 0)
            strcpy(t, dts);
    } else {
        memset(dts, 0x00, sizeof(dts));
    }
    return (strlen(dts) > 0) ? dts : NULL;
}

/* ============================================= */
/* coz_copyAtoB                                  */
/* Copy min(maxTarget,maxSource) bytes from s   */
/* to t; stop at any char in stopAt.             */
/* Target is blank-padded to maxTargetLen.       */
/* ============================================= */
static inline int coz_copyAtoB(
    char*       t,
    const int   maxTargetLen,
    const char* s,
    const int   maxSourceLen = 10,
    const char* stopAt = " ")
{
    int bStop  = 0;
    int rtnLen = 0;
    int maxLen = (maxTargetLen < maxSourceLen)
                  ? maxTargetLen : maxSourceLen;
    memset(t, ' ', maxTargetLen);
    for (int i=0; s[i]!=0x00 && i<maxLen; i++) {
        if (stopAt != NULL) {
            for (size_t j=0; j<strlen(stopAt); j++)
                if (s[i]==stopAt[j]) bStop=1;
        }
        if (bStop) break;
        t[i] = s[i];
        rtnLen++;
    }
    return rtnLen;
}

/* ============================================= */
/* coz_splitFormat                               */
/* Split a delimited list into fixed-width       */
/* entries written into pFixed.                  */
/* ============================================= */
static inline int coz_splitFormat(
    char*       pFixed,
    const char* pValues,
    int         entryLen   = 10,
    int         inLength   = -1,
    int         leadAction = 2,
    const char* delims     = " ,\n",
    const char  leadChar   = '*')
{
    int bufLen = (inLength <= 0)
                  ? (int)strlen(pValues)
                  : inLength;
    if (bufLen <= 0) return 0;
    std::vector<char> buf(
        pValues, pValues + bufLen);
    if (buf.back() != '\0')
        buf.push_back('\0');
    std::vector<char> dynoDelims;
    if (delims == NULL || strlen(delims)==0) {
        const char* def = " ,;\n";
        dynoDelims.assign(
            def, def + strlen(def));
    } else {
        dynoDelims.assign(
            delims, delims + strlen(delims));
    }
    if (delims && strchr(delims,'\n')) {
        dynoDelims.push_back(0x25);
        dynoDelims.push_back(0x15);
    }
    if (!dynoDelims.empty() &&
        dynoDelims.back() != '\0')
        dynoDelims.push_back('\0');
    char* pData    = &buf[0];
    int   counter  = 0;
    int   nextEntry = 0;
    char* saveptr  = NULL;
    char* token    =
        strtok_r(pData, &dynoDelims[0], &saveptr);
    while (token != NULL) {
        memset(pFixed + nextEntry, ' ', entryLen);
        switch (leadAction) {
            case 0:
                if (*token == leadChar) token++;
                break;
            case 1:
                if (*token != leadChar) {
                    *(pFixed+nextEntry) = leadChar;
                    pFixed++;
                }
                break;
            default: break;
        }
        _CPYBYTES(pFixed + nextEntry,
                  token, strlen(token));
        nextEntry += entryLen;
        counter++;
        token = strtok_r(
            NULL, &dynoDelims[0], &saveptr);
    }
    pFixed[nextEntry] = '\0';
    return counter;
}

/* ============================================= */
/* coz_getSysObjName                             */
/* Resolve long SQL name to 20-byte short API    */
/* format "OBJNAME   LIBNAME   " via QDBRTVSN.   */
/* If longLibrary is NULL, returns only the      */
/* short lib name for longObjectName.            */
/* ============================================= */
static inline char* coz_getSysObjName(
    const char* longObjectName,
    const char* longLibrary = NULL)
{
    static char result[21];
    char szLongName[129];
    char szShortName[21];
    char szLibName[11];
    int  nameLen  = 0;
    int  bGeneric = 0;
    coz_qusec ec;
    memset(result,    ' ', 20);
    result[20] = 0x00;
    memset(szLibName, ' ', sizeof(szLibName));
    /* Determine what to resolve first */
    const char* first =
        (longLibrary == NULL ||
         strlen(longLibrary) == 0)
        ? longObjectName : longLibrary;
    nameLen = coz_copyAtoB(
        szLongName, 128, first, 128, NULL);
    szLongName[nameLen] = 0x00;
    coz_toUpper(szLongName, nameLen);
    bGeneric =
        (memchr(szLongName,'*',nameLen)!=NULL)
        ? 1 : 0;
    if (!bGeneric &&
        szLongName[0] != '*' &&
        nameLen > coz_MAXSYSOBJNAME_LEN)
    {
        memset(szShortName,' ',sizeof(szShortName));
        ec.init();
        QDBRTVSN(szShortName,
                 szLongName, nameLen,
                 szLibName, &ec);
        if (ec.isEmpty())
            coz_copyPad(szLibName,
                        szShortName,
                        coz_MAXSYSOBJNAME_LEN);
        else
            coz_copyPad(szLibName,
                        szLongName,
                        coz_MAXSYSOBJNAME_LEN);
    } else {
        coz_copyPad(szLibName,
                    szLongName,
                    coz_MAXSYSOBJNAME_LEN);
    }
    /* Library-only case */
    if (longLibrary == NULL ||
        strlen(longLibrary) == 0)
    {
        coz_copyPad(result, szLibName, 10);
        return result;
    }
    /* Resolve the object name */
    if (longObjectName != NULL &&
        strlen(longObjectName) > 0)
    {
        nameLen = coz_copyAtoB(
            szLongName, 128,
            longObjectName, 128, NULL);
        szLongName[nameLen] = 0x00;
        coz_nameUpper(szLongName);
        bGeneric =
            (memchr(szLongName,'*',nameLen)!=NULL)
            ? 1 : 0;
        if (!bGeneric && nameLen >
            coz_MAXSYSOBJNAME_LEN)
        {
            memset(szShortName,' ',
                   sizeof(szShortName));
            ec.init();
            QDBRTVSN(szShortName,
                     szLongName, nameLen,
                     szLibName, &ec);
            if (ec.isEmpty())
                _CPYBYTES(result,
                          szShortName, 20);
            else {
                coz_copyPad(result,    szLongName, 10);
                coz_copyPad(result+10, szLibName,  10);
            }
        } else {
            coz_copyPad(result,    szLongName, 10);
            coz_copyPad(result+10, szLibName,  10);
        }
    }
    return result;
}

/* ============================================= */
/* coz_makeAPIObjNameFromLongName                */
/* Build a 20-byte API name from a long/short    */
/* qualified SQL name, resolving via QDBRTVSN.   */
/* ============================================= */
static inline void coz_makeAPIObjNameFromLongName(
    char*       objName,
    const char* qualObj,
    const char* dftLib = "*LIBL")
{
    char longName[129];
    char longLib[129];
    memset(objName,  ' ',
           coz_MAX2PARTSYSOBJNAME_LEN);
    /* Split on '/' or '.' */
    const char* slash = strchr(qualObj, '/');
    if (slash == NULL)
        slash = strchr(qualObj, '.');
    const char* firstBlank = strchr(qualObj, ' ');
    if (firstBlank != NULL && slash != NULL &&
        firstBlank < slash)
        slash = NULL;
    if (slash != NULL) {
        int libLen = (int)(slash - qualObj);
        coz_copyUntil(longName,
                      qualObj + libLen + 1,
                      coz_MAXSQLNAME_LEN);
        coz_copyUntil(longLib,
                      qualObj, libLen);
    } else {
        coz_copyUntil(longName,
                      qualObj,
                      coz_MAXSQLNAME_LEN, " ");
        if (dftLib == NULL)
            coz_copyUntil(longLib,
                          "*LIBL", 5);
        else
            coz_copyUntil(longLib,
                          dftLib,
                          coz_MAXSQLNAME_LEN);
    }
    char* pResult =
        coz_getSysObjName(longName, longLib);
    _CPYBYTES(objName, pResult, 20);
}

/* ============================================= */
/* coz_makeQualListFromLongName                  */
/* Parse a space/comma-delimited list of long    */
/* SQL file names and write 20-byte API names    */
/* into pQualList.  Returns the count of names.  */
/* ============================================= */
static inline int coz_makeQualListFromLongName(
    char*       pQualList,
    char*       pObjList,
    const char* pDftLib = NULL)
{
    char qualObj[coz_MAX2PARTNAME_LEN + 1];
    char dftLib[11];
    coz_copyPad(dftLib, "*LIBL", 10);
    if (pDftLib != NULL && strlen(pDftLib) > 0)
        coz_copyPad(dftLib, pDftLib, 10);
    int objCount = coz_splitFormat(
        pQualList, pObjList,
        coz_MAXQUALLONGNAME_LEN);
    for (int x = 0; x < objCount; x++) {
        memset(qualObj, ' ', sizeof(qualObj));
        coz_makeAPIObjNameFromLongName(
            qualObj,
            pQualList +
              (x * coz_MAXQUALLONGNAME_LEN),
            dftLib);
        _CPYBYTES(
            pQualList +
              (x * coz_MAX2PARTNAME_LEN),
            qualObj,
            coz_MAX2PARTNAME_LEN);
    }
    return objCount;
}

/* ============================================= */
/* resignalMsg (global scope)                    */
/* Free-function alias so code that calls        */
/* resignalMsg(ec) without namespace prefix      */
/* compiles when ec is coz_qusec / coz::qusec.   */
/* ============================================= */
static inline void resignalMsg(coz_qusec& ec)
{
    coz_resignalMsg(ec);
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

#define inCLOB(_v) \
    SQLUDF_CLOB *in##_v = \
        (SQLUDF_CLOB*)coz_getNextParmIf(\
            p,argc,argv)
#define inDBCLOB(_v) \
    SQLUDF_DBCLOB *in##_v = \
        (SQLUDF_DBCLOB*)coz_getNextParmIf(\
            p,argc,argv)
#define outDts(_v) \
    char *out##_v = \
        (char*)coz_getNextParmIf(\
            p,argc,argv,26)
/* outVarchar \Z lowercase-r alias for outVarChar */
#define outVarchar(_v) outVarChar(_v)

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

#define outCLOB(_v)   SQLUDF_CLOB*   out##_v = (SQLUDF_CLOB*)   coz_getNextParmIf(p, argc, argv, 4)
#define outBLOB(_v)   SQLUDF_CLOB*   out##_v = (SQLUDF_CLOB*)   coz_getNextParmIf(p, argc, argv, 4)
#define outDBCLOB(_v) SQLUDF_DBCLOB* out##_v = (SQLUDF_DBCLOB*) coz_getNextParmIf(p, argc, argv, 4)

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

/* ============================================= */
/* UDF termination/signaling macros              */
/* These macros reference locals that must exist */
/* in the calling scope: sqlstate, sqlmsgtext,   */
/* pScratch (for signalEOFFunc), errno           */
/* ============================================= */
#ifndef _ST_UDF_TERMINATION_
#define _ST_UDF_TERMINATION_
#define logStrerror() \
        Qp0zLprintf("errno(%d) %s\n", errno, strerror(errno))
#define signalEndFunc() \
        ::strcpy(sqlmsgtext, strerror(errno)); \
        ::strcpy(sqlstate, "02000")
#define signalEOFFunc() \
        ::strcpy(sqlmsgtext, strerror(errno)); \
        ::strcpy(sqlstate, "02000"); \
        pScratch->eof = 1
#endif /* _ST_UDF_TERMINATION_ */

/* ============================================= */
/* coz_rmvmsg                                    */
/* Remove a message from the job log.            */
/* Typically used after QUSCRTUS to suppress the */
/* CPC2206 "Owner Changed" completion message.   */
/* msgtype: e.g. "*COMP", "*PRVJLMSG" (default) */
/* msgkey:  4-byte msg key, or NULL              */
/* Returns 1=removed (or no match), 0=API error  */
/* ============================================= */
static inline int coz_rmvmsg(
    const char* msgtype,
    const char* msgkey)
{
    coz_qusec              ec;
    int                    callStackCounter = 0;
    char                   callStack[10];
    char                   msgType[11];
    char                   msgKey[4];
    Qmh_Rcvpm_RCVM0100_t   msgInfo;

    memset(callStack, ' ', sizeof(callStack));
    memset(msgType,   ' ', sizeof(msgType));
    memset(msgKey,  0x00,  sizeof(msgKey));
    memset(&msgInfo,0x00,  sizeof(msgInfo));

    callStack[0]     = '*';
    callStackCounter = 0;

    if (msgtype != NULL && msgtype[0] != ' ' &&
        strlen(msgtype) > 0)
    {
        size_t mlen = strlen(msgtype);
        if (mlen > 10) mlen = 10;
        _CPYBYTES(msgType, msgtype, (int)mlen);
    }
    else
    {
        _CPYBYTES(msgType, "*PRVJLMSG ", 10);
    }

    if (msgkey != NULL)
        _CPYBYTES(msgKey, msgkey, 4);

    ec.init();
    QMHRCVPM(&msgInfo, sizeof(msgInfo),
             "RCVM0100",
             callStack, callStackCounter,
             msgType, msgKey,
             0,          /* wait time: 0 = no wait */
             "*SAME     ", &ec);

    if (ec.hasError())
        return 0;

    if (memcmp(msgInfo.Message_Id, "CPC2206", 7) == 0)
    {
        ec.init();
        QMHRMVPM(callStack, callStackCounter,
                 msgInfo.Message_Key,
                 "*BYKEY    ", &ec);
    }
    return 1;
}

/* ============================================= */
/* namespace coz \Z compatibility wrapper         */
/* Allows code written against the original      */
/* coz:: namespace (S_NS_COZ.H) to compile when  */
/* COZUTILS.H is substituted in its place.       */
/* Only the types and functions used by the      */
/* open-UDTF project are provided here.          */
/* ============================================= */
#ifndef _COZ_COMPAT_NS_
#define _COZ_COMPAT_NS_
namespace coz {

    /* ---- types ---- */
    typedef coz_listHANDLE        listHANDLE;
    typedef coz_openListHandles_t openListHandles_t;
    typedef coz_qusec             qusec;

    /* ---- size constants ---- */
    enum MAX_SIZES {
        MAXUSRSPACESIZE       = coz_MAXUSRSPACESIZE,
        MAXMEM15M             = coz_MAXMEM15M,
        MAX2PARTNAME_LEN      = coz_MAX2PARTNAME_LEN,
        MAXLONGNAME_LEN       = coz_MAXLONGNAME_LEN,
        MAXQUALLONGNAME_LEN   = coz_MAXQUALLONGNAME_LEN,
        MAXSQLNAME_LEN        = coz_MAXSQLNAME_LEN,
        MAXQUALOBJNAME_LEN    = coz_MAXQUALOBJNAME_LEN,
        MAXSYSOBJNAME_LEN     = coz_MAXSYSOBJNAME_LEN,
        MAX2PARTSYSOBJNAME_LEN= coz_MAX2PARTSYSOBJNAME_LEN
    };

    /* ---- parameter helpers ---- */
    static inline char* getNextParmIf(
        int& p, int& argc, char** argv,
        int f = 0)
    { return coz_getNextParmIf(p,argc,argv,f); }

    /* ---- string/name utilities ---- */
    static inline void copyPad(
        char* t, const char* s,
        int len=10, char pc=' ')
    { coz_copyPad(t,s,len,pc); }

    static inline int copyUntil(
        char* t, const char* s,
        int maxLen=10,
        const char* stopAt=NULL,
        bool bTrim=false)
    { return coz_copyUntil(t,s,maxLen,stopAt,bTrim); }

    static inline void makeAPIObjName(
        char* n, const char* q,
        const char* d="*LIBL")
    { coz_makeAPIObjName(n,q,d); }

    static inline int toUpper(
        char* s, int len=-1, int ccsid=0)
    { return coz_toUpper(s,len,ccsid); }

    static inline int nameUpper(
        char* s, int len=-1)
    { return coz_nameUpper(s,len); }

    /* ---- IBM i object / user-space ---- */
    static inline char* TEMP_USRSPACE(char* r,
                                      const char* a="SQLTOOLS",
                                      int sz=4096)
    { return coz_TEMP_USRSPACE(r,a,sz); }

    static inline char* getPtrUsrSpace(
        const char* p)
    { return coz_getPtrUsrSpace(p); }

    static inline void DLTOBJ(
        const char* q, const char* t,
        const char* a="*")
    { coz_DLTOBJ(q,t,a); }

    /* ---- OS version ---- */
    static inline int getOSVer()
    { return coz_getOSVer(); }

    /* ---- date/time ---- */
    static inline char* cvtSYSDTStoDTS(
        char* t, const char* p,
        bool ms=false, bool tm=true)
    { return coz_cvtSYSDTStoDTS(t,p,ms,tm); }

    /* ---- string concat ---- */
    static inline void BCAT(
        char* d, const char* s1,
        const char* s2=NULL)
    { coz_BCAT(d,s1,s2); }

    /* ---- object list helpers ---- */
    static inline int makeQualListFromLongName(
        char* q, char* l,
        const char* d=NULL)
    { return coz_makeQualListFromLongName(q,l,d); }

    static inline bool isAllHexZeros(
        const char* p, size_t s)
    { return coz_isAllHexZeros(p,s); }

    static inline bool isAllBinZeros(
        const char* p, size_t s)
    { return coz_isAllBinZeros(p,s); }

    static inline bool isAllBlanks(
        const char* p, size_t s)
    { return coz_isAllBlanks(p,s); }

    static inline bool isNotEmpty(
        const char* p, size_t s)
    { return coz_isNotEmpty(p,s); }

    static inline void makeQual(
        char* d, const char* o, const char* l)
    { coz_makeQual(d,o,l); }

    static inline void makeAPIJobName(
        char* j, const char* n)
    { coz_makeAPIJobName(j,n); }


    /* ---- joblog ---- */
    static inline void joblog(const char* fmt, ...)
    {
        char buf[4096];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0x00;
        Qp0zLprintf("%s", buf);
    }

    static char buffer4k[4097];


    /* ---- error signalling ---- */
    static inline void resignalMsg(qusec& ec)
    { coz_resignalMsg(ec); }

    /* ---- job log message removal ---- */
    static inline int RMVMSG(
        const char* msgtype = "*COMP",
        const char* msgkey  = NULL)
    { return coz_rmvmsg(msgtype, msgkey); }

} /* end namespace coz */
#endif /* _COZ_COMPAT_NS_ */

#pragma datamodel(pop)

#endif /* _COZUTILS_H_ */
