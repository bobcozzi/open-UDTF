// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 1996-2026 by R. Cozzi, Jr.
// @author BobCozzi


#include <stdio.h>
#include <stdarg.h>
#include <decimal.h>
#include <ctype.h>

#ifndef __POSIX_LOCALE__
#define __POSIX_LOCALE__
#endif
#include <langinfo.h>
#include <locale.h>
#include <Qp0ztrc.h>
#include <qusec.h>

#include <COZUTILS.H>
#include <OBJLIST.H>

#include <list>
#include <map>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace coz;

#pragma datamodel(P128)

CObjList::CObjList(const char *objName,
                   const char *objLib,
                   const char *objType,
                   const char *objAttr)
{
    init();
    open(objName, objLib, objType, objAttr);
}

CObjList::CObjList()
{
    init();
}

CObjList::CObjList(const coz::listHANDLE &hList)
{
    init();
    attach(hList);
}

CObjList::~CObjList()
{
    if (!isDeferred())
    {
        close();
    }
    else
    {
        detach(); // detach handle and clear object count
    }
}

void CObjList::init()
{
    m_decMask[0] = *(nl_langinfo(CRNCYSTR) + 1);
    m_decMask[1] = *(nl_langinfo(THOUSEP));
    m_decMask[2] = *(nl_langinfo(RADIXCHAR));

    memset(m_hList, ' ', sizeof(m_hList));

    m_bDefer = 0;
    m_APIFMT = 0;
    m_objCount = 0;
    m_nextRcdNbr = 0;
    m_sortKeys.clear();
    m_objAttrFilter.clear();

    // For now we just use an * to indicate that objects with any status (damanged, not damaged, etc) are returned
    m_selectCtrl.fixed.Format_Length = sizeof(m_selectCtrl);
    m_selectCtrl.fixed.Sel_or_Omit_Status = 0; // 0=Select using statuses, 1=Omit using statuses

    m_selectCtrl.fixed.Status_Offset = 20;
    m_selectCtrl.fixed.Num_Status = 1;
    m_selectCtrl.fixed.Reserved = 0;
    m_selectCtrl.Status[0] = '*'; // 8=All, A=Authority, D=Damaged, L=Locked, or P=Partial Damaged
                                  // addDamagedLockStatus();      // No need to call since we init it above.
    setJobID();                   // Initialize the Job ID in case user asks for a different JOB's QTEMP object list

    p200 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400.Key0300.Key0200;
    p300 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400.Key0300;
    p400 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400;
    p500 = &m_objDesc.f700.Key0700.Key0600.Key0500;
    p600 = &m_objDesc.f700.Key0700.Key0600;
    p700 = &m_objDesc.f700.Key0700;
}

void CObjList::detach()
{
    memset(m_hList, ' ', sizeof(m_hList));
    m_objCount = 0;
}

int CObjList::attach(const listHANDLE &hList, const char *szAttrFilter)
{
    setDefer(true);

    m_decMask[0] = *(nl_langinfo(CRNCYSTR) + 1);
    m_decMask[1] = *(nl_langinfo(THOUSEP));
    m_decMask[2] = *(nl_langinfo(RADIXCHAR));
    m_objCount = 0;
    m_nextRcdNbr = 0; // reset QGYGTLE counter

    memset(m_hList, ' ', sizeof(m_hList));

    _CPYBYTES(m_hList, hList, sizeof(m_hList));

    coz::qusec ec;

    // We only get/use the List Info, so the return variable and length doesn't matter
    memset((char *)&m_listInfo, 0x00, sizeof(m_listInfo));

    QGYGTLE(&m_objDesc, sizeof(m_objDesc), m_hList, &m_listInfo, 0, 0, &ec);
    if (ec.hasNoError())
    {
        p200 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400.Key0300.Key0200;
        p300 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400.Key0300;
        p400 = &m_objDesc.f700.Key0700.Key0600.Key0500.Key0400;
        p500 = &m_objDesc.f700.Key0700.Key0600.Key0500;
        p600 = &m_objDesc.f700.Key0700.Key0600;
        p700 = &m_objDesc.f700.Key0700;
        m_objCount = m_listInfo.Total_Records;

        if (szAttrFilter != NULL && memicmp((char *)szAttrFilter, "*ALL", 4) != 0 && szAttrFilter[0] != ' ')
        { // Set thet attributes for filtering out objects by "Extended Object Attribute"
            m_objAttrFilter.clear();
            m_objAttrFilter.assign(szAttrFilter);
            std::transform(m_objAttrFilter.begin(), m_objAttrFilter.end(), m_objAttrFilter.begin(), ::toupper);
        }
        return m_listInfo.Total_Records;
    }
    return 0;
}

int CObjList::getHandle(listHANDLE &hList) const
{
    // Copy the content of m_hList to the result parameter
    _CPYBYTES(hList, m_hList, sizeof(m_hList));
    return m_nextRcdNbr;
}

void CObjList::setDefer(const bool bDefer)
{
    m_bDefer = bDefer;
}

bool CObjList::isDeferred() { return m_bDefer; }

// Helper function to extract the column heading that are
// hidden at the start of the CSV data row and move them
// into the CSysToken instance.

void CObjList::setAPIFmt(int APIFMT)
{
    if (APIFMT < 100 || APIFMT > 700)
    {
        m_APIFMT = 150;
    }
    else
        m_APIFMT = APIFMT;
}

int CObjList::getAPIFmt() const
{
    return m_APIFMT;
}

