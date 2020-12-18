
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


extern int32_t gsfError;


/********************************************************************
 *
 * Function Name : writeHistory
 *
 * Description : Load the contents of the history record at program
 *    startup
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

static void writeHistory (char *commandLine, int32_t gsfHandle, char *gsfFileName, char *chrtr2File, int32_t out_of_area)
{
  int32_t       ret, comment_size;
  char          *comment, temp[512], sumFile[128];
  gsfDataID     gsfID;
  gsfRecords    rec;
  FILE          *sumFD;
  time_t        t;
  struct tm     *tm;


  memset (&rec, 0, sizeof (rec));


  // Load the contents of the gsf history record

  time (&t);
  rec.history.history_time.tv_sec = t;
  rec.history.history_time.tv_nsec = 0;


  comment_size = 512;
  comment = (char *) malloc (comment_size);
  if (comment == NULL)
    {
      perror ("Allocating comment memory");
      exit (-1);
    }

  sprintf (comment, "Datum shift applied:\n");

  sprintf (temp, "Uncorrected = %d\n", out_of_area);
  strcat (comment, temp);
  strcat(comment, "\n");

  sprintf (temp, "CHRTR2 file : %s\n", chrtr2File);
  comment_size += (strlen (temp) + 1);
  comment = (char *) realloc (comment, comment_size);
  if (comment == NULL)
    {
      perror ("Re-Allocating comment memory");
      exit (-1);
    }
  strcat (comment, temp);

  rec.history.comment = comment;


  rec.history.command_line = commandLine;
  gethostname (rec.history.host_name, sizeof (rec.history.host_name));


  //  Seek to the end of the file and write a new history record

  ret = gsfSeek (gsfHandle, GSF_END_OF_FILE);
  memset (&gsfID, 0, sizeof(gsfID));
  gsfID.recordID = GSF_RECORD_HISTORY;
  ret = gsfWrite (gsfHandle, &gsfID, &rec);
  if (ret < 0)
    {
      QString msg = datumShift::tr ("Error: %1, writing history record to file: %2").arg (gsfError).arg (gsfFileName);
      qWarning () << qPrintable (msg);
    }


  // Add a history record to the summary file

  strncpy (sumFile, gsfFileName, sizeof(sumFile));
  sumFile[strlen(sumFile) - 3] = 's';


  /* Now add the history record to the summary file
   * Entry will look like:
   * ------------------------------------------
   * | HISTORY RECORD | hh:mm:ss mmm dd, yyyy |
   * ------------------------------------------
   * Host Name: <hostname>
   * Command Line: <program command line>
   * Comment: <application specific comment>
   */

  sumFD = fopen (sumFile, "a+");
  if (sumFD == NULL) return;


  tm = gmtime (&rec.history.history_time.tv_sec);
  strftime (temp, sizeof(temp), "%H:%M:%S %b %d, %Y", tm);
  fprintf (sumFD, "\n");
  fprintf (sumFD, "------------------------------------------\n");
  fprintf (sumFD, "| HISTORY RECORD | %s |\n", temp);
  fprintf (sumFD, "------------------------------------------\n");
  fprintf (sumFD, "Host Name: %s\n", rec.history.host_name);
  fprintf (sumFD, "Command Line: %s\n", rec.history.command_line);
  if ((rec.history.comment != (char *) NULL) && (strlen (rec.history.comment) > 1)) fprintf (sumFD, "Comment: %s\n", rec.history.comment);

  fprintf (sumFD, "\n");

  fclose (sumFD);

  free (comment);

  return;
}



