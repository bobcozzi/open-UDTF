

     // (c) 2020 - Bob Cozzi, Jr.



// (c) 2020 - out Cozzi, Jr.

#ifndef __STDC_WANT_DEC_FP__
#define __STDC_WANT_DEC_FP__
#endif

#ifndef __POSIX_LOCALE__
#define __POSIX_LOCALE__
#endif


#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <p_time.h>
#include <time.h>

#include <langinfo.h>
#include <QP0LSTDI.h>
#include <QP0ZTRC.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <decimal.h>

#include <qusgen.h>
#include <qdbrtvfd.h>   // Retrieve File Description API
#include <quslrcd.h>      // List record format names

#include <CPYBYTES.mih>
#include <triml.mih>

#include <cstring>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <ctime>

#include <algorithm>
#include <vector>

#include <COZUTILS.h>
#include <OBJLIST.h>

#include <SQL.h>
#include <SQLUDF.h>

using namespace std;

// from the OLCLASS include
object_Desc_t od;

typedef _Packed struct tagScratch
{
    int  length;         // Scratch Pad length
    char QRCDINFO[21];   // Record Format Description
    char RCDFMTLIST[21];  // List of Record Format names
    char QFILEINFO[21];  // File Description
    char QHANDLES[21];   // CObjDesc *FILE list Handles
    char dbfAttr[17];    // PF, LF, DDMF assiged to CObjList attribute filter
    char fileName[11];    // PF, LF, DDMF assiged to CObjList attribute filter
    char fileLib[11];    // PF, LF, DDMF assiged to CObjList attribute filter
    char fileCRTDATE[11];    // *FILE object creation date/timestamp
    int  PF;
    int  LF;
    int  DDMF;
    int  SRC;
    int  DATA;

    int LISTSIZE;
    int NEXTOBJ; // Object List record-to-get counter
    int NBRFIELDS;   // Number of fields in "this" file
    int NEXTFLD;     // Current index (0-based) of the field being returned
    int NEXTFLDOFFSET;

    int nbrOf_rcdFmt;    // Number of record formats for this file
    int rcdFmt_Index;    // Current record format being processed (0-based)
    char rcdFmt_TEXT[51];
    char rcdFmt_ID[14];

    int IBMiVRM;      // IBM I version release as an integer (e.g., 740)
    int eof;
    int counter;
} scratch_t;

scratch_t scratch;

int addToStack(coz::openListHandles_t *pHandle, SQLUDF_CLOB* nameList, const char *dftLib, scratch_t *pScratchPad);
int popLastHandle(coz::openListHandles_t   *pHandle);
int getLastHandle(coz::openListHandles_t   *pHandle, coz::listHANDLE& hList);
int closeAllHandles(coz::openListHandles_t *pHandle);
int buildRcdFmtList(const char* pUserSpaceName,
                  const char* pFileName,
                  const char* pLibName);
Qdb_Lrcd_RCDL0200_t* getRcdFmtEntry(const char* pUserSpace, int entry);