void CObjList::setCurRcdNbr(int currentRecordNumber)
{
    m_nextRcdNbr = currentRecordNumber;
}
void CObjList::setNextObj(int nextObjectNumber)
{
    m_nextRcdNbr = nextObjectNumber;
}
int CObjList::openList(const char *objName, const char *objLib, const char *objType, const char *objAttr)
{
    return open(objName, objLib, objType, objAttr);
}

int CObjList::openListAll(const char *objName, const char *objLib, const char *objType, const char *objAttr)
{
    return open(objName, objLib, objType, objAttr, 700);
}

int CObjList::open(const char *objName, const char *objLib, const char *objType, const char *objAttr, int APIFMT)
{
    char qualObjName[21];
    char szObject[11];
    char szLib[11];
    char objtype[11];
    const int MAX_NAMELEN = 10;
    int rtnSize = 0;
    char *pData = NULL;
    coz::qusec ec;

    Qgy_Olobj_AuthControl_t authCtrl;

    int keyFieldCount = 0;
    int keyFieldArray[32];
    int len = 0;

    // If no object name passed in, then return empty results as:  -1 return code
    if (objName == NULL || objName[0] == ' ' || objName[0] == 0x00)
        return -1;

    m_objCount = 0;
    m_nextRcdNbr = 0;
    m_objAttrFilter.clear();
    memset(m_hList, ' ', sizeof(m_hList));

    if (!isDeferred() && !coz::isAllBlanks(m_hList, sizeof(m_hList)))
    {
        close(); // If re-using an open list, then close it.
    }

    if (objType == NULL || strlen(objType) == 0 || objType[0] == ' ') // Object Type omitted?
    {
        coz::copyPad(objtype, "*ALL", MAX_NAMELEN);
    }
    else
    {
        int i = 0;
        if (objType[0] != '*')
        {
            i = 1;
        }
        coz::copyPad(objtype, objType + i, MAX_NAMELEN);
        objtype[0] = '*';
    }
    coz::copyUntil(szObject, objName, MAX_NAMELEN);
    coz::copyUntil(szLib, objLib, MAX_NAMELEN);
    coz::nameUpper(szObject);
    coz::nameUpper(szLib);
    coz::toUpper(objtype);

    if (memcmp(objtype, "*LIB", 4) == 0) // Requesting Library info?
    {
        strcpy(szLib, "QSYS"); // Then force library name to QSYS
    }

    coz::makeQual(qualObjName, szObject, szLib);

    if (qualObjName[MAX_NAMELEN] == ' ') // Is Library name omitted?
    {
        coz::copyPad(qualObjName + MAX_NAMELEN, "*CURLIB", MAX_NAMELEN); // Then use CURLIB as default
    }

    memset((char *)&m_objDesc, 0x00, sizeof(m_objDesc));

    if (!m_sortKeys.empty()) // If Sort Keys specified...
    {
        m_sortInfo.Num_Keys = m_sortKeys.size();
        for (int s = 0; s < m_sortInfo.Num_Keys; s++)
        {
            m_sortInfo.keyInfo[s] = m_sortKeys.back();
            m_sortKeys.pop_back();
        }
    }
    else
    {
        m_sortInfo.Num_Keys = 0;
    }

    authCtrl.Format_Length = sizeof(authCtrl);
    authCtrl.Call_Level = 0;
    authCtrl.Obj_Auth_Offset = 0;
    authCtrl.Num_Obj_Auth = 0;
    authCtrl.Lib_Auth_Offset = 0;
    authCtrl.Num_Lib_Auth = 0;
    authCtrl.Reserved = 0;

    // SPCVAL(*) = All/any status of the objects
    if (APIFMT != NULL && APIFMT != 0 && m_APIFMT == 0)
    {
        m_APIFMT = APIFMT;
    }
    if (m_APIFMT == 0)
    {
        m_APIFMT = 150;
    }

    int k = 0;

    pData = (char *)&m_objDesc;
    if (m_APIFMT >= 0 && m_APIFMT <= 100) // *ALLSIMPLE means just lib/obj/type only
    {
        keyFieldArray[0] = 0;
        k = 0; // No data, only object, lib, and type are returned in the ID substruct (fastest)
    }
    else if (m_APIFMT >= 200) // If *GE 200 then use the built-in formats from the API itself
    {                         // e.g., 200, 300, 400, etc.
        rtnSize = sizeof(object_Info700_t);
        keyFieldArray[k++] = m_APIFMT;
    }
    else if (m_APIFMT == 150) // Default format is our customized format (aka "150")
    {
        rtnSize = sizeof(object_Info150_t);
        keyFieldArray[k++] = 201; // Stupid "Status" also in the .id. substruct
        keyFieldArray[k++] = 202; // Attribute ("Extended Object attribute")
        keyFieldArray[k++] = 203; // Text
        keyFieldArray[k++] = 302; // Owner USRPRF
        keyFieldArray[k++] = 405; // Creator USRPRF
        keyFieldArray[k++] = 406; // Created on System Name
        keyFieldArray[k++] = 303; // Domain
        keyFieldArray[k++] = 304; // CRT Date/Time
        keyFieldArray[k++] = 305; // CHG Date/Time
        keyFieldArray[k++] = 601; // Last Used Date
        keyFieldArray[k++] = 602; // Last Used Date Reset Date
        keyFieldArray[k++] = 603; // Days used since last used reseet date
        keyFieldArray[k++] = 411; // Licesed Program for Object (e.g. 5770SS1) CHAR(16)
                                  //  1-7 Product ID; 8-18 VxxRxxMxx
        keyFieldArray[k++] = 701; // Object size
        keyFieldArray[k++] = 702; // Object size multiplier
    } // else it uses the 3-digit key(s) option, e.g., 600 means use format 600 (see above)

    keyFieldCount = k;

    int rcdCount = -1; // Get first 10 records synchronously, the rest asynchronously

    QGYOLOBJ(pData, rtnSize,      // return buffer length
             (void *)&m_listInfo, // returned information on the list
             (int)rcdCount,       // number of records to return on a get next
             (void *)&m_sortInfo, // pre-sort options)

             (char *)qualObjName, // Qualified object name
             (char *)objtype,     // Object Type

             (void *)&authCtrl,      // Authority selection control block
             (void *)&m_selectCtrl,  // Damaged object selection control block
             (int)keyFieldCount,     // Number of key fields provided to the API to be returned
             (void *)&keyFieldArray, // Array of key fields to be returned.
             (void *)&ec,            // Exception/Error return class
             (void *)&m_jobInfo,
             (void *)"JIDF0100");

    if (ec.isError())
    {
        coz::resignalMsg(ec);
    }
    else
    {
        // Copy Open List Handle to its member variable for later use
        memcpy(m_hList, m_listInfo.Request_Handle, sizeof(m_listInfo.Request_Handle));
        m_objCount = m_listInfo.Total_Records;

        if (objAttr != NULL && memicmp((char *)objAttr, "*ALL", 4) != 0 && objAttr[0] != ' ')
        { // Save object attributes for filters during get Next Obj() method
            m_objAttrFilter.clear();
            m_objAttrFilter.assign(objAttr);
            std::transform(m_objAttrFilter.begin(), m_objAttrFilter.end(), m_objAttrFilter.begin(), ::toupper);
        }
    }
    return m_objCount;
}

