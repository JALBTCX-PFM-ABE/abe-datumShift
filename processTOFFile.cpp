
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


void processTOFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress)
{
  int32_t            i, j, k, end, rec_num = 1, percent, old_percent, chrtr2_handle = -1;
  float              datum = 0.0;
  char               ch2_file[1024];
  FILE               *fp = NULL;
  TOF_HEADER_T       tof_header;
  TOPO_OUTPUT_T      *tof;
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


  if ((fp = open_tof_file (file)) == NULL)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error %1, opening TOF file : %2").arg
                             (strerror (errno)).arg (QString (file)));
      exit (-1);
    }


  percent = 0;
  old_percent = -1;

  tof_read_header (fp, &tof_header);

  int32_t num_recs = 20000;
  tof = (TOPO_OUTPUT_T *) malloc (num_recs * sizeof (TOPO_OUTPUT_T));
  if (tof == NULL)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error allocating TOF memory : %1").arg (QString (strerror (errno))));
      exit (-1);
    }

  for (i = 0 ; i < tof_header.text.number_shots ; i += num_recs)
    {
      end = MIN (tof_header.text.number_shots, (i + num_recs +1));

      for (j = i ; j < end ; j++)
        {
          k = j - i;
          rec_num = j + 1;

          tof_read_record (fp, rec_num, &tof[k]);

          switch (options->type)
            {
            case 0:
              chrtr2_read_record_lat_lon (chrtr2_handle, tof[k].latitude_last, tof[k].longitude_last, &chrtr2_record);
              datum = chrtr2_record.z;
              break;

            case 1:
              datum = get_egm08 (tof[k].latitude_last, tof[k].longitude_last);
              break;

            case 2:
              datum = options->offset;
              break;
            }

          tof[k].elevation_last = tof[k].result_elevation_last - datum;


          if (tof[k].result_elevation_first != -998.0)
            {
              switch (options->type)
                {
                case 0:
                  chrtr2_read_record_lat_lon (chrtr2_handle, tof[k].latitude_first, tof[k].longitude_first, &chrtr2_record);
                  datum = chrtr2_record.z;
                  break;

                case 1:
                  datum = get_egm08 (tof[k].latitude_first, tof[k].longitude_first);
                  break;

                case 2:
                  datum = options->offset;
                  break;
                }

              tof[k].elevation_first = tof[k].result_elevation_first - datum;
            }
        }


      for (j = i ; j < end ; j++)
        {
          k = j - i;
          rec_num = j + 1;

          tof_write_record (fp, rec_num, &tof[k]);


          percent = ((float) (j) / (float) tof_header.text.number_shots) * 100.0;
          if (old_percent != percent) progress->fbar->setValue (percent);
          old_percent = percent;
        }
    }


  free (tof);


  fclose (fp);


  if (options->type == 0) chrtr2_close_file (chrtr2_handle);
}
