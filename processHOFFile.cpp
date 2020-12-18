
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


void processHOFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress)
{
  int32_t            i, j, k, end, rec_num = 1, percent, old_percent, chrtr2_handle = -1;
  float              datum = 0.0;
  char               ch2_file[1024];
  FILE               *fp = NULL;
  HOF_HEADER_T       hof_header;
  HYDRO_OUTPUT_T     *hof;
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


  if ((fp = open_hof_file (file)) == NULL)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error %1, opening HOF file : %2").arg (strerror (errno)).arg (QString (file)));
      exit (-1);
    }


  percent = 0;
  old_percent = -1;

  hof_read_header (fp, &hof_header);

  int32_t num_recs = 20000;
  hof = (HYDRO_OUTPUT_T *) malloc (num_recs * sizeof (HYDRO_OUTPUT_T));
  if (hof == NULL)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error allocating HOF memory : %1").arg (QString (strerror (errno))));
      exit (-1);
    }

  for (i = 0 ; i < hof_header.text.number_shots ; i += 20000)
    {
      end = MIN (hof_header.text.number_shots, (i + num_recs +1));

      for (j = i ; j < end ; j++)
        {
          k = j - i;
          rec_num = j + 1;

          hof_read_record (fp, rec_num, &hof[k]);


          switch (options->type)
            {
            case 0:
              chrtr2_read_record_lat_lon (chrtr2_handle, hof[k].latitude, hof[k].longitude, &chrtr2_record);
              datum = chrtr2_record.z;
              break;

            case 1:
              datum = get_egm08 (hof[k].latitude, hof[k].longitude);
              break;

            case 2:
              datum = options->offset;
              break;
            }

          if (hof[k].data_type)
            {
              hof[k].kgps_datum = datum;

              if (hof[k].kgps_res_elev != -998.0)
                {
                  hof[k].correct_depth = hof[k].kgps_res_elev - datum;


                  if (hof[k].kgps_sec_elev != -998.0 && hof[k].abdc != 70)
                    {
                      hof[k].correct_sec_depth = hof[k].kgps_sec_elev - datum;
                    }
                }


              /*  Check for shoreline depth swap (72) or known land value (70).  */

              if ((hof[k].abdc == 72 || hof[k].abdc == 70) && hof[k].kgps_topo != -998.0) hof[k].correct_depth = hof[k].kgps_topo - datum;
            }
        }


      for (j = i ; j < end ; j++)
        {
          k = j - i;
          rec_num = j + 1;

	  if (hof[k].kgps_datum != 0.0) DPRINT

          hof_write_record (fp, rec_num, &hof[k]);

          percent = ((float) (j) / (float) hof_header.text.number_shots) * 100.0;
          if (old_percent != percent) progress->fbar->setValue (percent);
          old_percent = percent;
        }
    }


  free (hof);


  fclose (fp);


  if (options->type == 0) chrtr2_close_file (chrtr2_handle);
}