void CObjList::close()
{
    if (coz::isNotEmpty(m_hList, sizeof(m_hList)))
    {
        coz::qusec ec;
        QGYCLST(m_hList, &ec);

        memset(m_hList, ' ', sizeof(m_hList));
        m_objCount = 0;
        m_nextRcdNbr = 0;
        m_objAttrFilter.clear();
    }
    return;
}

int CObjList::getCurrentRecordNumber() const
{
    return m_nextRcdNbr;
}

int CObjList::getCurRcdNbr() const
{
    return m_nextRcdNbr;
}

int CObjList::listagg(char *pObjects, int items /* 1=Obj, 2=Obj & Lib, 3=Obj, Lib & Type */)
{
    char *pObjDesc = NULL;
    int count = 0;
    int len = 0;
    char szObj[11];
    char szLib[11];
    char szType[11];

    len = std::min<int>(std::max<int>(1, items), 3) * 10;
    m_nextRcdNbr = 0;
    for (int i = 0; i < m_objCount; i++)
    {
        memset(szObj, 0x00, sizeof(szObj));
        memset(szLib, 0x00, sizeof(szLib));
        memset(szType, 0x00, sizeof(szType));
        getNextObj(szObj, szLib, szType);
        if (items > 0)
            _CPYBYTES(pObjects + (i * len), szObj, strlen(szObj));
        if (items > 1)
            _CPYBYTES(pObjects + (i * len) + 10, szLib, strlen(szLib));
        if (items > 2)
            _CPYBYTES(pObjects + (i * len) + 20, szType, strlen(szType));
        count++;
    }
    return count;
}
// returns a count of the objects retrieved
// and a pointer to the last retrieved object.
// NOTE: Does not memcpy, but returnes a pointer to internal member variable
// Be sure to cast the returned value to the format that was used.
// Or use our object_Desc_t format and the f150 or f700 member for the data.
// rowToGet == 0 to start getting from list and return first entry
// rowToGet < 0 to read current row "again" without incrementing rowCounter
char *CObjList::getNextObj(int &rowToGet)
{
    int recordsToReturn = 1; // One at a time processing
    int rtnSize = 0;
    int rcdAdj = 0;
    int keyFieldLength = 0;
    Qgy_Olobj_RecVar_t recvar;

    char *pData = NULL;
    char *pKeyFieldData = NULL;

    coz::qusec ec;

    if (rowToGet < 0) // re-Get Current object record?
    {
        rcdAdj = -1; // Need to decrement to get *current record
    }

    if (rowToGet >= 0)
        m_nextRcdNbr = rowToGet;

    if (m_nextRcdNbr <= 0)
        m_nextRcdNbr = 1;
    if (m_nextRcdNbr > m_objCount)
        return NULL;

    pData = (char *)&m_objDesc;
    rtnSize = sizeof(m_objDesc);
    while (1)
    {
        if (m_nextRcdNbr > m_objCount)
            return NULL;

        memset((char *)&m_listInfo, 0x00, sizeof(m_listInfo));
        QGYGTLE(pData, rtnSize, m_hList, &m_listInfo, recordsToReturn, m_nextRcdNbr + rcdAdj, &ec);
        if (ec.isError())
        {
            coz::resignalMsg(ec);
        }
        if (m_listInfo.Records_Returned == 0)
        {
            return NULL;
        }

        // If rowToGet < 0 then do not increment rowCounter
        if (rowToGet >= 0)
        {
            m_nextRcdNbr += m_listInfo.Records_Returned;
        }

        // If object attribute filter is not empty
        if (!m_objAttrFilter.empty())
        {
            pKeyFieldData = getDataByKey(202, &keyFieldLength); // Get Object Attribute Key(202)
            if (pKeyFieldData != NULL)
            {
                pKeyFieldData[keyFieldLength] = 0x00;
                pKeyFieldData[::triml(pKeyFieldData, ' ')] = 0x00;
                if (strlen(pKeyFieldData) > 0 && pKeyFieldData[0] != ' ')
                {
                    if (m_objAttrFilter.find(pKeyFieldData) == std::string::npos)
                    {
                        if (rowToGet >= 0)
                        {
                            continue; // Iterate to next object/skip/filter this object
                        }
                    }
                }
                else
                {
                    if (rowToGet >= 0)
                    {
                        continue; // Iterate to next object/skip/filter this object
                    }
                }
            }
        }
        break;
    };
    rowToGet = m_nextRcdNbr; // returns next row to get to caller.
    return pData;
}