int main(int argc, char *argv[])
{

    int p = 0;

    _MI_Time       mt;
    time_t         epochTime;
    struct timeval tv;
    coz::qusec     ec;
    int            rc = 0;

    /**********************************************************/
    /* Copyright 1992-2022 by out Cozzi, Jr.                   */
    /* All rights reserved.                                   */
    /**********************************************************/

    //////////////////////////////////////////////
    //  INPUT Parameters
    //////////////////////////////////////////////
    inParm(LIB);    // Library name to qualify the File name(s)
    inCLOB(FILES);  // File name (generic, full or *ALL)   CLOB(1M)
    inParm(ATTRS);  // File attribute (i.e., PF, LF, DDMF, *ALL only)

    //////////////////////////////////////////////
    //  OUTPUT Fields
    //////////////////////////////////////////////
    outChar(OBJLIB);
    outChar(FILENAME);
    outChar(FILEATTR);    // File Attribute  (PF, LF, DDMF)
    outChar(FILETYPE);    // Source or Database files (*SRC or *DATA)
    outChar(SQLTYPE);     // SQL Object Type (TABLE, VIEW, INDEX)
    outDts(CRTDATE);      // File creation date

    outInt(FLDCNT);        // Field count
    outInt(RCDFMTCNT);     // Number of record formats

    outChar(LVLCHK);
    outChar(LVLCHKID);

    outChar(RCDFMTTEXT);  // Record Format Text
    outChar(RCDFMTID);    // 13-character Record format ID
    outInt(RCDFMTNBR);    // Record Format Ordinal for "this" field.
    outChar(RCDFMT);      // Record format name
    outSmallInt(RCDLEN);  // Record length

    outInt(SEQNBR);
    outChar(FLDNAME);
    outChar(DATATYPE);
    outInt(BUFLEN);  // length in buffer
    outInt(FLDLEN);
    outSmallInt(DECPOS);

    outChar(RPGDEFN);  // RPG IV DCL-S statement
    outChar(DEFNKWDS);
    outInt(CCSID);       // Field's CCSID or 65535 for "Binary" or "No CCSID"
    outVarchar(ALWNULL); // Is the ALWNULL keyword in use for this field?

    outInt(STARTPOS);
    outInt(ENDPOS);
    outChar(FLDALIAS);   // 128-byte SQL Column name
    outChar(USAGE);
    outChar(COLHDG);  // 60-byte 3-part column headers
    outChar(TEXT);    // 50-byte Field Text (if any)

    outChar(FLDPROCPGM);
    outChar(FLDPROCLIB);

    outChar(FILEALIAS);
    outChar(FILETEXT);

    ////////////////////////////////////////////////////////////
    //  Input Parameters' INDICATORS
    ////////////////////////////////////////////////////////////

    inIndy(LIB);    // Library name to qualify the File name(s)
    inIndy(FILES);  // File name (generic, full or *ALL)
    inIndy(ATTRS);  // File attribute (i.e., PF, LF, DDMF, *ALL only)

    ////////////////////////////////////////////////////////////
    //  Output Columns' INDICATORS
    ////////////////////////////////////////////////////////////
    outIndy(OBJLIB);
    outIndy(FILENAME);
    outIndy(FILEATTR);    // File Attribute  (PF, LF, DDMF)
    outIndy(FILETYPE);    // Source or Database files (*SRC or *DATA)
    outIndy(SQLTYPE);     // SQL Object Type (TABLE, VIEW, INDEX)
    outIndy(CRTDATE);     // SQL Object Type (TABLE, VIEW, INDEX)

    outIndy(FLDCNT);       // Number of fields in this record format
    outIndy(RCDFMTCNT);    // Number of record formats

    outIndy(LVLCHK);
    outIndy(LVLCHKID);

    outIndy(RCDFMTTEXT);   // Record Format Text (if any) or NULL
    outIndy(RCDFMTID);       // 13-character Record format ID
    outIndy(RCDFMTNBR);
    outIndy(RCDFMT);       // Record format name
    outIndy(RCDLEN);       // Record format length

    outIndy(SEQNBR);     // ORDINAL_POSITION (field's sequence in the format)
    outIndy(FLDNAME);    // Short Field name
    outIndy(DATATYPE);   // SQL Data Type
    outIndy(BUFLEN);  // length in buffer
    outIndy(FLDLEN);
    outIndy(DECPOS);

    outIndy(RPGDEFN);  // RPG IV DCL-S statement
    outIndy(DEFNKWDS);
    outIndy(CCSID);    // Field's CCSID or 65535 for "Binary" or "No CCSID"
    outIndy(ALWNULL); // Is the ALWNULL keyword in use for this field?

    outIndy(STARTPOS); // FROM POSITION
    outIndy(ENDPOS);   // TO POSITION
    outIndy(FLDALIAS);   // 128-byte SQL Column name
    outIndy(USAGE);
    outIndy(COLHDG);  // 60-byte 3-part column headers
    outIndy(TEXT);    // 50-byte Field Text (if any)

    outIndy(FLDPROCPGM);    // 50-byte Field Text (if any)
    outIndy(FLDPROCLIB);    // 50-byte Field Text (if any)

    outIndy(FILEALIAS);    // 50-byte File Text (if any)
    outIndy(FILETEXT);    // 50-byte File Text (if any)

    ////////////////////////////////////////////////////////////
    //  SQL specific parameters
    ////////////////////////////////////////////////////////////
    char *sqlstate = (char *)coz::getNextParmIf(p, argc, argv);
    char *funcName = (char *)coz::getNextParmIf(p, argc, argv);
    char *specificName = (char *)coz::getNextParmIf(p, argc, argv);
    char *sqlmsgtext = (char *)coz::getNextParmIf(p, argc, argv);
    char *scratchPad = (char *)coz::getNextParmIf(p, argc, argv);
    int  *sqlOpCode = (int *)coz::getNextParmIf(p, argc, argv);

    ////////////////////////////////////////////////////////////
    //  BEGIN main() body (after parms starts here)
    ////////////////////////////////////////////////////////////

     scratch_t* pScratch = (scratch_t *)scratchPad;
     char*      pBuffer = NULL;
     char*      pFileDesc = NULL;
     char*      pRcdFmt   = NULL;
     char*      pRcdFmtList = NULL;
     bool       bSkipFile = false;

     std::string  stg;   // General purpose "quick use" string class implementation

     Qdb_Mbrd0300_t  mbrDesc;   // Member Description
     Qdb_Lrcd_RCDL0200_t rcdDesc;  // Record Format List Description Entries

     char OBJD_APIFMT[] = "OBJD0200";
     char MBRD_APIFMT[] = "MBRD0300";

     char APIFMT1[] = "FILD0100";  // File attributes
     char APIFMT2[] = "FILD0200";  // RcdFmt attributes

     char fileInfo[11];
     coz::copyPad(fileInfo,"*FILETYPE", 10);

     char  qualFile[21];
     char  qualLib[21];

     char  rtnFileLib[21];
     char  szFileName[11];
     char  szLibName[11];

     char  overrides[2] = "0";

     Qdb_Qdbfh_t*    fdHdr = NULL;   // FILD0100
     Qdb_Qddfmt_t*   fdFmt = NULL;   // FILD0200
     Qdb_Qddffld_t*  ffd = NULL;     // Qddffld Field Definition Array

     Qdb_Qdbfphys_t* fdPhyAttr = NULL;
     Qdb_Qdbflogl_t* fdLglAttr = NULL;
     Qdb_Qaaf2_t*    pAddlFlags = NULL;
     Qdb_Qdbfhflg_t* pFileFlags = NULL;
     Qdb_Qddfrefi_t* pRef = NULL;
     Qdb_Qdbfdic_t*  pIDDU = NULL;
     Qdb_Qdbflngn_t* pFileAlias = NULL;
     Qdb_Qdbfjoal_t* pJRN = NULL;
     Qdb_Qdbfb_t*    pScope = NULL;


    if (*sqlOpCode == SQLUDF_TF_OPEN)
    {
        // DO ONCE STUFF GOES HERE


        // Initialize Scratch pad
        memset(pScratch, 0x00, sizeof(scratch));
        pScratch->IBMiVRM = coz::getOSVer();

        memset(pScratch->QFILEINFO, ' ', sizeof(scratch.QFILEINFO));
        memset(pScratch->QRCDINFO, ' ', sizeof(scratch.QRCDINFO));
        memset(pScratch->RCDFMTLIST, ' ', sizeof(scratch.RCDFMTLIST));
        memset(pScratch->QHANDLES, ' ', sizeof(scratch.QHANDLES));

        pFileDesc = (char*) coz::TEMP_USRSPACE(pScratch->QFILEINFO, "FILEINFO", coz::MAXMEM15M);   // Workspace for the API call results
        pRcdFmt   = (char*) coz::TEMP_USRSPACE(pScratch->QRCDINFO,   "RCDINFO",  coz::MAXMEM15M);    // The data in the Record Format area
        pRcdFmtList   = (char*) coz::TEMP_USRSPACE(pScratch->RCDFMTLIST, "RCDFMTLIST");    // The data in the Record Fromat area
        coz::openListHandles_t *pHandle = (coz::openListHandles_t *)coz::TEMP_USRSPACE(pScratch->QHANDLES, "hOBJLIST");   // Object List file handles
        pHandle->count = 0;

           // The inFILES parameter may contain long or short object names.
           // They may be qualified or unqualified.
           // Unqualified names shall be qualfied to the library name
           // specified on the inLIB parameter.

        if (*indyInATTRS >= 0 && strlen(inATTRS) > 0)
        {
            coz::toUpper(inATTRS);
            stg.assign(inATTRS);
            bool ALL = (stg.find("ALL") !=std::string::npos) ? true : false;

            if (ALL)
            {
              pScratch->PF = true;
              pScratch->LF = true;
              pScratch->DDMF = true;
              pScratch->SRC = true;
              pScratch->DATA = true;
            }
            else
            {
              pScratch->PF = (stg.find("PF")!=std::string::npos ||
                              stg.find("PHY") !=std::string::npos) ? true : false;
              pScratch->LF = (stg.find("LF")!=std::string::npos ||
                              stg.find("LGL") !=std::string::npos) ? true : false;
              pScratch->DDMF = (stg.find("DDMF")!=std::string::npos ||
                              stg.find("DDM") !=std::string::npos) ? true : false;
              pScratch->SRC = (stg.find("SRC") !=std::string::npos ||
                               stg.find("SOURCE") !=std::string::npos) ? true : false;
              pScratch->DATA = (stg.find("DATA") !=std::string::npos) ? true : false;
              if (!pScratch->SRC && !pScratch->DATA)
              {
                pScratch->SRC = true;
                pScratch->DATA = true;
              }
            }
            if (!pScratch->PF && !pScratch->LF && !pScratch->DDMF)
            {
               if (pScratch->DATA)
               {
                  pScratch->PF = true;
                  pScratch->LF = true;
                  pScratch->DDMF = true;
               }
               if (pScratch->SRC)
               {
                  pScratch->PF = true;
               }
            }

        }
        else
        {
          pScratch->PF = true;
          pScratch->LF = true;
          pScratch->DDMF = true;
        }

        char szLib[129];
        if (*indyInLIB >= 0 && strlen(inLIB) > 0)
        {
            coz::nameUpper(inLIB);
            strcpy(szLib,inLIB);
        }
        else
        {
           strcpy(szLib, "*LIBL");
        }
        // If multiple file names are specified,
        // we do multiple Open List objects
        // and copy each handle into the File Space
        if (*indyInFILES >= 0 && inFILES->length > 0)
        {
            // Create an Open List for each File name specified.
            // Store the open list handles in the File Handles space.
            pScratch->LISTSIZE = addToStack(pHandle, inFILES, inLIB, pScratch);
        }

        if (pHandle->count == 0)
        {
            strcpy(sqlstate, "02000");
            pScratch->eof = 1;
            return -1;
        }

        // If filtering by File Attribute (PF, LF, DDMF) then
        // set the flags in the scratchpad accordingly.
        // The default is *ALL which accepts all 3 data file types.
        // Note that a file must be either *SRC or *DATA and
        // one of these attributes to be included.
    }

    if (*sqlOpCode == SQLUDF_TF_FETCH && pScratch->eof == 1)
    {
        strcpy(sqlstate, "02000");
        pScratch->eof = 1;
    }

    // ------------------------------------------------------
    // F E T C H   Operation
    // ------------------------------------------------------
    if (*sqlOpCode == SQLUDF_TF_FETCH && pScratch->eof == 0)
    {
        int                   keyCount   = 0;
        int                   keyPos     = 0;
        int                   lnCount    = 0;
        int                   len        = 0;
        int                   rtvBuffLen = 0;

        object_Desc_t*          pObjDesc   = NULL;
        Qgy_Olobj_KeyData_t*    pKeyData   = NULL;
        coz::openListHandles_t* pHandle    = NULL;
        Qdb_Lrcd_RCDL0200_t*    pRcdFmtEntry = NULL;
        char*                   pData      = NULL;
        char*                   pUserInfo  = NULL;
        char*                   pOutput    = NULL;
        char                    szRcdFmt[11];

        ec.init();

        pFileDesc   = coz::getPtrUsrSpace(pScratch->QFILEINFO);
        pRcdFmt     = coz::getPtrUsrSpace(pScratch->QRCDINFO);
        pRcdFmtList = coz::getPtrUsrSpace(pScratch->RCDFMTLIST);    // The data in the Record Fromat area
        pHandle     = (coz::openListHandles_t*) coz::getPtrUsrSpace(pScratch->QHANDLES);

        // Qp0zLprintf("PTR QFILEINFO(%.10s/%.10s)=%p\n",
        //             pScratch->QFILEINFO,    pScratch->QFILEINFO+10,  (void*)pFileDesc);
        // Qp0zLprintf("PTR QRCDINFO(%.10s/%.10s)=%p\n",
        //             pScratch->QRCDINFO,     pScratch->QRCDINFO+10,   (void*)pRcdFmt);
        // Qp0zLprintf("PTR RCDFMTLIST(%.10s/%.10s)=%p\n",
        //             pScratch->RCDFMTLIST,   pScratch->RCDFMTLIST+10, (void*)pRcdFmtList);
        // Qp0zLprintf("PTR QHANDLES(%.10s/%.10s)=%p\n",
        //             pScratch->QHANDLES,     pScratch->QHANDLES+10,   (void*)pHandle);


        CObjList   ol;
        coz::listHANDLE hList;

        if (getLastHandle(pHandle, hList) == 0)
        {
          strcpy(sqlstate, "02000");
          pScratch->eof = 1;
          return -1;
        }

        if (pScratch->NEXTFLD == 0 || pScratch->NEXTFLD >= pScratch->NBRFIELDS)
        do
        {
          if (pScratch->nbrOf_rcdFmt > 1 &&
              pScratch->rcdFmt_Index > 0 &&
              pScratch->rcdFmt_Index < pScratch->nbrOf_rcdFmt)
          {
            pScratch->NEXTFLD = 0;  // Field to read this paass, 0-based array
            pScratch->NEXTFLDOFFSET = 0;  // Field to read this paass, 0-based array
            break;  // additional conditioning but placed inside this IF instead of the "do"
          }

          bSkipFile = false;
          pScratch->NEXTOBJ++;    // Bump it up to read the "next" File Profile (if any)
          ol.attach(hList, pScratch->dbfAttr);       // Assign handle to object list
          ol.setNextObj(pScratch->NEXTOBJ);

          while ((pObjDesc = (object_Desc_t*) ol.getNextObj()) == NULL)
          {
       //     ol.close();   // close the last list handle (now performed in _CLOSE routine)
            if (popLastHandle(pHandle) == 0 ||
                getLastHandle(pHandle, hList) == 0)
            {
                strcpy(sqlstate, "02000");
                pScratch->eof = 1;
                return -1;
            }
            ol.attach(hList, pScratch->dbfAttr); // Assign handle to object list
            pScratch->NEXTOBJ = 1; // We read the first row so get the next one on next pass
            ol.setNextObj(1);   // New list so start at the start
          }

          pScratch->NEXTFLD = 0;  // Field to read this paass, 0-based array
          pScratch->NEXTFLDOFFSET = 0;  // Field to read this paass, 0-based array

          int keyCount = ol.getKeyCount();  // pObjDesc->id.Num_Fields_Retd

          for (int keyPos = 0; keyPos < keyCount; keyPos++)
          {
               // Used GetKeyDataByPos to process key data sequencially
               // We are only using the basic user-defined objDesc format
               // which is format f150
               // Note: The CObjDesc object filters out non-PF, LF, and DDMF
               //       files automagically
              pKeyData = (Qgy_Olobj_KeyData_t*)ol.getDataByPos(keyPos);
              if (pKeyData != NULL)
              {
                    pData = ((char *)pKeyData) + sizeof(Qgy_Olobj_KeyData_t);
                    switch (pKeyData->Key_Field)
                    {
                    case 304: // Creation Date a DTS[8]
                        pOutput = outCRTDATE;
                        *indyCRTDATE = (coz::cvtSYSDTStoDTS(pOutput, pData) == NULL) ? -1 : 0;
                        if (*indyCRTDATE == 0)
                        {
                           strcpy(pScratch->fileCRTDATE, outCRTDATE);
                        }
                        break;
                    case 202: // Object (extended) Attribute (PF, LF, DDMF)
                        pOutput = outFILEATTR;
                        _CPYBYTES(pOutput, pData, pKeyData->Data_Length);
                        pOutput[pKeyData->Data_Length] = 0x00;
                        pOutput[::triml(pOutput, ' ')] = 0x00;
                        if (!((memcmp(pOutput,"PF",2)==0    && pScratch->PF) ||
                              (memcmp(pOutput,"LF",2)==0    && pScratch->LF) ||
                              (memcmp(pOutput,"DDMF",4)==0  && pScratch->DDMF)))
                        {
                          bSkipFile = true;
                          break;
                        }
                        break;
                    } // end switch
                }     // end if keyData is not null

                if (bSkipFile) {
                    break; // leave for loop
                }
          }       // end for loop


         if (bSkipFile) continue; // skip this object and go get next

          coz::copyUntil(outFILENAME, pObjDesc->id.Obj_Name, 10);
          coz::copyUntil(outOBJLIB,   pObjDesc->id.Obj_Lib,  10);

         coz::makeAPIObjName(qualFile, outFILENAME, outOBJLIB);
         coz::copyPad(fileInfo,"*FILETYPE",10);
         if (strcmp(outFILEATTR,"DDMF")==0)
         {
               // TODO: Add DDM related adjustments to the API call, here.
            coz::copyPad(fileInfo,"*RMT",10);
         }
         if (pScratch->NEXTFLD == 0)
         {
          ec.init();
          rtvBuffLen = coz::MAXMEM15M;
          QDBRTVFD(pFileDesc,            /* Receiver variable           */
                   rtvBuffLen,           /* Receiver length             */
                   rtnFileLib,           /* Returned file and library   */
                   APIFMT1,              /* File definition template    */
                   qualFile,             /* File and library name       */
                   "*FIRST    ",         /* *FIRST Record Format (not used here)  */
                   overrides,            /* No override processing      */
                   fileInfo,             /* LOCAL/RMT/FILETYPE system   */
                   "*EXT      ",         /* External format (not used here)       */
                   &ec);

          if (ec.isNotEmpty() &&
              (ec.compare("CPF9162")  ||
               ec.compare("CPD3E37")  ||
               ec.compare("CPF32")    ||
               ec.compare("CPF3021"))  // Can't connect to remote?
             )
          {
             resignalMsg( ec );
          }


          if (ec.isNotEmpty() || ((Qdb_Qdbfh_t*) pFileDesc)->Qdbfyret == 0)
          {
             signalEOFFunc();
             return -1;
          }
          coz::copyUntil(pScratch->fileName, rtnFileLib, 10);
          coz::copyUntil(pScratch->fileLib,  rtnFileLib+10,  10);

        }

         //  _CPYBYTES(rtnFileLib, qualFile, sizeof(qualFile));

            // If we got the file attributes, then get the record format attributes
          fdHdr = (Qdb_Qdbfh_t*) pFileDesc;
          pFileFlags = &(fdHdr->Qdbfhflg);
          pAddlFlags = &(fdHdr->Qaaf2);
          pScope = (Qdb_Qdbfb_t*) (pFileDesc + (fdHdr->Qdbfos));

          if (!pScratch->SRC && (pFileFlags->Qdbfhfsu & 0x01) == 1)
          {
              bSkipFile = true;
              continue;
          }
          if (!pScratch->DATA && (pFileFlags->Qdbfhfsu & 0x01) != 1)
          {
              bSkipFile = true;
              continue;
          }
        } while (bSkipFile); // end DO WHILE (which is basically a "Do Until")


           // If we got the file attributes, then get the record format attributes
         coz::copyUntil(outFILENAME,pScratch->fileName);
         coz::copyUntil(outOBJLIB, pScratch->fileLib);

         fdHdr = (Qdb_Qdbfh_t*) pFileDesc;
         pFileFlags = &(fdHdr->Qdbfhflg);
         pAddlFlags = &(fdHdr->Qaaf2);
         if ((pFileFlags->Qdbfhfpl & 0x01)==0x00)   // Is PF (physical file)?
         {
            fdPhyAttr  = (Qdb_Qdbfphys_t*)(pFileDesc + fdHdr->Qdbpfof);
            fdLglAttr  = NULL;
             strcpy(outSQLTYPE,((fdPhyAttr->Qflags.Qdbfsqlt & 0x01)==1) ? "TABLE" : "");
         }
         else if (pFileFlags->Qdbfhfpl & 0x01)    // Is LF (logical file)?
         {
            fdLglAttr  = (Qdb_Qdbflogl_t*)(pFileDesc + fdHdr->Qdblfof);
            fdPhyAttr  = NULL;
            strcpy(outSQLTYPE,((fdLglAttr->Qlfa.Qdbfsqlv & 0x01)==1) ? "VIEW" :
                              ((fdLglAttr->Qlfa.Qdbfsqli & 0x01)==1) ? "INDEX" : "");
         }

         ec.init();
         rtvBuffLen = coz::MAXMEM15M;

         strcpy(outCRTDATE,pScratch->fileCRTDATE);
         *indyCRTDATE = (strlen(outCRTDATE) > 0) ? 0 : -1;
         strcpy(outFILEATTR,((pFileFlags->Qdbfhfpl & 0x01)==0) ? "PF" : "LF");
         strcpy(outFILETYPE,((pFileFlags->Qdbfhfsu & 0x01)==1) ? "*SRC" : "*DATA");

      //   strcpy((pFileFlags->Qdbfdisf & 0x01)==1) ? "*DIST" : "*NONDIST");

         if (strcmp(outFILEATTR,"DDMF")==0)
         {
               // TODO: Add DDM related adjustments to the QDBRTVFD API call, here.
            coz::copyPad(fileInfo,"*FILETYPE",10);
         }

         if (pScratch->rcdFmt_Index >= pScratch->nbrOf_rcdFmt || pScratch->nbrOf_rcdFmt==0)
         {
            pScratch->nbrOf_rcdFmt = buildRcdFmtList(pScratch->RCDFMTLIST, pScratch->fileName, pScratch->fileLib);
            // pScratch->rcdFmt_Index = 0;
         }

         if (pScratch->NEXTFLD == 0)
         {
          pRcdFmtEntry = getRcdFmtEntry(pRcdFmtList, pScratch->rcdFmt_Index);

          if (pRcdFmtEntry != NULL)
          {
            coz::copyUntil(pScratch->rcdFmt_ID,   pRcdFmtEntry->Format_Id,13);
            _CPYBYTES(szRcdFmt, pRcdFmtEntry->Format_Name,10);
            coz::copyUntil(pScratch->rcdFmt_TEXT, pRcdFmtEntry->Record_Text_Description,50);
            pScratch->rcdFmt_Index++;
          }
          else
          {
            _CPYBYTES(szRcdFmt, "*FIRST    ",10);
          }


              // Now get the file's first or only record format
           QDBRTVFD(pRcdFmt,             /* Receiver variable            */
                   rtvBuffLen,           /* Receiver length              */
                   rtnFileLib,           /* Returned file and library    */
                   APIFMT2,              /* File definition template     */
                   qualFile,             /* File and library name        */
                   szRcdFmt,             /* Record format FILD0200       */
                   overrides,            /* override processing          */
                   fileInfo,             /* Local system                 */
                   "*EXT      ",         /* External format              */
                   &ec);

            fdFmt = (Qdb_Qddfmt_t*) pRcdFmt;
            pScratch->NBRFIELDS = fdFmt->Qddffldnum;
         }
         else
         {
           fdFmt = (Qdb_Qddfmt_t*) pRcdFmt;
         }

         int recdLen = fdFmt->Qddfrlen;
         coz::copyUntil(outRCDFMT, fdFmt->Qddfname,10);


         Qdb_Qddffld_t  FFD;
         Qdb_Qddfmt_t   FMT;

          // Calculate pointer to the current field's index in the Qddffldx array
         ffd = (Qdb_Qddffld_t*) (pRcdFmt + sizeof(FMT) + pScratch->NEXTFLDOFFSET);

         pScratch->NEXTFLD++;
         pScratch->NEXTFLDOFFSET += ffd->Qddfdefl;
         pScratch->counter++;
         *outSEQNBR = pScratch->NEXTFLD;

         coz::copyUntil(outFLDNAME, ffd->Qddfflde,sizeof(FFD.Qddfflde));
         coz::copyUntil(outRCDFMTTEXT, pScratch->rcdFmt_TEXT, 50);
         coz::copyUntil(outRCDFMTID,   pScratch->rcdFmt_ID, 13);
         *outRCDFMTNBR = pScratch->rcdFmt_Index;

         char fmtBuff[64];
         unsigned short dt = *((unsigned short*)ffd->Qddfftyp);
         bool     bNUMERIC = false;
         bool     bDATE    = false;
         bool     bLOB     = false;
         switch (dt)
         {
             case 0x0000:
                strcpy(outDATATYPE,"INT");
                bNUMERIC = true;
                break;
             case 0x0001:
                strcpy(outDATATYPE,"FLOAT");
                bNUMERIC = true;
                break;
             case 0x0002:
                strcpy(outDATATYPE,"NUMERIC");
                bNUMERIC = true;
                break;
             case 0x0003:
                strcpy(outDATATYPE,"DEC");
                bNUMERIC = true;
                break;
             case 0x0004:
                strcpy(outDATATYPE,"CHAR");
                break;
             case 0x8004:
                strcpy(outDATATYPE,"VARCHAR");
                break;
             case 0x0005:
                strcpy(outDATATYPE,"GRAPHIC");
                break;
             case 0x8005:
                strcpy(outDATATYPE,"VARGRAPHIC");
                break;
             case 0x0006:
                strcpy(outDATATYPE,"DBCS-CAPABLE");
                break;
             case 0x8006:
                strcpy(outDATATYPE,"VAR DBCS-CAPABLE");
                break;
             case 0x000B:
                strcpy(outDATATYPE,"DATE");
                bDATE = true;
                break;
             case 0x000C:
                strcpy(outDATATYPE,"TIME");
                bDATE = true;
                break;
             case 0x000D:
                strcpy(outDATATYPE,"TIMESTAMP");
                bDATE = true;
                break;
             case 0x000F:
                strcpy(outDATATYPE,"DECFLOAT");
                bNUMERIC = true;
                break;
             case 0x4004:
                strcpy(outDATATYPE,"BLOB/CLOB");
                bLOB = true;
                break;
             case 0x4005:
                strcpy(outDATATYPE,"DBCLOB");
                bLOB = true;
                break;
             case 0x4006:
                strcpy(outDATATYPE,"CLOB-OPEN");
                bLOB = true;
                break;
             case 0x8044:
                strcpy(outDATATYPE,"DATALINK-CHAR");
                break;
             case 0x8046:
                strcpy(outDATATYPE,"DATALINK-OPEN");
                break;
             case 0xFFFF:
                strcpy(outDATATYPE,"NULL");
                break;
         }
         strcpy(fmtBuff, (dt == 0x000B) ? "DATFMT(%s)" :
                         (dt == 0x000C) ? "TIMFMT(%s)" :
                         (dt == 0x000D) ? "DATFMT(%s)" : "");
         switch (ffd->Qddfdttf)
         {
             case 0xfe:
               sprintf(outDEFNKWDS,fmtBuff,"*JOB");
               break;
             case 0x01:
               sprintf(outDEFNKWDS,fmtBuff,"*USA");
               break;
             case 0x03:
               sprintf(outDEFNKWDS,fmtBuff,"*ISO");
               break;
             case 0x05:
               sprintf(outDEFNKWDS,fmtBuff,"*EUR");
               break;
             case 0x07:
               sprintf(outDEFNKWDS,fmtBuff,"*JIS");
               break;
             case 0x09:
                // Do not add DATFMT(*SAA) for TImeStamps
               // sprintf(outDEFNKWDS,fmtBuff,"*SAA");
               memset(fmtBuff,0x00,sizeof(fmtBuff));
               break;
             case 0x17:
               sprintf(outDEFNKWDS,fmtBuff,"*MDY");
               break;
             case 0x18:
               sprintf(outDEFNKWDS,fmtBuff,"*DMY");
               break;
             case 0x19:
               sprintf(outDEFNKWDS,fmtBuff,"*YMD");
               break;
             case 0x1a:
               sprintf(outDEFNKWDS,fmtBuff,"*JUL");
               break;
             case 0x1b:
               sprintf(outDEFNKWDS,fmtBuff,"*HMS");
               break;
             case 0x25:
               sprintf(outDEFNKWDS,fmtBuff,"*CMDY");
               break;
             case 0x26:
               sprintf(outDEFNKWDS,fmtBuff,"*CDMY");
               break;
             case 0x27:
               sprintf(outDEFNKWDS,fmtBuff,"*CYMD");
               break;
             case 0x28:
               sprintf(outDEFNKWDS,fmtBuff,"*MDYY");
               break;
             case 0x29:
               sprintf(outDEFNKWDS,fmtBuff,"*DMYY");
               break;
             case 0x2A:
               sprintf(outDEFNKWDS,fmtBuff,"*YYMD");
               break;
             case 0x2B:
               sprintf(outDEFNKWDS,fmtBuff,"*YM");
               break;
             case 0x2C:
               sprintf(outDEFNKWDS,fmtBuff,"*MY");
               break;
             case 0x2D:
               sprintf(outDEFNKWDS,fmtBuff,"*YYM");
               break;
             case 0x2E:
               sprintf(outDEFNKWDS,fmtBuff,"*MYY");
               break;
             case 0x30:
               sprintf(outDEFNKWDS,fmtBuff,"*LONGJUL");
               break;
         }
         if (dt == 0x000B ||
             dt == 0x000C)
         {
           strcpy(fmtBuff, (dt == 0x000B) ? "%s DATSEP('%s')" :
                           (dt == 0x000C) ? "%s TIMSEP('%s')" : "");
           switch (ffd->Qddfdtts)
           {
             case 0x00:
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS,"*JOB");
               break;
             case '/':
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS,"/");
               break;
             case '-':
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS,"-");
               break;
             case '.':
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS,".");
               break;
             case ' ':
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS," ");
               break;
             case ':':
               sprintf(outDEFNKWDS,fmtBuff,outDEFNKWDS,":");
               break;
           }
         }

         switch (ffd->Qddffiob)
         {
            case 0x01:
               sprintf(outUSAGE,"INPUT");
               break;
            case 0x02:
               sprintf(outUSAGE,"OUTPUT");
               break;
            case 0x03:
               sprintf(outUSAGE,"BOTH");
               break;
            case 0x04:
               sprintf(outUSAGE,"NONE");
               break;
            case 0xFF:
               sprintf(outUSAGE,"UNKNONW");
               break;
         }

         memset(fmtBuff,0x00,sizeof(fmtBuff));
         *outCCSID = 65535; // Initialized to binary but change later if has a CCSID
         if (ffd->Qddfcsid != 0)
         {
            if (!(dt == 0x4004 &&
                  ffd->Qdducs2co.Qddfencode[0]==0 &&
                  ffd->Qdducs2co.Qddfencode[1]==0))
            {
              sprintf(fmtBuff, "CCSID(%d)", ffd->Qddfcsid);
              coz::BCAT(outDEFNKWDS, fmtBuff);
              *outCCSID = ffd->Qddfcsid;
            }
         }
                                     //  Qddffldb  GRAPHIC is Number of Bytes
                                     //  Qddffldd  GRAPHIC is Number of characters
         *outBUFLEN  = ffd->Qddffldb;
         *outSTARTPOS = ffd->Qddffibo + 1;
         *outENDPOS   = ffd->Qddffibo + ffd->Qddffldb;
         if (bNUMERIC)
         {
            *outFLDLEN = ffd->Qddffldd;
            *outDECPOS = ffd->Qddffldp;
            if (strcmp(outDATATYPE,"DEC")==0)
            {
               sprintf(outRPGDEFN, "%s(%d:%d)", "PACKED",ffd->Qddffldd,ffd->Qddffldp);
            }
            else if (strcmp(outDATATYPE,"NUMERIC")==0)
            {
               sprintf(outRPGDEFN, "%s(%d:%d)", "ZONED",ffd->Qddffldd,ffd->Qddffldp);
            }
            else if (strcmp(outDATATYPE,"GRAPHIC")==0)
            {
               sprintf(outRPGDEFN, "%s(%d:%d)", "GRAPH",ffd->Qddffldd,ffd->Qddffldp);
            }
            else if (strcmp(outDATATYPE,"INT")==0)
            {
               // "BINARY" fields are actually Integers
               sprintf(outRPGDEFN, "INT(%d)", (ffd->Qddffldd==4) ? 5 :
                                              (ffd->Qddffldd==9) ? 10 :
                                              (ffd->Qddffldd > 9) ? 20 :
                                              (ffd->Qddffldd < 4) ? 3 : 0);
               switch (ffd->Qddffldd)
               {
                  case 4:
                  case 5:
                    strcpy(outDATATYPE,"SMALLINT");
                    break;
                  case 1:
                  case 3:
                    strcpy(outDATATYPE,"TINY");
                    break;
                  case 9:
                  case 10:
                    strcpy(outDATATYPE,"INT");
                    break;
                  case 18:
                  case 19:
                  case 20:
                    strcpy(outDATATYPE,"BIGINT");
                    break;
               }
            }
            else
            {
               sprintf(outRPGDEFN, "%s(%d:%d)", outDATATYPE,ffd->Qddffldd,ffd->Qddffldp);
            }
         }
         else
         {
              //  Qddffldb  When GRAPHIC is Number of Bytes
              //  Qddffldd  When GRAPHIC is Number of characters
            *outFLDLEN = ffd->Qddffldb;
            if (dt == 0x0005 ||    // GRAPHIC
                dt == 0x8005)      // VARGRAPHIC
            {
               *outFLDLEN = ffd->Qddffldd;  // Use Number of characters not byte count
               sprintf(outRPGDEFN, "%sGRAPH(%d)", (dt == 0x8005)? "VAR" : "", *outFLDLEN);
            }
            else if (dt == 0x8004 )      // VARCHAR
            {
               *outFLDLEN -= 2;
            }
            else if (dt == 0x4004 ||    // xLOB
                dt == 0x4005 ||
                dt == 0x4006)
            {                                         //DBCLOB
               *outFLDLEN = ffd->Qddfbmaxl;
               if (ffd->Qdducs2co.Qddfencode[0]==0 &&
                   ffd->Qdducs2co.Qddfencode[1]==0)
               {
                 strcpy(outDATATYPE,"BLOB");
                 sprintf(outRPGDEFN, "SQLTYPE(BLOB:%d)", *outFLDLEN);
               }
               else
               {
                 strcpy(outDATATYPE,"CLOB");
                 sprintf(outRPGDEFN, "SQLTYPE(CLOB:%d)", *outFLDLEN);
               }
            }
            else if (dt == 0x000B ||    // Date/Time/Timestamp?
                     dt == 0x000C ||
                     dt == 0x000D)
            {                                         //DBCLOB
              sprintf(outRPGDEFN, "%s", outDATATYPE);
            }
            else
            {
              sprintf(outRPGDEFN, "%s(%d)", outDATATYPE,*outFLDLEN);
            }
            *indyDECPOS = -1;
         }
       //  *outBUFLEN   = ffd->Qddflalc;

            // Flags 1 - field status flags ffd->Qddffldst -- NOT USED

            // Flags 2
           if ((ffd->Qddffldst2.Qddffnul & 0x01)==0x01)
           {
             strcpy(outALWNULL,"*YES");
             if (strlen(outDEFNKWDS) > 0)
             {
             strcat(outDEFNKWDS, " ALWNULL");  // If Allow Nulls, then add ALWNULL
             }
             else
             {
               strcpy(outDEFNKWDS,"ALWNULL");  // No other kwds? Then just copy ALWNULL when needed
             }
           }
           else
           {
            strcpy(outALWNULL,"*NO");
           }

    //     strcpy(outVARFIELD, ((ffd->Qddffldst2.Qddffvar  & 0x01)==0x01) ? "VARY" : "");

         if (ffd->Qdducs2co.Qddf_Row_Change_Tmstmp_Generat=='1')
         {
             // TODO:  maybe do something with "generate always as timestamp"
         }
            // RCAC procedure (program) to call
         coz::copyUntil(outFLDPROCPGM,ffd->Qdducs2co.Qddf_Fld_Proc_Program);
         coz::copyUntil(outFLDPROCLIB,ffd->Qdducs2co.Qddf_Fld_Proc_Library);
   //      pIDDU = ((char*)ffd) + ffd->Qddfdicd;

           // TEXT
         if (ffd->Qddftxtd > 0)
         {
           pBuffer = ((char*)ffd) + ffd->Qddftxtd;
           Qdb_Qddfftxt_t* pText = (Qdb_Qddfftxt_t*)pBuffer;
           coz::copyUntil(outTEXT, pText->Qddfftxt,50);
         }

           // COLHDG
         if (ffd->Qddfchd > 0)
         {
          pBuffer = ((char*)ffd) + ffd->Qddfchd;
          Qdb_Qddfcolh_t* pCOLHDG = (Qdb_Qddfcolh_t*)pBuffer;
          coz::copyUntil(outCOLHDG, (char*)pCOLHDG,60);
         }

          // ALIAS
         if (ffd->Qddfxals > 0)
         {
           pBuffer = ((char*)ffd) + ffd->Qddfxals;
           Qdb_Qddfalis_t* pAlias = (Qdb_Qddfalis_t*)pBuffer;
           char* pAliasName = pBuffer + sizeof(Qdb_Qddfalis_t);
           coz::copyUntil(outFLDALIAS, pAliasName, pAlias->Qddfalsl);
         }

          // If no ALIAS, default to "short" field name for Field ALIAS name
         if (strlen(outFLDALIAS) == 0)
         {
            strcpy(outFLDALIAS, outFLDNAME);
         }


           // long file name?
         if (fdHdr->Qdbfodic)
         {
           pIDDU = (Qdb_Qdbfdic_t*)(pFileDesc + fdHdr->Qdbfodic);
           if (pIDDU != NULL && pIDDU->Qdbflnnum > 0)
           {
             lnCount = pIDDU->Qdbflnnum;  // Alias/LongName count
             pFileAlias= (Qdb_Qdbflngn_t*)(pFileDesc + pIDDU->Qdbfolng);
             if (pFileAlias->Qdbflnel > 0)
             {
               len = pFileAlias->Qdbflnlg;  // Long name length
               char* pLongName = (char*)pFileAlias + sizeof(Qdb_Qdbflngn_t);
               coz::copyUntil(outFILEALIAS,pLongName ,len);
             }
           }
         }
           // If there is no Alias (aka "Long SQL Name")
           // then copy the short system object file name to the long name.
         if (strlen(outFILEALIAS) == 0)
         {
            strcpy(outFILEALIAS, outFILENAME);
         }

           *outRCDLEN = fdHdr->Qdbfmxrl;
           *outFLDCNT = fdHdr->Qdbfmxfnum;
           *outRCDFMTCNT = fdHdr->Qdbffmtnum;
           coz::copyUntil(outLVLCHKID, fdHdr->Qdbfhcrt,13);
           coz::copyUntil(outLVLCHK,   ((pFileFlags->Qdbfhflc & 0x01) == 1) ? "*YES" : "*NO");

           coz::copyUntil(outFILETEXT,fdHdr->Qdbfhtx.Qdbfhtxt,50);


           coz::copyUntil(outFILETYPE, ((pFileFlags->Qdbfhfsu & 0x01) == 1) ? "*SRC" : "*DATA");

    }

    if (*sqlOpCode == SQLUDF_TF_CLOSE)
    {
        // Qp0zLprintf("...");

        coz::openListHandles_t* pHandles = (coz::openListHandles_t *)coz::getPtrUsrSpace(pScratch->QHANDLES);
        closeAllHandles(pHandles);

        coz::DLTOBJ(pScratch->QHANDLES,  "*USRSPC");
        coz::DLTOBJ(pScratch->QFILEINFO, "*USRSPC");
        coz::DLTOBJ(pScratch->QRCDINFO,  "*USRSPC");
        coz::DLTOBJ(pScratch->RCDFMTLIST,  "*USRSPC");
    }
    if (*sqlOpCode == SQLUDF_TF_FINAL ||
        *sqlOpCode == SQLUDF_TF_FINAL_CRA)
    {
        // no final call code for now
    }
}

