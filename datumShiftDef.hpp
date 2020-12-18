
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/


#ifndef __DATUMSHIFTDEF_H__
#define __DATUMSHIFTDEF_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#ifdef NVWIN3X
    #include <winsock.h>
#endif

#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "binaryFeatureData.h"
#include "czmil.h"
#include "gsf.h"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"

#include "chrtr2.h"
#include "nvutility.h"
#include "nvutility.hpp"


typedef struct
{
  int32_t                     window_x;
  int32_t                     window_y;
  int32_t                     window_width;
  int32_t                     window_height;
  uint8_t                     type;                 /*!<  Set to 0 for CHRTR2 grid file correction (from datumSurface program), 1 for EGM2008 correction,
                                                          2 for fixed value offset.  */
  float                       offset;               //!<  Fixed offset
  QString                     chrtr2_file;          //!<  Last used CHRTR2 file name
  QString                     chrtr2_dir;           //!<  Last used CHRTR2 input directory
  QString                     input_dir;            //!<  Last used input file directory
  QString                     inputFilter;          //!<  Last used input file mask
  int32_t                     datum_type;           //!<  Datum from czmil_macros.h
  QFont                       font;                 //!<  Font used for all ABE GUI applications
} OPTIONS;


typedef struct
{
  QGroupBox           *obox;
  QProgressBar        *obar;
  QGroupBox           *fbox;
  QProgressBar        *fbar;
  QListWidget         *list;
} RUN_PROGRESS;


#endif