// Get object "extended" attribute (Key[202])
char *CObjList::getObjAttr(char *pAttrBuffer)
{
    char objAttr[11];
    int keyFieldLength = 0;
    char *pKeyFieldData = NULL;
    memset(objAttr, 0x00, sizeof(objAttr));
    pKeyFieldData = getDataByKey(202, &keyFieldLength); // Get Object Attribute Key(202)
    if (pKeyFieldData != NULL && keyFieldLength > 0)
    {
        pKeyFieldData[keyFieldLength] = 0x00;              // Add null
        pKeyFieldData[::triml(pKeyFieldData, ' ')] = 0x00; // right-Trim blanks
        if (pAttrBuffer != NULL)
        {
            strcpy(pAttrBuffer, pKeyFieldData);
        }
        strcpy(coz::buffer4k, pKeyFieldData);
    }
    return coz::buffer4k;
}
// returns a count of the objects retrieved
// and a pointer to the last retrieved object.
// NOTE: Does not memcpy, but returnes a pointer to internal member variable
// Be sure to cast the returned value to the format that was used.
// Or use our object_Desc_t format and the f150 or f700 member for the data.
char *CObjList::getNextObj()
{
    int recordsToReturn = 1; // One at a time processing
    int rtnSize = 0;
    int keyFieldLength = 0;
    char *pData = NULL;
    Qgy_Olobj_RecVar_t recvar;
    char *pKeyFieldData = NULL;
    coz::qusec ec;
    ec.init();

    if (m_nextRcdNbr <= 0)
        m_nextRcdNbr = 1;
    if (m_nextRcdNbr > m_objCount)
        return NULL;

    pData = (char *)&m_objDesc;
    rtnSize = sizeof(m_objDesc);
    while (1)
    {
        if (m_nextRcdNbr > m_objCount)
            return NULL;

        memset((char *)&m_listInfo, 0x00, sizeof(m_listInfo));
        QGYGTLE(pData, rtnSize, m_hList, &m_listInfo, recordsToReturn, m_nextRcdNbr, &ec);

        if (m_listInfo.Records_Returned == 0)
        {
            return NULL;
        }
        m_nextRcdNbr += m_listInfo.Records_Returned;
        if (!m_objAttrFilter.empty())
        {

            pKeyFieldData = getDataByKey(202, &keyFieldLength); // Get Object Attribute Key(202)
            if (pKeyFieldData != NULL)
            {
                pKeyFieldData[keyFieldLength] = 0x00;
                pKeyFieldData[::triml(pKeyFieldData, ' ')] = 0x00;
                if (strlen(pKeyFieldData) > 0 && pKeyFieldData[0] != ' ')
                {
                    if (m_objAttrFilter.find(pKeyFieldData) == std::string::npos)
                    {
                        continue; // Iterate to next object/skip/filter this object
                    }
                }
                else
                {
                    continue; // If Obj has no extAttr and attribute filter is specified, skip it
                }
            }
        }
        break;
    };
    return pData;
}

// returns a pointer to the resultset data (200_t struct)
void *CObjList::getNextObj(char *objname, char *objlib, char *objtype)
{
    int recordsToReturn = 1; // One at a time processing
    int rtnSize = 0;
    int keyFieldLength = 0;
    char *pData = NULL;
    Qus_EC_t ec;

    Qgy_Olobj_RecVar_t recvar;
    char *pKeyFieldData = NULL;
    char objAttrFilter[11];

    memset((char *)&ec, 0x00, sizeof(ec));
    ec.Bytes_Provided = sizeof(ec);

    pData = (char *)&m_objDesc;
    rtnSize = sizeof(m_objDesc);

    if (m_nextRcdNbr <= 0)
        m_nextRcdNbr = 1;

    while (1)
    {
        if (m_nextRcdNbr > m_objCount)
            return NULL;

        memset((char *)&m_listInfo, 0x00, sizeof(m_listInfo));
        QGYGTLE(pData, rtnSize, m_hList, &m_listInfo, recordsToReturn, m_nextRcdNbr, &ec);

        if (m_listInfo.Records_Returned == 0)
        {
            return NULL;
        }
        m_nextRcdNbr += m_listInfo.Records_Returned;
        if (!m_objAttrFilter.empty())
        {
            memset(objAttrFilter, 0x00, sizeof(objAttrFilter));
            pKeyFieldData = getDataByKey(202, &keyFieldLength); // Get Object Attribute Key(202)
            if (pKeyFieldData != NULL)
            {
                pKeyFieldData[keyFieldLength] = 0x00;
                pKeyFieldData[::triml(pKeyFieldData, ' ')] = 0x00;
                if (strlen(objAttrFilter) > 0 && objAttrFilter[0] != ' ')
                {
                    if (m_objAttrFilter.find(pKeyFieldData) == std::string::npos)
                    {
                        continue; // Iterate to next object/skip/filter this object
                    }
                }
            }
        }
        break;
    };
    Qgy_Olobj_RecVar_t *pReceiver = (Qgy_Olobj_RecVar_t *)pData;

    if (objname != NULL)
    {
        memcpy(objname, pReceiver->Obj_Name, sizeof(recvar.Obj_Name));
    }
    if (objlib != NULL)
    {
        memcpy(objlib, pReceiver->Obj_Lib, sizeof(recvar.Obj_Lib));
    }
    if (objtype != NULL)
    {
        memcpy(objtype, pReceiver->Obj_Type, sizeof(recvar.Obj_Type));
    }
    return pData;
}