/********************************************************************
 *
 * Function Name : processGSFFile
 *
 * Description : This function manages the application of correctors
 *   for a single input file.
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

int32_t processGSFFile (char *commandLine, char *file, OPTIONS *options, RUN_PROGRESS *progress)
{
  double             mult, offset, dtemp, min_depth = 0.0, max_depth = 0.0, sep, sep_diff;
  uint8_t            cFlag;
  int32_t            ltemp, ret, numArrays = 0, inHandle, outHandle, chrtr2_handle, percent = 0, old_percent = -1, current, eof, len, total = 0, ioffset,
                     out_of_area = 0;
  char               ch2_file[1024], *p, outFile[1024];
  struct stat        stat_buf;
  gsfDataID          gsfID;
  gsfRecords         rec, paramRec;
  gsfMBParams        mbParams;
  CHRTR2_HEADER      chrtr2_header;
  CHRTR2_RECORD      chrtr2_record;


  strcpy (ch2_file, options->chrtr2_file.toLatin1 ());

  if ((chrtr2_handle = chrtr2_open_file (ch2_file, &chrtr2_header, CHRTR2_READONLY)) < 0)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error opening chrtr2 file : %1").arg (QString (chrtr2_strerror ())));
      exit (-1);
    }


  //  Try to open the specified gsf file

  ret = gsfOpen (file, GSF_UPDATE, &inHandle);
  if (ret)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error opening GSF file : %1").arg (QString (file)));
      exit (-1);
    }


  //  Get the MBParams from the input file and set the datum to the requested datum (note: GSF datums are the same as CZMIL datums).

  gsfGetMBParams (&paramRec, &mbParams, &numArrays);
  mbParams.vertical_datum = options->datum_type;


  // Build the name of the temporary file to work in

  strncpy (outFile, file, sizeof(outFile));
  p = outFile + strlen(outFile);
  len = strlen(outFile);
  for (int32_t i = 0 ; i < len ; i++)
    {
      p--;
      if ((*p == '/') || (*p == '\\'))
	{
          p++;
          break;
	}
    }
  sprintf(p, "datum_shift_%05d.tmp", getpid());


  // Make sure the output file doesn't exist, though this should never happen

  if ((ret = stat (outFile, &stat_buf)) != -1)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error, output file %1 already exists").arg (QString (outFile)));
      exit (-1);
    }


  // Create the output file

  ret = gsfOpen (outFile, GSF_CREATE, &outHandle);
  if (ret)
    {
      QMessageBox::critical (0, "datumShift", datumShift::tr ("Error, unable to create temporary output file %1").arg (QString (outFile)));
      exit (-1);
    }



  /*  This is the main processing loop, where we read each record, apply the correctors and write each record. Note that all pings
      in the file are processed, even if the ignore ping flag has been set.  This is done so that if later the edited data are viewed,
      they will have had the same corrections applied. All beams, except those with the ignore beam flag set but no reason specified are
      processed. (Ignore beam with no reason mask indicates that there was no detection made by the sonar.)  */

  eof = 0;
  while (!eof)
    {
      ret = gsfRead (inHandle, GSF_NEXT_RECORD, &gsfID, &rec, NULL, 0);
      if (ret < 0)
	{
          if (gsfError != GSF_READ_TO_END_OF_FILE)
	    {
              QMessageBox::critical (0, "datumShift", datumShift::tr ("Error %1, reading GSF file %2").arg (gsfError).arg (QString (outFile)));
              exit (-1);
	    }

          eof = 1;
	}
      else
        {
          if (gsfID.recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
            {
              if (chrtr2_read_record_lat_lon (chrtr2_handle, rec.mb_ping.latitude, rec.mb_ping.longitude, &chrtr2_record))
                {
                  out_of_area++;
                }
              else
                {
                  //  Check to see if this record has been tide corrected.  If so, we want to bypass it.

                  if (!(rec.mb_ping.ping_flags & 0xc000))
                    {
                      //  Note that we flip the sign on sep since, for GSF, positive Z is down.

                      sep = -chrtr2_record.z;

                      min_depth = 99999.9;
                      max_depth = -99999.9;


                      //  GSF stores sep to the nearest whole centimeter, round the values to the nearest whole unit of precision supported by the data.

                      gsfGetScaleFactor (inHandle, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, &cFlag, &mult, &offset);


                      //  Round to precision.

                      if (sep < 0.0)
                        {
                          dtemp = (sep * mult) - 0.501;
                          ltemp = (int32_t) (dtemp);
                        }
                      else
                        {
                          dtemp = (sep * mult) + 0.501;
                          ltemp = (int32_t) (dtemp);
                        }
                      sep = ((double) ltemp) / mult;


                      //  We have to do this to back out the old corrector.

                      sep_diff = sep - rec.mb_ping.sep;

                      for (int32_t i = 0 ; i < rec.mb_ping.number_beams ; i++)
                        {
                          if (rec.mb_ping.beam_flags != (uint8_t *) NULL)
                            {
                              /*  If NV_GSF_IGNORE_BEAM is set, and nothing else is set then there was no detection made by the 
                                  sonar, correct all beams but a "NULL" beam.  It makes sense to correct edited beams in case 
                                  they are un-edited later.  */

                              if (!check_flag (rec.mb_ping.beam_flags[i], NV_GSF_IGNORE_NULL_BEAM))
                                {
                                  //  Make sure this data has a depth field

                                  if (rec.mb_ping.depth != NULL)
                                    {
                                      rec.mb_ping.depth[i] += sep_diff;

                                      if (rec.mb_ping.depth[i] < min_depth) min_depth = rec.mb_ping.depth[i];
                                      if (rec.mb_ping.depth[i] > max_depth) max_depth = rec.mb_ping.depth[i];
                                    }


                                  //  Set the PING flag to indicate that we have done a datum shift (as opposed to tide correcting).

                                  rec.mb_ping.ping_flags |= NV_GSF_GPS_VERTICAL_CONTROL;


                                  //  Datum shift the nominal depth field if it is present

                                  if (rec.mb_ping.nominal_depth != NULL) rec.mb_ping.nominal_depth[i] += sep_diff;
                                }
                            }
                        }
                      rec.mb_ping.sep = sep;

                      total++;
                    }
                }


              /*  Before we write the record we need to scan it for negative values (drying heights) that may have been created when we 
                  applied the datum shift.  We also want to make sure that we haven't pushed a value over the precision limit of the
                  scale factors.  This requires a change to the scale factors.  We will use a scale factor of 0.005 m for depths of 326 meters
                  or less, 0.01 m for depths of 654 meters or less, 0.1 m for depths to 6552 meters, and 1.0 m for anything over that.  We 
                  don't apply datum shifts in depths over 200 meters so why I'm doing the last two is anybody's guess.  JCD  */

              if (min_depth != 99999.9)
                {
                  if (min_depth < 0.0) 
                    {
                      ioffset = -((int32_t) min_depth + 1);
                    }
                  else
                    {
                      ioffset = 0;
                    }

                  if (max_depth < 327.0) 
                    {
                      mult = 0.005;
                    }
                  else if (max_depth < 655.0)
                    {
                      mult = 0.01;
                    }
                  else if (max_depth < 6553)
                    {
                      mult = 0.1;
                    }
                  else
                    {
                      mult = 1.0;
                    }

                  int32_t sf = gsfLoadScaleFactor (&rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, cFlag, mult, ioffset);
                  if (sf) 
                    {
                      QString msg = datumShift::tr ("%1 %2 %3 - Error loading scale factors: %4\n").arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).arg (sf);
                      qWarning () << qPrintable (msg);
                    }
                }
            }


          // Write the current record to the output file

          ret = gsfWrite (outHandle, &gsfID, &rec);
          if (ret < 0)
            {
              QMessageBox::critical (0, "datumShift", datumShift::tr ("Error %1, writing GSF file %2").arg (gsfError).arg (QString (outFile)));
              exit (-1);
            }
        }


      //  Print a percent complete message

      current = gsfPercent (inHandle);
      if (old_percent != current) progress->fbar->setValue (percent);
      old_percent = percent;
    }


  //  Write out the parameter records (with the correct vertical datum) if we actually modified anything.

    if (total) gsfPutMBParams (&mbParams, &paramRec, outHandle, numArrays);


    //  All done with these files

    chrtr2_close_file (chrtr2_handle);
    gsfClose (inHandle);
    gsfClose (outHandle);


    //  Move the output file to the input file name

    ret = rename (outFile, file);
    if (ret)
      {
        QMessageBox::critical (0, "datumShift", datumShift::tr ("Error updating file: %1\nResults remain in file: %2").arg (QString (file).arg (QString (outFile))));
	exit (-1);
      }


          //  We need to remove the index file because changing scale factors can cause relocation of records.

    strcpy (outFile, file);
    outFile[strlen (outFile) - 3] = 'n';
    remove (outFile);


    //  Re-open the now updated input file, and add a simple history record

    ret = gsfOpen (file, GSF_UPDATE, &inHandle);
    if (ret)
      {
        QMessageBox::critical (0, "datumShift", datumShift::tr ("Unable to write history record to file: %1").arg (QString (outFile)));
        exit (-1);
      }


    writeHistory (commandLine, inHandle, file, ch2_file, out_of_area);
    gsfClose (inHandle);


    if (out_of_area)
      {
        progress->list->addItem (" ");
        QListWidgetItem *cur;

        if (!total)
          {
            cur = new QListWidgetItem (datumShift::tr ("%L1 pings outside of CHRTR2 file area - 100\%").arg (out_of_area));
          }
        else
          {
            cur = new QListWidgetItem (datumShift::tr ("%L1 pings outside of CHRTR2 file area - %2").arg (out_of_area).arg
                                       (((float) out_of_area / (float) total) * 100.0));
          }

        progress->list->addItem (cur);
        progress->list->setCurrentItem (cur);
        progress->list->scrollToItem (cur);
      }

    return (0);
}
