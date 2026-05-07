
#ifndef _ST_OBJLIST_CLASS_
#define _ST_OBJLIST_CLASS_

           /***********************************************************/
           /* (c) Copyright Cozzi Research 2016                       */
           /* All rights reserved.                                    */
           /* Licensed Material--Property of Cozzi Research           */
           /***********************************************************/
#pragma datamodel(P128)

#include <string>
#include <vector>
#include <decimal.h>

#include <QwcAttr.h>
#include <QGy.h>
#include <QGy.h>
#include <qgyolobj.h>
#include <QGYGTLE.h>
#include <QGYCLST.h>

#include <langinfo.h>
#include <QP0LSTDI.h>
#include <QP0ZTRC.h>

#include <COZUTILS.H>           // standalone coz:: namespace

typedef _Packed struct tagAllObjListSel {
  Qgy_Olobj_SelControl_t fixed;
  char Status[4];   //  D=Damaged objects, L=Locked Objects, P=Partially damaged, *=ALL object statuses
} allObjSelect_t;

typedef _Packed struct tagObjectInfo150
{
    Qgy_Olobj_KeyData_t key201;
    char  Info_Status;              /* Key 201 (_ A D L P)           */
    Qgy_Olobj_KeyData_t key202;
    char  Object_Attr[10];          /* Key 202                       */
    Qgy_Olobj_KeyData_t key203;
    char  Text_Desc[50];            /* Key 203                       */
    Qgy_Olobj_KeyData_t key302;
    char  Object_Owner[10];         /* Key 302                       */
    Qgy_Olobj_KeyData_t key405;
    char  Object_Creator[10];       /* Key 405      */
    Qgy_Olobj_KeyData_t key406;
    char  Created_on_Sys[8];        /* Key 406      */
    Qgy_Olobj_KeyData_t key303;
    char  Object_Domain[2];         /* Key 303                       */
    Qgy_Olobj_KeyData_t key304;
    char  Creation_Date_Time[8];    /* Key 304                       */
    Qgy_Olobj_KeyData_t key305;
    char  Changed_Date_Time[8];     /* Key 305                       */
    Qgy_Olobj_KeyData_t key601;
    char  Last_Used_Date_Time[8];   /* Key 601                       */
    Qgy_Olobj_KeyData_t key602;
    char  Reset_Date_Time[8];       /* Key 602                       */
    Qgy_Olobj_KeyData_t key603;
    int   Days_Used_Count;          /* Key 603                        */
    Qgy_Olobj_KeyData_t key411;
    char  LICPGM[7];                /* Key 411 e.g., 5770SS1          */
    char  LICPGMRLS[9];             /* Key 411 Pt 2: VxxRyyMzz        */
    Qgy_Olobj_KeyData_t key701;
    int   Object_Size;              /* Key 701 Object Size            */
    Qgy_Olobj_KeyData_t key702;
    int   Object_Size_Multiplier;   /* Key 702 Object Size Multiplier */
} object_Info150_t;

typedef _Packed struct tagObjectInfo700
{
       Qgy_Olobj_KeyData_t keyData;
       Qgy_Olobj_Key0700_t Key0700; // Object Description
} object_Info700_t;

typedef _Packed struct tagObjectDesc
{
     Qgy_Olobj_RecVar_t  id; // Object Identification
     union {
       object_Info150_t     f150;
       object_Info700_t     f700;
     };
} object_Desc_t;

typedef _Packed struct Qgy_Olobj_SortInfo2
{
   int                      Num_Keys;
   Qgy_Olobj_SortKeyInfo_t  keyInfo[40];
} Qgy_Olobj_SortInfo_t2;

using namespace std;