int CObjList::getCount() const
{
    return m_objCount;
}

int CObjList::getRecords() const
{
    return m_objCount;
}

void CObjList::joblog(const char *msgText, ...)
{

    va_list arg_ptr;

    va_start(arg_ptr, msgText);
    Qp0zLprintf((char *)msgText, arg_ptr);
    va_end(arg_ptr);

    if (msgText[strlen(msgText) - 1] != '\n')
    {
        Qp0zLprintf("\n");
    }
}

void CObjList::setJobID(const char *jobName, const char *jobid /* = NULL */)
{
    char job[32];
    memset((char *)&m_jobInfo.Job_Name, ' ', sizeof(m_jobInfo.Job_Name));
    memset((char *)&m_jobInfo.Job_Number, ' ', sizeof(m_jobInfo.Job_Number));
    memset((char *)&m_jobInfo.User_Name, ' ', sizeof(m_jobInfo.User_Name));
    memset(m_jobInfo.Int_Job_ID, ' ', sizeof(m_jobInfo.Int_Job_ID)); // Must be blanks when Job_Name != *INT

    memset(m_jobInfo.Thread_Id, 0x00, sizeof(m_jobInfo.Thread_Id));
    memset(m_jobInfo.Reserved, 0x00, sizeof(m_jobInfo.Reserved));
    m_jobInfo.Thread_Indicator = 2;

    if ((jobName == NULL || strlen(jobName) == 0) && (jobid == NULL || strlen(jobid) == 0))
    {
        m_jobInfo.Job_Name[0] = '*';
    }
    else
    {
        strcpy(job, jobName);
        coz::toUpper(job);
        if (strcmp(job, "INT") == 0 || strcmp(job, "*INT") == 0)
        { // Use Internal job id?
            memcpy(m_jobInfo.Int_Job_ID, jobid, sizeof(m_jobInfo.Int_Job_ID));
            memcpy(m_jobInfo.Job_Name, "*INT", 4);
        }
        else
        {
            memset(m_jobInfo.Int_Job_ID, ' ', sizeof(m_jobInfo.Int_Job_ID)); // Must be blanks when Job_Name != *INT
            coz::makeAPIJobName((char *)&m_jobInfo, job);
        }
    }
}

int CObjList::getKeyCount()
{
    return m_objDesc.id.Num_Fields_Retd;
}

