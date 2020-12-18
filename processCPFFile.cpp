
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


void processCPFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress)
{
  int32_t            end, count, percent, old_percent, chrtr2_handle = -1, cpf_handle = -1;
  double             lat_sum, lon_sum, lat, lon;
  float              datum = 0.0;
  char               ch2_file[1024];
  CZMIL_CPF_Header   cpf_header;
  CZMIL_CPF_Data     *cpf_record;
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


  if ((cpf_handle = czmil_open_cpf_file (file, &cpf_header, CZMIL_UPDATE)) < 0)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error opening CPF file : %1").arg (QString (czmil_strerror ())));
      exit (-1);
    }


  //  Set the local_vertical_datum field in the header.

  cpf_header.local_vertical_datum = options->datum_type;


  int32_t num_recs = 100000;
  cpf_record = (CZMIL_CPF_Data *) malloc (num_recs * sizeof (CZMIL_CPF_Data));
  if (cpf_record == NULL)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error allocating cpf_record memory : %1").arg (QString (strerror (errno))));
      exit (-1);
    }

  percent = 0;
  old_percent = -1;

  for (int32_t i = 0 ; i < cpf_header.number_of_records ; i += num_recs)
    {
      end = MIN (cpf_header.number_of_records - 1, (i + num_recs - 1));

      for (int32_t m = i ; m <= end ; m++)
        {
          int32_t n = m - i;

          if (czmil_read_cpf_record (cpf_handle, m, &cpf_record[n]) < 0)
            {
              QMessageBox::critical (0, "datumShift", datumShift::tr ("Error reading CPF record : %1").arg (QString (czmil_strerror ())));
              exit (-1);
            }


          switch (options->type)
            {
            case 0:
            case 1:

              //  Since we may have up to CZMIL_MAX_RETURNS per channel and 9 channels but only 1 datum value we
              //  want to find the average position of the returns for the CZMIL_SHALLOW_CHANNEL_(1-7) and the returns
              //  for the CZMIL_DEEP_CHANNEL.  There could be as much as 800 meters between positions of returns but
              //  this usually only happens if your first return is from a bird that is right near the aircraft.  Most
              //  of the returns will be clustered within a few meters of each other.  Since the reference position
              //  is always the first return encountered we want to average any anomalies out of the equation.  We
              //  also won't be using invalid points.  In the case where we have no valid returns we'll still set the
              //  datum value (based on the reference position if we're using egm08 or chrtr2).

              lat_sum = lon_sum = 0.0;
              count = 0;

              for (int32_t j = 0 ; j < 9 ; j++)
                {
                  //  Not using the IR channel.

                  if (j != 7)
                    {
                      for (int32_t k = 0 ; k < cpf_record[n].returns[j] ; k++)
                        {
                          //  Don't use null or invalid points for our computation.

                          if (!(cpf_record[n].channel[j][k].status & (CZMIL_RETURN_INVAL)) &&
                              cpf_record[n].channel[j][k].elevation != cpf_header.null_z_value)
                            {
                              lat_sum += cpf_record[n].channel[j][k].latitude;
                              lon_sum += cpf_record[n].channel[j][k].longitude;
                              count++;
                            }
                        }
                    }
                }


              //  Just in case we don't get any returns...

              if (!count)
                {
                  lat = cpf_record[n].reference_latitude;
                  lon = cpf_record[n].reference_longitude;
                }
              else
                {
                  lat = lat_sum / (double) count;
                  lon = lon_sum / (double) count;
                }


              if (options->type)
                {
                  datum = get_egm08 (lat, lon);
                }
              else
                {
                  chrtr2_read_record_lat_lon (chrtr2_handle, lat, lon, &chrtr2_record);
                  datum = chrtr2_record.z;
                }
              break;

            case 2:
              datum = options->offset;
              break;
            }

          cpf_record[n].local_vertical_datum_offset = datum;
        }


      for (int32_t m = i ; m <= end ; m++)
        {
          int32_t n = m - i;

          if (czmil_update_cpf_record (cpf_handle, m, &cpf_record[n]))
            {
              QMessageBox::critical (0, "datumShift", datumShift::tr ("Error updting CPF record : %1").arg (QString (czmil_strerror ())));
              exit (-1);
            }


          percent = NINT (((float) m / (double) cpf_header.number_of_records) * 100.0);
          if (old_percent != percent) progress->fbar->setValue (percent);
          old_percent = percent;
        }
    }


  free (cpf_record);


  //  We need to update the header to include the local_vertical_datum.

  if (czmil_update_cpf_header (cpf_handle, &cpf_header))
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error updating CPF header : %1").arg (QString (czmil_strerror ())));
      exit (-1);
    }


  czmil_close_cpf_file (cpf_handle);


  if (options->type == 0) chrtr2_close_file (chrtr2_handle);
}
