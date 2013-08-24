#ifndef __GLOBALS_H__
#define __GLOBALS_H__



#include <string.h>



#define MAX_TILE_COUNT 80

typedef unsigned short WORD;
typedef unsigned char BYTE;

class CMapError
   {
   public:
   int m_nX;
   int m_nY;
   char m_szErrorDescription[128];

   CMapError (int nX, int nY, char *psz)
      {
      m_nX = nX;
      m_nY = nY;
      strcpy(m_szErrorDescription, psz);
      }

   protected:
   CMapError ()
      {
      }

   };

class CGenericError
   {
   public:
      char m_szErrorDescription[1068];

      CGenericError (char *psz)
         {
         strcpy(m_szErrorDescription, psz);
         }

   protected:
      CGenericError ()
         {
         }
   };



#endif