class CObjList
{
  public:
      CObjList();
      CObjList(const char* objName,
               const char* objLib="*CURLIB",
               const char* objType="*ALL",
               const char* objAttr="*ALL");
      CObjList(const coz::listHANDLE& hList);
     ~CObjList();

enum sort_datatype { INT_type   = 0,
                           ZONED_type = 2,
                           DEC_type   = 3,
                           PKD_type   = 3,
                           PACKED_type= 3,
                           DBCSCHAR_type  = 4,
                           SBCSCHAR_type  = 5,
                           CHAR_type      = 6,
                           UINT_type      = 9,
                           MDY_type  = 13,    // MM/DD/YY
                           DMY_type  = 14,    // DD/MM/YY
                           DMYDOT_type  = 15, // DD.MM.YY
                           MDYY_type  = 16,    // MM/DD/YYYY  (4-digit year)
                           DATE_type  = 17,    // other date formats
                           HMS_type   = 18,    // HH:MM xM
                           VARCHAR_type = 21 };

private:
      coz::listHANDLE           m_hList;

      char                 m_decMask[3];
      int                  m_objCount;
      int                  m_nextRcdNbr;
      int                  m_APIFMT;
      bool                 m_bDefer;   // Do not CLOSE?Delete the handle in Destructor

      std::string          m_objAttrFilter;
      Qgy_List_Info_T      m_listInfo;

      allObjSelect_t         m_selectCtrl;
      std::vector<Qgy_Olobj_SortKeyInfo_t> m_sortKeys;
      Qgy_Olobj_SortInfo_t2  m_sortInfo;
      Qwc_JIDF0100_t         m_jobInfo;
      object_Desc_t          m_objDesc;

public:
      Qgy_Olobj_Key0200_t  f200;
      Qgy_Olobj_Key0300_t  f300;
      Qgy_Olobj_Key0400_t  f400;
      Qgy_Olobj_Key0500_t  f500;
      Qgy_Olobj_Key0600_t  f600;
      Qgy_Olobj_Key0700_t  f700;
      Qgy_Olobj_Key0200_t*  p200;
      Qgy_Olobj_Key0300_t*  p300;
      Qgy_Olobj_Key0400_t*  p400;
      Qgy_Olobj_Key0500_t*  p500;
      Qgy_Olobj_Key0600_t*  p600;
      Qgy_Olobj_Key0700_t*  p700;

private:
     void  init();  // Common initialization routine; called from constructors
     char* getDataByKey_FULL(int keyID, int* rtnLen=NULL);

public:
     int   attach(const coz::listHANDLE& hList, const char* AttrFilter = NULL);
     void  detach();
     void  setAPIFmt( int APIFMT = 150 );
     void  setCurRcdNbr( int nextObj = 0 );
     void  setNextObj( int nextObj = 0 );

     void  setDefer( const bool bDefer );
     void  defer( const bool bDefer ) { setDefer( bDefer ); }
     bool  isDeferred();

     char* getDataByKey(int keyID, int* pReturnKeyLength = NULL);
     Qgy_Olobj_KeyData_t* getDataByPos(int keyPos);

     char* getObjAttr(char* pAttrBuffer = NULL);  // Get object "extended" attribute

     int addSortField(int pos, int len, short dataType=CHAR_type, char seq='A');
     int addSortFieldByKey(int key, char seq='A');

     int  addDamagedLockStatus( const char* pStatus );
     void setJobID(const char* job =  NULL, const char* jobid = NULL );

     int   getKeyCount();

     int   getHandle(coz::listHANDLE& result) const;
     int   getAPIFmt() const;

     int   getCount()  const;
     int   getRecords()  const;
     int   getCurrentRecordNumber() const;
     int   getCurRcdNbr() const;

     char* getNextObj(int& rowToFetch);
     char* getNextObj();
     void* getNextObj( char* objname, char* objlib=NULL, char* objtype=NULL);

     int   listagg(char* pObjects, int items = 2 /* 1=Obj, 2=Obj & Lib, 3=Obj, Lib & Type */);



     int   open(const char* objName, const char* objLib="*CURLIB", const char* objType="*ALL", const char* objAttr="*ALL", int APIFMT=NULL);
     int   openList(const char* objName, const char* objLib="*CURLIB", const char* objType="*ALL", const char* objAttr="*ALL");
     int   openListAll(const char* objName, const char* objLib="*CURLIB", const char* objType="*ALL", const char* objAttr="*ALL");
     void  close();

public:
     static void  joblog( const char* msgText, ... ) ;
};

#pragma datamodel(pop)

#endif