// Get key by key FULL is for formats 200 to 700
// Key data is returned as char* to the actual data itself
// integer values are converted to and returned as text.
char *CObjList::getDataByKey_FULL(int keyID, int *rtnLen)
{
    Qgy_Olobj_KeyData_t *pKeyData = NULL;
    Qgy_Olobj_RecVar_t *pObjHeader = NULL;
    object_Desc_t *pObject = NULL;
    char *pData = NULL;
    static char szData[32];
    int keyPos = 0;
    int keyOffset = 0;
    int keyCount = 0;

    pObject = &m_objDesc;
    keyCount = pObject->id.Num_Fields_Retd;
    if (m_APIFMT >= 200)
    {
        if (keyID < 299)
        {
            switch (keyID)
            {
            case 201:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f200.Info_Status);
                return &p200->Info_Status;
            case 202:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f200.Ext_Obj_Attr);
                return p200->Ext_Obj_Attr;
            case 203:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f200.Text_Description);
                return p200->Text_Description;
            case 204:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f200.User_Def_Attr);
                return p200->User_Def_Attr;
            case 205:
                memset(szData, 0x00, sizeof(szData));
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f200.Order_In_Library_List);
                sprintf(szData, "%d", p200->Order_In_Library_List);
                return szData;
            }
        }
        else if (keyID < 399)
        {
            switch (keyID)
            {
            case 301:
                memset(szData, 0x00, sizeof(szData));
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Aux_Storage_Pool);
                sprintf(szData, "%d", p300->Aux_Storage_Pool);
                return szData;
            case 302:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Obj_Owner);
                return p300->Obj_Owner;
            case 303:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Obj_Domain);
                return p300->Obj_Domain;
            case 304:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Creation_Date_Time);
                return p300->Creation_Date_Time;
            case 305:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Change_Date_Time);
                return p300->Change_Date_Time;
            case 306:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Storage);
                return p300->Storage;
            case 307:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Comp_Status);
                return &p300->Comp_Status;
            case 308:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Allow_Change);
                return &p300->Allow_Change;
            case 309:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Changed);
                return &p300->Changed;
            case 310:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Audit_Value);
                return p300->Audit_Value;
            case 311:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Digitally_Signed);
                return &p300->Digitally_Signed;
            case 312:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Signer_Trusted);
                return &p300->Signer_Trusted;
            case 313:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Multiple_Signatures);
                return &p300->Multiple_Signatures;
            case 314:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f300.Lib_ASP_Number);
                sprintf(szData, "%d", p300->Lib_ASP_Number);
                return szData;
            }
        }
        else if (keyID < 499)
        {
            switch (keyID)
            {
            case 401:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.SF_Name);
                return p400->SF_Name;
            case 402:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.SF_Name);
                return p400->SF_Name;
            case 403:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.SF_Mem_Name);
                return p400->SF_Mem_Name;
            case 404: // 13-part date CYYMMDDHHMMSS
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.SF_Update_Date_Time);
                return p400->SF_Update_Date_Time;
            case 405:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Creator_Usr_Prf);
                return p400->Creator_Usr_Prf;
            case 406:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Creation_Sys);
                return p400->Creation_Sys;
            case 407:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Sys_Level);
                return p400->Sys_Level;
            case 408:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Compiler);
                return p400->Compiler;
            case 409:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Obj_Level);
                return p400->Obj_Level;
            case 410:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Usr_Changed);
                return &p400->Usr_Changed;
            case 411:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Lic_Pgm);
                return p400->Lic_Pgm;
            case 412:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.PTF);
                return p400->PTF;
            case 413:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.APAR);
                return p400->APAR;
            case 414:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Primary_Group);
                return p400->Primary_Group;
            case 415:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Optimum_Space_Alignment);
                return &p400->Optimum_Space_Alignment;
            case 416:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f400.Primary_Associated_Space_Size);
                sprintf(szData, "%d", p400->Primary_Associated_Space_Size);
                return szData;
            }
        }
        else if (keyID < 599)
        {
            switch (keyID)
            {
            case 501:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Obj_S_Date_Time);
                return p500->Obj_S_Date_Time;
            case 502:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Obj_R_Date_Time);
                return p500->Obj_R_Date_Time;
            case 503:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Size);
                sprintf(szData, "%d", p500->S_Size);
                return szData;
            case 504: // 13-part date CYYMMDDHHMMSS
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Size_Mult);
                sprintf(szData, "%d", p500->S_Size_Mult);
                return szData;
            case 505:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Seq_No);
                sprintf(szData, "%d", p500->S_Seq_No);
                return szData;
            case 506:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Command);
                return p500->S_Command;
            case 507:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Vol_ID);
                return p500->S_Vol_ID;
            case 508:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Dev);
                return p500->S_Dev;
            case 509:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_File_Name);
                return p500->S_File_Name;
            case 510:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_File_Lib_Name);
                return p500->S_File_Lib_Name;
            case 511:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Label);
                return p500->S_Label;
            case 512:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.S_Active_Date_Time);
                return p500->S_Active_Date_Time;
            case 513:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Status);
                return &p500->Journal_Status;
            case 514:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Name);
                return p500->Journal_Name;
            case 515:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Library);
                return p500->Journal_Library;
            case 516:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Images);
                return &p500->Journal_Images;
            case 517:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Entries_Omitted);
                return &p500->Journal_Entries_Omitted;
            case 518:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Journal_Start_Date_Time);
                return p500->Journal_Start_Date_Time;
            case 519:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f500.Remote_Journal_Filter);
                return &p500->Remote_Journal_Filter;
            }
        }
        else if (keyID < 699)
        {
            switch (keyID)
            {
            case 601:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Last_Date_Time);
                return p600->Last_Date_Time;
            case 602:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Reset_Date_Time);
                return p600->Reset_Date_Time;
            case 603:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Days_Used_Count);
                sprintf(szData, "%d", p600->Days_Used_Count);
                return szData;
            case 604:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Usage_Info_Updated);
                return &p600->Usage_Info_Updated;
            case 605:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Obj_ASP_Device);
                return p600->Obj_ASP_Device;
            case 606:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f600.Lib_ASP_Device);
                return p600->Lib_ASP_Device;
            }
        }
        else if (keyID < 799)
        {
            switch (keyID)
            {
            case 701:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Obj_Size);
                sprintf(szData, "%d", p700->Obj_Size);
                return szData;
            case 702:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Obj_Size_Mult);
                sprintf(szData, "%d", p700->Obj_Size_Mult);
                return szData;
            case 703:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Obj_Ovf_ASP);
                return &p700->Obj_Ovf_ASP;
            case 704:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Object_ASP_Group);
                return p700->Object_ASP_Group;
            case 705:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Library_ASP_Group);
                return p700->Library_ASP_Group;
            case 706:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Starting_Jrn_Rcv_For_Apply);
                return p700->Starting_Jrn_Rcv_For_Apply;
            case 707:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Starting_Jrn_Rcv_Lib);
                return p700->Starting_Jrn_Rcv_Lib;
            case 708:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Starting_Jrn_Rcv_Lib_ASP_Dev);
                return p700->Starting_Jrn_Rcv_Lib_ASP_Dev;
            case 709:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Starting_Jrn_Rcv_Lib_ASP_Group);
                return p700->Starting_Jrn_Rcv_Lib_ASP_Group;
            case 710:
                if (rtnLen != NULL)
                    *rtnLen = sizeof(f700.Build_ID);
                return p700->Build_ID;
            }
        }
    }
    return NULL;
}

