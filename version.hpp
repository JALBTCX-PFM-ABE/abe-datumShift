
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


#ifndef VERSION

#define     VERSION     "PFM Software - datumShift V2.03 - 03/13/19"

#endif

/*

    Version 1.00
    Jan C. Depner (PFM Software)
    07/13/15

    First working version of Qt Wizard convenience app to shell the command line datum_shift program.


    Version 1.01
    Jan C. Depner (PFM Software)
    07/13/16

    Fixed silly cut and paste error in inputPage.cpp.  Caused segfault when trying to input directory.


    Version 2.00
    Jan C. Depner (PFM Software)
    08/12/16

    - No longer shells datum_shift as a QProcess.  Actually performs the shift internally.
    - Removed support for XYZ files.
    - Warning: not yet tested with GSF files.


    Version 2.01
    Jan C. Depner (PFM Software)
    08/27/16

    - Now uses the same font as all other ABE GUI apps.  Font can only be changed in pfmView Preferences.


    Version 2.02
    Jan C. Depner (PFM Software)
    09/26/17

    - A bunch of changes to support doing translations in the future.  There is a generic
      datumShift_xx.ts file that can be run through Qt's "linguist" to translate to another language.


    Version 2.03
    Jan C. Depner (PFM Software)
    03/13/19

    - Fixed bug caused by no valid returns in shot...
    - Fixed problem with fixed offset.

*/