// Parses input object names list.
// Opens the QGYOLOBJ list for each name.
// Stores the handle of each open list in
// the user space via the handles parameter.
// Note that generic names are supported, hence the use of CObjList
int addToStack(coz::openListHandles_t *pHandle, SQLUDF_CLOB* pObjects, const char* pDftLib, scratch_t *pScratch)
{
    char delims[] = ", ";
    char *pToken = NULL;
    char *pValue = NULL;
    char *pQualObj = NULL;
    char *pObjList = NULL;

    const int QUALNAME = 20;
    int hOffset = 0;
    int val_len = 0;
    int counter = 0;
    int objCount = 0;
    char szObject[11];
    char szLib[11];
    coz::listHANDLE hList;

    CObjList* ol = NULL;
    char* pQualList = coz::getPtrUsrSpace(pScratch->QFILEINFO);

      // Because we are going to re-attach the handles
      // the saved attributes are not retained with the object list handle.
      // So this isn't needed and won't work. We instead do the
      // the filtering by PF, LF, and DDMF via the getNextObj() loop (during _FETCH)
    memset(pScratch->dbfAttr,0x00,sizeof(pScratch->dbfAttr));
    if (pScratch->PF)    strcat(pScratch->dbfAttr, "PF ");
    if (pScratch->LF)    strcat(pScratch->dbfAttr, "LF ");
    if (pScratch->DDMF)  strcat(pScratch->dbfAttr, "DDMF ");

    pObjects->data[ pObjects->length ] = 0x00;  // Treat the clob like a NULL-terminated string
    pObjList = pObjects->data;

    int fileNames = coz::makeQualListFromLongName(pQualList, pObjList, pDftLib);

    for (int i = 0 ; i < fileNames ; i++)
    {
        pQualObj = pQualList + (i * coz::MAX2PARTNAME_LEN);

        coz::copyUntil(szObject, pQualObj,    10);
        coz::copyUntil(szLib,    pQualObj+10, 10);
        ol = new CObjList();
        objCount = ol->openList(szObject, szLib, "*FILE", pScratch->dbfAttr);
        if (objCount > 0)
        {
            counter += objCount;
            ol->getHandle(hList);
            ol->setDefer(true); // Avoid closing list on class destructor
            _CPYBYTES((char*)pHandle->handle[pHandle->count], (char*)hList, sizeof(hList));
            pHandle->count++;
            delete ol;
            ol = NULL;
        }
        else
        {
          ol->close();
          delete ol;
          ol = NULL;
        }
    }
    if (ol != NULL) delete ol;
    return counter;
}