// Note: getDataByKey and getDataByPos are for Format 150 calls.
// with our custom format 150. The other IBM-supplied formats
// do not conform to the KeyData template structure but are
// instead, setup like a traditional data-structure.
char *CObjList::getDataByKey(int keyID, int *rtnLen)
{
    Qgy_Olobj_KeyData_t *pKeyData = NULL;
    Qgy_Olobj_RecVar_t *pObjHeader = NULL;
    object_Desc_t *pObject = &m_objDesc;
    char *pData = NULL;
    int keyPos = 0;
    int keyOffset = 0;
    int keyCount = 0;

    pObject = &m_objDesc;
    keyCount = pObject->id.Num_Fields_Retd;

    if (m_APIFMT == 150)
    {
        for (keyPos = 0; keyPos < keyCount; keyPos++)
        {
            pKeyData = (Qgy_Olobj_KeyData_t *)getDataByPos(keyPos);
            if (pKeyData != NULL)
            {
                if (pKeyData->Key_Field == keyID)
                {
                    if (rtnLen != NULL)
                    {
                        *rtnLen = pKeyData->Data_Length;
                    }
                    return ((char *)pKeyData) + sizeof(Qgy_Olobj_KeyData_t);
                }
            }
        }
    }
    else
    {
        pData = getDataByKey_FULL(keyID, rtnLen);
    }
    return pData;
}

Qgy_Olobj_KeyData_t *CObjList::getDataByPos(int keyPos)
{
    Qgy_Olobj_KeyData_t *pKeyData = NULL;
    Qgy_Olobj_RecVar_t *pObjInfo = NULL;
    object_Desc_t *pObject = &m_objDesc;
    char *pKeyInfo = NULL;
    int keyOffset = 0;

    pObjInfo = &pObject->id;
    if (pObjInfo->Num_Fields_Retd >= keyPos)
    {
        keyOffset = sizeof(Qgy_Olobj_RecVar_t);
        pKeyData = (Qgy_Olobj_KeyData_t *)&pObject->f150;
        pKeyInfo = (char *)pKeyData;
        for (int i = 0; i < keyPos; i++)
        {
            pKeyInfo += pKeyData->Field_Info_Length;
            pKeyData = (Qgy_Olobj_KeyData_t *)pKeyInfo;
        }
    }
    return pKeyData;
}

// int CObjList::addSortFieldByKey(int key, char seq /* seq='1' (ASC) */)
//{
//     int pos = 0;
//     int len = 0;
//     short dataType = CHAR_type;
//
//     if (key==201)
//     {
//        pos = offsetof(object_Info150_t, Info_Status);
//        len = sizeof(m_objDesc.f150.Info_Status);
////       dataType = CHAR_type;
//    }
//    if (key==202)
//    {
//       pos = offsetof(object_Info150_t, Object_Attr);
//       len = sizeof(m_objDesc.f150.Object_Attr);
//       dataType = CHAR_type;
//    }
//    if (key==203)
//    {
//       pos = offsetof(object_Info150_t, Text_Desc);
//       len = sizeof(m_objDesc.f150.Text_Desc);
//       dataType = CHAR_type;
//    }
//    if (key==302)
//    {
//       pos = offsetof(object_Info150_t, Object_Owner);
//       len = sizeof(m_objDesc.f150.Object_Owner);
//       dataType = CHAR_type;
//    }
//    if (key==405)
//    {
//       pos = offsetof(object_Info150_t, Object_Creator);
//       len = sizeof(m_objDesc.f150.Object_Creator);
//       dataType = CHAR_type;
//    }
//    if (key==406)
//    {
//       pos = offsetof(object_Info150_t, Created_on_Sys);
//       len = sizeof(m_objDesc.f150.Created_on_Sys);
//       dataType = CHAR_type;
//    }
//    if (key==303)
//    {
//       pos = offsetof(object_Info150_t, Object_Domain);
//       len = sizeof(m_objDesc.f150.Object_Domain);
//       dataType = CHAR_type;
//    }
//    if (key==304)
//    {
//       pos = offsetof(object_Info150_t, Creation_Date_Time);
//       len = sizeof(m_objDesc.f150.Creation_Date_Time);
//       dataType = UINT_type;
//    }
//    if (key==305)
//    {
//       pos = offsetof(object_Info150_t, Changed_Date_Time);
//       len = sizeof(m_objDesc.f150.Changed_Date_Time);
//       dataType = UINT_type;
//    }
//    if (key==601)
//    {
//       pos = offsetof(object_Info150_t, Last_Used_Date_Time);
//       len = sizeof(m_objDesc.f150.Last_Used_Date_Time);
//       dataType = UINT_type;
//    }
//    if (key==602)
//    {
//       pos = offsetof(object_Info150_t, Reset_Date_Time);
//       len = sizeof(m_objDesc.f150.Reset_Date_Time);
//       dataType = UINT_type;
//    }
//    if (key==603)
//    {
//       pos = offsetof(object_Info150_t, Days_Used_Count);
//       len = sizeof(m_objDesc.f150.Days_Used_Count);
//       dataType = INT_type;
//    }
//    if (key==411)
//    {
//       pos = offsetof(object_Info150_t, LICPGM);
//       len = sizeof(m_objDesc.f150.LICPGM) + sizeof(m_objDesc.f150.LICPGMRLS);
//       dataType = CHAR_type;
//    }
//    pos += sizeof(Qgy_Olobj_RecVar_t);  // adjust for the "ID" member variable
//    coz::joblog("AddSortKey: Key(%d) POS(%d) LEN(%d)", key, pos+4, len);
//    return addSortField(pos+4, len, dataType, seq);
//}

