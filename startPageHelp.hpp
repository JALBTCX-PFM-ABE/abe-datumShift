
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


QString corTypeText = 
  startPage::tr ("Select the method to be used to datum shift the files:<br><br>"
                 "<ul>"
                 "<li>CHRTR2 grid - use a CHRTR2 file created by the <b>datumSurface</b> program</li>"
                 "<li>Fixed value - use a fixed offset value</li>"
                 "<li>EGM2008 - use corrections from the EGM2008 model</li>"
                 "</ul>");

QString chrtr2_fileText = 
  startPage::tr ("If you selected <b>CHRTR2 grid</b> as the correction type use the Browse button to select the CHRTR2 grid file "
                 "to be used to retrieve datum corrections based on geographic position.  The CHRTR2 grid file is created using "
                 "the <b>datumSurface</b> program.");

QString chrtr2_fileTip = 
  startPage::tr ("Use the browse button to select a CHRTR2 grid file from which to retrieve datum corrections.");

QString chrtr2_fileBrowseText = 
  startPage::tr ("Use this button to select the CHRTR2 grid file from which to retrieve datum corrections.");

QString chrtr2_fileBrowseTip = 
  startPage::tr ("Select the CHRTR2 grid file from which to retrieve datum corrections.");

QString offsetText = 
  startPage::tr ("If you selected <b>Fixed value</b> as the correction type set the correction value to be used in this field.");