int getLastHandle(coz::openListHandles_t *pHandle, coz::listHANDLE& hList)
{
    if (pHandle->count > 0)
    {
        _CPYBYTES((char*)hList, (char*)pHandle->handle[pHandle->count - 1], sizeof(hList));
        return pHandle->count;
    }
    return 0;
}
int closeAllHandles(coz::openListHandles_t *pHandle)
{
     CObjList ol;
     for (int h = 0 ; h < pHandle->count ; h++)
     {
       if (!coz::isAllHexZeros((char*)pHandle->handle[pHandle->count], 4))
       {
         ol.attach(pHandle->handle[h]);
         ol.close();
         memset((char*)pHandle->handle[pHandle->count], 0x00, 4);
       }
     }
     return pHandle->count;
}

int popLastHandle(coz::openListHandles_t *pHandle)
{
    CObjList ol;

    if (pHandle->count > 0)
    {
        pHandle->count--;
        ol.attach(pHandle->handle[pHandle->count]);
        ol.close();
        memset((char*)pHandle->handle[pHandle->count], 0x00, 4);
    }
    return pHandle->count;
}

  // typedef _Packed struct Qdb_Lrcd_RCDL0200
  //  {
  //     char   Format_Name[10];
  //     char   Format_Id[13];
  //     char   Reserved_1;
  //     int    Record_Length;
  //     int    Number_Fields;
  //     char   Record_Text_Description[50];
  //     char   Reserved_2[2];
  //     int    Record_Description_CCSID;
  //  } Qdb_Lrcd_RCDL0200_t;

