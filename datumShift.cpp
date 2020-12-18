
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



#include "datumShift.hpp"


double settings_version = 2.0;


datumShift::datumShift (QWidget *parent, int argc, char **argv)
  : QWizard (parent)
{
  commandLine[0] = 0;
  for (int32_t i = 0 ; i < argc ; i++)
    {
      strcat (commandLine, argv[i]);
      strcat (commandLine, " ");
    }

  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/datumShiftWatermark.png"));


  envin ();


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);

  setPage (0, new startPage (this, &options));
  setPage (1, new inputPage (this, &inputFiles, &options));
  setPage (2, new runPage (this, &progress, &options));


  setButtonText (QWizard::CustomButton1, tr ("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the datum shift process"));
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



datumShift::~datumShift ()
{
}



void datumShift::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);

  switch (id)
    {
    case 0:
      input_files.clear ();
      break;

    case 1:
      break;

    case 2:

      button (QWizard::CustomButton1)->setEnabled (true);

      options.chrtr2_file = field ("chrtr2_file_edit").toString ();
      options.offset = field ("offset").toDouble ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      envout ();


      break;
    }
}



void datumShift::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



//  This is where the fun stuff happens.

void 
datumShift::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  void processBFDFile (char *file, OPTIONS *options, RUN_PROGRESS *progress);
  void processCPFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress);
  int32_t processGSFFile (char *commandLine, char *file, OPTIONS *options, RUN_PROGRESS *progress);
  void processHOFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress);
  void processTOFFile (char *file, OPTIONS *options, RUN_PROGRESS *progress);


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  Get the files from the QTextEdit box on the fileInputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList isort_files;

  isort_files.clear ();

  do
    {
      isort_files << inputCursor.block ().text ();
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  isort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (int32_t i = 0 ; i < isort_files.length () ; i++)
    {
      name = isort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  input_file_count = input_files.length ();
  QStringList arguments;
  QString tmp;


  //  Main processing loop

  progress.obar->setRange (0, input_file_count);


  //  Loop through each input file.

  for (int32_t i = 0 ; i < input_file_count ; i++)
    {
      name = input_files.at (i);


      progress.fbar->reset ();
      tmp = QString (tr ("Shifting file %1 of %2 - %3").arg (i + 1).arg (input_file_count).arg (QFileInfo (name).fileName ()));
      progress.fbox->setTitle (tmp);
      qApp->processEvents ();


      char file[1024];
      strcpy (file, name.toLatin1 ());

      if (strstr (file, ".cpf"))
        {
          processCPFFile (file, &options, &progress);
        }
      else if (strstr (file, ".hof"))
        {
          processHOFFile (file, &options, &progress);
        }
      else if (strstr (file, ".tof"))
        {
          processTOFFile (file, &options, &progress);
        }
      else if (strstr (file, ".bfd"))
        {
          processBFDFile (file, &options, &progress);
        }
      else
        {
          int32_t gsf_handle;


          /*  Check for GSF file.  */

          if (!gsfOpen (file, GSF_READONLY, &gsf_handle))
            {
              gsfClose (gsf_handle);


              if (options.type != 0)
                {
                  QMessageBox::critical (this, "datumShift", tr ("GSF files can only be shifted using CHRTR2 files."));
                  exit (-1);
                }


              processGSFFile (commandLine, file, &options, &progress);
            }
          else
            {
              QMessageBox::critical (this, "datumShift", tr ("Unrecognized input file type : %1").arg (QString (file)));
              exit (-1);
            }
        }


      progress.fbar->setValue (100);
      progress.obar->setValue (i);
      qApp->processEvents ();
    }

  progress.obar->setValue (input_file_count);


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  progress.list->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Processing complete, press Finish to exit."));

  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);
}



//  Get the users defaults.

void
datumShift::envin ()
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options.font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options.font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options.font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options.window_x = 0;
  options.window_y = 0;
  options.window_width = 900;
  options.window_height = 500;
  options.type = 0;
  options.datum_type = CZMIL_V_DATUM_MLLW;
  options.offset = 0.0;
  options.chrtr2_file = "";
  options.chrtr2_dir = ".";
  options.input_dir = ".";
  options.inputFilter = "CZMIL (*.cpf)";


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/datumShift.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/datumShift.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("datumShift");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options.window_width = settings.value (QString ("width"), options.window_width).toInt ();
  options.window_height = settings.value (QString ("height"), options.window_height).toInt ();
  options.window_x = settings.value (QString ("x position"), options.window_x).toInt ();
  options.window_y = settings.value (QString ("y position"), options.window_y).toInt ();

  options.type = settings.value (QString ("type"), options.type).toInt ();

  options.datum_type = settings.value (QString ("datum type"), options.datum_type).toInt ();

  options.offset = settings.value (QString ("offset"), options.offset).toFloat ();

  options.chrtr2_file = settings.value (QString ("chrtr2 file"), options.chrtr2_file).toString ();

  options.chrtr2_dir = settings.value (QString ("chrtr2 dir"), options.chrtr2_dir).toString ();

  options.input_dir = settings.value (QString ("input dir"), options.input_dir).toString ();

  options.inputFilter = settings.value (QString ("input filter"), options.inputFilter).toString ();


  settings.endGroup ();
}




//  Save the users defaults.

void
datumShift::envout ()
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/datumShift.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/datumShift.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("datumShift");


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("width"), options.window_width);
  settings.setValue (QString ("height"), options.window_height);
  settings.setValue (QString ("x position"), options.window_x);
  settings.setValue (QString ("y position"), options.window_y);

  settings.setValue (QString ("type"), options.type);

  settings.setValue (QString ("datum type"), options.datum_type);

  settings.setValue (QString ("offset"), options.offset);

  settings.setValue (QString ("chrtr2 file"), options.chrtr2_file);

  settings.setValue (QString ("chrtr2 dir"), options.chrtr2_dir);

  settings.setValue (QString ("input dir"), options.input_dir);

  settings.setValue (QString ("input filter"), options.inputFilter);


  settings.endGroup ();
}



void 
datumShift::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}
