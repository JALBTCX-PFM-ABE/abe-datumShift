
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

#include "datumShift.hpp"


void processBFDFile (char *file, OPTIONS *options, RUN_PROGRESS *progress)
{
  int32_t            percent, old_percent, chrtr2_handle = -1, bfd_handle = -1;
  float              datum = 0.0;
  char               ch2_file[1024];
  BFDATA_HEADER      bfd_header;
  BFDATA_RECORD      bfd_record;
  CHRTR2_HEADER      chrtr2_header;
  CHRTR2_RECORD      chrtr2_record;


  //  If we're using a CHRTR2 file, open it and read the needed information.

  if (options->type == 0)
    {
      strcpy (ch2_file, options->chrtr2_file.toLatin1 ());

      if ((chrtr2_handle = chrtr2_open_file (ch2_file, &chrtr2_header, CHRTR2_READONLY)) < 0)
        {
          QMessageBox::critical (0, "datumShift", datumShift::tr ("Error opening chrtr2 file : %1").arg (QString (chrtr2_strerror ())));
          exit (-1);
        }
    }


  if ((bfd_handle = binaryFeatureData_open_file (file, &bfd_header, BFDATA_UPDATE)) < 0)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error opening BFD file : %1").arg (QString (binaryFeatureData_strerror ())));
      exit (-1);
    }

  percent = 0;
  old_percent = -1;

  for (uint32_t i = 0 ; i < bfd_header.number_of_records ; i++)
    {
      binaryFeatureData_read_record (bfd_handle, i, &bfd_record);

      switch (options->type)
        {
        case 0:
          chrtr2_read_record_lat_lon (chrtr2_handle, bfd_record.latitude, bfd_record.longitude, &chrtr2_record);
          datum = chrtr2_record.z;
          break;

        case 1:
          datum = get_egm08 (bfd_record.latitude, bfd_record.longitude);
          break;

        case 2:
          datum = options->offset;
          break;
        }

      bfd_record.datum = datum;


      binaryFeatureData_write_record (bfd_handle, i, &bfd_record, NULL, NULL);


      percent = ((float) i / (float) bfd_header.number_of_records) * 100.0;
      if (old_percent != percent) progress->fbar->setValue (percent);
      old_percent = percent;
    }


  binaryFeatureData_close_file (bfd_handle);


  if (options->type == 0) chrtr2_close_file (chrtr2_handle);
}