int CObjList::addDamagedLockStatus(const char *pStatus)
{
    bool bDamaged = false;
    bool bPartial = false;
    bool bLocked = false;

    for (int i = 0; i < strlen(pStatus); i++)
    {
        switch (pStatus[i])
        {
        case 'd':
        case 'D':
            if (!bDamaged)
            {
                m_selectCtrl.Status[m_selectCtrl.fixed.Num_Status] = 'D'; // 'D' = Damaged Objects
                m_selectCtrl.fixed.Num_Status++;
                bDamaged = true;
            }
            break;
        case 'p':
        case 'P':
            if (!bPartial)
            {
                m_selectCtrl.Status[m_selectCtrl.fixed.Num_Status] = 'P'; // 'P' = Partially Damaged Objects
                m_selectCtrl.fixed.Num_Status++;
                bPartial = true;
            }
            break;
        case 'l':
        case 'L':
            if (!bLocked)
            {
                m_selectCtrl.Status[m_selectCtrl.fixed.Num_Status] = 'L'; // 'L' = Object Locked
                m_selectCtrl.fixed.Num_Status++;
                bLocked = true;
            }
            break;
        default:
            break;
        }
    }
    if (m_selectCtrl.fixed.Num_Status < 1 ||
        m_selectCtrl.fixed.Num_Status > 3)
    {
        m_selectCtrl.fixed.Num_Status = 1; // Switch to *ALL if more than 3 or fewer than 1
        m_selectCtrl.Status[0] = '*';      // All object statuses (default)
    }
    return m_selectCtrl.fixed.Num_Status;
}

int CObjList::addSortFieldByKey(int key, char seq /* seq='1' (ASC) */)
{
    int pos = 0;
    int len = 0;
    short dataType = CHAR_type;

    switch (key)
    {
    case 201:
        pos = offsetof(object_Info150_t, Info_Status);
        len = sizeof(m_objDesc.f150.Info_Status);
        break;

    case 202:
        pos = offsetof(object_Info150_t, Object_Attr);
        len = sizeof(m_objDesc.f150.Object_Attr);
        break;

    case 203:
        pos = offsetof(object_Info150_t, Text_Desc);
        len = sizeof(m_objDesc.f150.Text_Desc);
        break;

    case 302:
        pos = offsetof(object_Info150_t, Object_Owner);
        len = sizeof(m_objDesc.f150.Object_Owner);
        break;

    case 405:
        pos = offsetof(object_Info150_t, Object_Creator);
        len = sizeof(m_objDesc.f150.Object_Creator);
        break;

    case 406:
        pos = offsetof(object_Info150_t, Created_on_Sys);
        len = sizeof(m_objDesc.f150.Created_on_Sys);
        break;

    case 303:
        pos = offsetof(object_Info150_t, Object_Domain);
        len = sizeof(m_objDesc.f150.Object_Domain);
        break;

    case 304:
        pos = offsetof(object_Info150_t, Creation_Date_Time);
        len = sizeof(m_objDesc.f150.Creation_Date_Time);
        dataType = UINT_type;
        break;

    case 305:
        pos = offsetof(object_Info150_t, Changed_Date_Time);
        len = sizeof(m_objDesc.f150.Changed_Date_Time);
        dataType = UINT_type;
        break;

    case 601:
        pos = offsetof(object_Info150_t, Last_Used_Date_Time);
        len = sizeof(m_objDesc.f150.Last_Used_Date_Time);
        dataType = UINT_type;
        break;

    case 602:
        pos = offsetof(object_Info150_t, Reset_Date_Time);
        len = sizeof(m_objDesc.f150.Reset_Date_Time);
        dataType = UINT_type;
        break;

    case 603:
        pos = offsetof(object_Info150_t, Days_Used_Count);
        len = sizeof(m_objDesc.f150.Days_Used_Count);
        dataType = INT_type;
        break;

    case 411:
        pos = offsetof(object_Info150_t, LICPGM);
        len = sizeof(m_objDesc.f150.LICPGM) + sizeof(m_objDesc.f150.LICPGMRLS);
        break;

    default:
        // Handle default case or raise an error if needed
        break;
    }

    pos += sizeof(Qgy_Olobj_RecVar_t); // adjust for the "ID" member variable
    coz::joblog("AddSortKey: Key(%d) POS(%d) LEN(%d)", key, pos + 4, len);
    return addSortField(pos + 4, len, dataType, seq);
}

// For Deferred Object Lists Only. Create/Add a Sort Field (max 40)
int CObjList::addSortField(int pos, int len, short dataType, char seq)
{
    Qgy_Olobj_SortKeyInfo_t sortKey;
    sortKey.Reserved = 0x00;

    sortKey.Field_Start_Pos = pos;
    sortKey.Field_Length = len;

    if (dataType == CHAR_type && (seq == '0' || seq == '1' || seq == 'A'))
    {
        sortKey.Field_Data_Type = 0; // Field_Data_Type is "short int"
        sortKey.Sort_Order = 0x00;   // Sort_Order is "char"
    }
    else
    {
        // This method accepts 1 or 'A' for ASC, and D, 0 or '2' for DESC
        sortKey.Sort_Order = (seq == '0' || seq == '1' || seq == 'A') ? '1' : '2'; // 1=ASC 2=DESC
        sortKey.Field_Data_Type = dataType;                                        // See QLGSORT for dataType codes
    }
    m_sortKeys.push_back(sortKey);
    return m_sortKeys.size();
}

#pragma datamodel(pop)