Qdb_Lrcd_RCDL0200_t* getRcdFmtEntry(const char* pUserSpace, int entry)
{

  Qus_Generic_Header_0100_t* pGenHdr = NULL;
  Qdb_Lrcd_Input_t inputArea;
  Qdb_Lrcd_Header_t fileArea;
  Qdb_Lrcd_RCDL0200_t* pRcdFmt = NULL;

  int entryCount  = 0;
  int entrySize   = 0;
  int entryOffset = 0;

  coz::qusec      ec;
  pGenHdr = (Qus_Generic_Header_0100_t *)pUserSpace;
  entryCount = pGenHdr->Number_List_Entries;
  entrySize  = pGenHdr->Size_Each_Entry;
  entryOffset= pGenHdr->Offset_List_Data;
  if (entry >= entryCount) {
    return NULL;
  }

  pRcdFmt = (Qdb_Lrcd_RCDL0200_t*) (pUserSpace + (entryOffset + (entry * entrySize)));
  Qp0zLprintf("FMT: %d - Format_Id=%.13s Format_Name=%.10s Record_Length=%d Number_Fields=%d Record_Text=%.50s Record_CCSID=%d\n",
              entry,
              pRcdFmt->Format_Id,
              pRcdFmt->Format_Name,
              pRcdFmt->Record_Length,
              pRcdFmt->Number_Fields,
              pRcdFmt->Record_Text_Description,
              pRcdFmt->Record_Description_CCSID);
  return pRcdFmt;
}

int buildRcdFmtList(const char* pUserSpaceName,
                  const char* pFileName,
                  const char* pLibName)
{

  Qus_Generic_Header_0100_t* pGenHdr = NULL;
  Qdb_Lrcd_Input_t inputArea;
  Qdb_Lrcd_Header_t fileArea;

  int entryCount  = 0;
  int entrySize   = 0;
  int entryOffset = 0;

  coz::qusec      ec;

  char           overrides[2] = "0";
  char           qualFile[21];
  coz::makeAPIObjName(qualFile, pFileName, pLibName);
  QUSLRCD((char*)pUserSpaceName, "RCDL0200", (char*)qualFile,overrides, &ec);

  if (ec.isEmpty())
  {
     char* pUserSpace = coz::getPtrUsrSpace(pUserSpaceName);
     pGenHdr = (Qus_Generic_Header_0100_t *)pUserSpace;
     entryCount = pGenHdr->Number_List_Entries;
     entrySize  = pGenHdr->Size_Each_Entry;
     entryOffset= pGenHdr->Offset_List_Data;
  }
  return entryCount;
}
