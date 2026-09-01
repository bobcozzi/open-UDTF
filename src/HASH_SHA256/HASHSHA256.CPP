// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 1996-2026 by R. Cozzi, Jr.

  // @author BobCozzi

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
#include <string.h>
#include <errno.h>
#include <lecond.h>
#include <except.h>

#include <ctype.h>
#include <inttypes.h>


#include <qc3cci.h>
#include <qc3hash.h>

#include <langinfo.h>
#include <QP0LSTDI.h>
#include <QP0ZTRC.h>
#include <qusec.h>


  // include syntax required for MI Instruction's h files
#include <mih/cpybytes.h>
#include <mih/triml.h>
#include <mih/cvtch.h>
#include <mih/cvthc.h>


#include <string>
#include <memory>

#include <algorithm>
#include <vector>

#include <SQL.h>
#include <SQLUDF.h>


using namespace std;


/* ============================================= */
/* qusec - IBM i standard error code wrapper */
/* ============================================= */

typedef struct tag_QUS_EC
 {
    union { int  Bytes_Provided; int  length; };
    union { int  Bytes_Available; int  Bytes_Returned; };
    union { char Exception_Id[7]; char msgid[7]; };
    char Reserved;
    union { char Exception_Data[255]; char msgdta[255]; };
 } qusec_t;



/* ============================================= */
/* Helper function prototypes                    */
/* ============================================= */


inline char* getNextParmIf(int& pC, int& argc, char** argv, int ioFlag = 0);

#define CLOBDATA(_v) ((SQLUDF_CLOB*) _v)->data
#define CLOBLEN(_v)  ((SQLUDF_CLOB*) _v)->length
#define BINDATA(_v)  ((SQLUDF_VARBINARY*) _v)->data
#define BINLEN(_v)   ((SQLUDF_VARBINARY*) _v)->length

#define inChar(_v) \
          char *in##_v = \
          (char*) getNextParmIf(p,argc,argv)

#define inCLOB(_v)   \
           SQLUDF_CLOB*  in##_v  = \
          (SQLUDF_CLOB*)   getNextParmIf(p, argc, argv)

#define outVARBINARY(_v) \
           SQLUDF_VARBINARY* out##_v = \
          (SQLUDF_VARBINARY*) getNextParmIf(p, argc, argv,2)


#define inIndy(_v) \
    short *indyIn##_v = \
        (short*) getNextParmIf(p,argc,argv)
#define outIndy(_v) \
    short *indy##_v = \
        (short*) getNextParmIf(p,argc,argv,2)

int main(int argc, char *argv[])
{
   int p = 0;
   qusec_t        ec;


    //////////////////////////////////////////////
    //  INPUT Parameter List
    //////////////////////////////////////////////
    inCLOB(RAWDATA);      // Data to be hashed

   //////////////////////////////////////////////
    //  OUTPUT Column List
    //////////////////////////////////////////////
    outVARBINARY(HASH);  // 64-byte SHA-256 result


    ////////////////////////////////////////////////////////////
    //  Input Parameter INDICATORS
    ////////////////////////////////////////////////////////////
    inIndy(RAWDATA);      // Raw input data (read only)

    ////////////////////////////////////////////////////////////
    //  Output Column INDICATORS
    ////////////////////////////////////////////////////////////
    outIndy(HASH);

    ////////////////////////////////////////////////////////////
    //  SQL specific parameters
    ////////////////////////////////////////////////////////////
   char *sqlstate = (char *)getNextParmIf(p, argc, argv);
   char *sqlfuncName = (char *)getNextParmIf(p, argc, argv);
   char *specificName = (char *)getNextParmIf(p, argc, argv);
   char *sqlmsgtext = (char *)getNextParmIf(p, argc, argv);

     const int MD5_LEN = 16;
     const int SHA_1_LEN = 20;
     const int SHA_224_LEN = 28;
     const int SHA3_224_LEN = 28;

     const int SHA_256_LEN = 32;   // This version of HASH is for SHA256
     const int SHA3_256_LEN = 32;   // This version of HASH is for SHA256
     const int SHA_384_LEN = 48;
     const int SHA3_384_LEN = 48;
     const int SHA_512_LEN = 64;
     const int SHA3_512_LEN = 64;

     const int MD5    = 1;
     const int SHA1   = 2;
     const int SHA256 = 3;
     const int SHA384 = 4;
     const int SHA512 = 5;
     const int SHA224 = 7;
     const int SHA3_224 = 8;
     const int SHA3_256 = 9;
     const int SHA3_384 = 11;
     const int SHA3_512 = 12;

      // This program supports SHA-512 which is a 64-byte result
      // however this implementation ony does SHA-256 which is 32-bytes

     char rtnHash[128];     // Double the buffer output size we really need

     char cryptoService[10];
     char service = '0';
     char* pInput = NULL;
     int   inputLen = 0;

     int   rtnLen = 0;

     Qc3_Format_ALGD0500_T algoDesc;


     memset(rtnHash, ' ',sizeof(rtnHash));
     memset(cryptoService, ' ',sizeof(cryptoService));

    memset(&ec,0x00,sizeof(ec));
    ec.Bytes_Provided = sizeof(ec);

    pInput = CLOBDATA(inRAWDATA);
    inputLen = CLOBLEN(inRAWDATA);

      // Customize hash to desired algorithm
    algoDesc.Hash_Alg = SHA256;
    rtnLen = SHA_256_LEN;

    Qc3CalculateHash(pInput, &inputLen,   "DATA0100",
                      (char* volatile) &algoDesc, "ALGD0500",
                      (char* volatile) &service,  cryptoService,
                      rtnHash, &ec);

    BINLEN(outHASH) = 0;

    if (rtnLen > 0)
    {
      BINLEN(outHASH) = rtnLen;
      _CPYBYTES(BINDATA(outHASH), rtnHash, rtnLen);
      *indyHASH = 0;
    }
    else
    {
      *indyHASH = -1;
    }

}



/* ============================================= */
/* getNextParmIf                             */
/* Advance parameter counter and return argv[n]. */
/* If ioFlag>0, memset output parm to 0x00.      */
/* ============================================= */
inline char* getNextParmIf(
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

