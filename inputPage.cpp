
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



#include "inputPage.hpp"
#include "inputPageHelp.hpp"

inputPage::inputPage (QWidget *parent, QTextEdit **infiles, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Input Data Files"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/datumShiftWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QHBoxLayout *browseBox = new QHBoxLayout;
  browseBox->setSpacing (5);


  QGroupBox *fileBox = new QGroupBox (tr ("Files"), this);
  QHBoxLayout *fileBoxLayout = new QHBoxLayout;
  fileBox->setLayout (fileBoxLayout);
  fileBoxLayout->setSpacing (10);


  inputBrowse = new QPushButton (tr ("Browse"), this);
  inputBrowse->setToolTip (tr ("Add input files to the list"));
  inputBrowse->setWhatsThis (inputBrowseText);
  connect (inputBrowse, SIGNAL (clicked ()), this, SLOT (slotInputBrowseClicked ()));
  fileBoxLayout->addWidget (inputBrowse);
  browseBox->addWidget (fileBox, 1);


  QGroupBox *dirBox = new QGroupBox (tr ("Directories"), this);
  QHBoxLayout *dirBoxLayout = new QHBoxLayout;
  dirBox->setLayout (dirBoxLayout);
  dirBoxLayout->setSpacing (10);


  QGroupBox *maskBox = new QGroupBox (tr ("Directory file mask"), this);
  QHBoxLayout *maskBoxLayout = new QHBoxLayout;
  maskBox->setLayout (maskBoxLayout);
  maskBoxLayout->setSpacing (10);

  fileMask = new QComboBox (this);
  fileMask->setToolTip (tr ("Set the file mask for the directory Browse button"));
  fileMask->setWhatsThis (fileMaskText);
  fileMask->setEditable (true);
  fileMask->addItem ("GSF (*.d[0-9][0-9] *.gsf)");
  fileMask->addItem ("CZMIL (*.cpf)");
  fileMask->addItem ("HOF (*.hof)");
  fileMask->addItem ("TOF (*.tof)");
  fileMask->addItem ("Binary Feature Data (*.bfd)");
  maskBoxLayout->addWidget (fileMask);

  if (options->inputFilter == "CZMIL (*.cpf)")
    {
      fileMask->setCurrentIndex (1);
    }
  else if (options->inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (options->inputFilter == "TOF (*.tof)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (options->inputFilter == "Binary Feature Data (*.bfd)")
    {
      fileMask->setCurrentIndex (4);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
  connect (fileMask, SIGNAL (currentIndexChanged (const QString &)), this, SLOT (slotFileMaskTextChanged (const QString &)));
  dirBoxLayout->addWidget (maskBox);


  dirBrowse = new QPushButton (tr ("Browse"), this);
  dirBrowse->setToolTip (tr ("Add input directory contents to the list"));
  dirBrowse->setWhatsThis (dirBrowseText);
  connect (dirBrowse, SIGNAL (clicked ()), this, SLOT (slotDirBrowseClicked ()));
  dirBoxLayout->addWidget (dirBrowse);


  browseBox->addWidget (dirBox, 1);


  vbox->addLayout (browseBox);


  QGroupBox *inputBox = new QGroupBox (tr ("Input file list"), this);
  QHBoxLayout *inputBoxLayout = new QHBoxLayout;
  inputBox->setLayout (inputBoxLayout);
  inputBoxLayout->setSpacing (10);


  inputFiles = new QTextEdit (this);
  inputFiles->setWhatsThis (inputFilesText);
  inputFiles->setLineWrapMode (QTextEdit::NoWrap);
  inputBoxLayout->addWidget (inputFiles);


  vbox->addWidget (inputBox, 10);


  *infiles = inputFiles;
}



void 
inputPage::slotInputBrowseClicked ()
{
  QFileDialog fd (this, tr ("datumShift Input files"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options->input_dir);


  QStringList filters;
  filters << "GSF (*.d[0-9][0-9] *.gsf)"
          << "CZMIL (*.cpf)"
          << "HOF (*.hof)"
          << "TOF (*.tof)"
          << "Binary FeatureData (*.bfd)";

  fd.setNameFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFiles);
  fd.selectNameFilter (options->inputFilter);


  QStringList files;


  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();

      for (int32_t i = 0 ; i < files.size () ; i++)
        {
          inputFiles->append (files.at (i));
        }
    }

  options->input_dir = fd.directory ().absolutePath ();
  options->inputFilter = fd.selectedNameFilter ();

  if (options->inputFilter == "CZMIL (*.cpf)")
    {
      fileMask->setCurrentIndex (1);
    }
  else if (options->inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (options->inputFilter == "TOF (*.tof)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (options->inputFilter == "Binary Feature Data (*.bfd)")
    {
      fileMask->setCurrentIndex (4);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
}



void 
inputPage::slotDirBrowseClicked ()
{
  QString title = tr ("datumShift Input directories, mask = %1").arg (options->inputFilter);
  QFileDialog *fd = new QFileDialog (this, title);
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  QStringList filters;
  filters << "GSF (*.d[0-9][0-9] *.gsf)"
          << "CZMIL (*.cpf)"
          << "HOF (*.hof)"
          << "TOF (*.tof)"
          << "Binary Feature Data (*.bfd)";

  fd->setNameFilters (filters);

  fd->setFileMode (QFileDialog::Directory);

  fd->setNameFilter (options->inputFilter);


  //  Make the directory file dialog support multiple directories.

  QListView *fdListView = fd->findChild<QListView*> ("listView");

  if (fdListView)
    {
      fdListView->setSelectionMode (QAbstractItemView::ExtendedSelection);
    }

  QTreeView *fdTreeView = fd->findChild<QTreeView*> ();

  if (fdTreeView)
    {
      fdTreeView->setSelectionMode (QAbstractItemView::ExtendedSelection);
    }


  QString file;
  QStringList files;

  if (fd->exec () == QDialog::Accepted) 
    {
      //  Save the directory that we were in when we selected a directory.

      options->input_dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();

      for (int32_t i = 0 ; i < files.size () ; i++)
        {
          file = files.at (i);

          if (file.isEmpty ()) file = fd->directory ().absolutePath ();

          QDir dirs;
          dirs.cd (file);

          dirs.setFilter (QDir::Dirs | QDir::Readable);


          //  Get all matching files in this directory.

          QDir files_dir;
          files_dir.setFilter (QDir::Files | QDir::Readable);

          QStringList nameFilter;
          QString type;
          if (options->inputFilter == "GSF (*.d[0-9][0-9] *.gsf)")
            {
              type = "GSF";
              nameFilter << "*.d[0-9][0-9]" << "*.gsf";
            }
          else if (options->inputFilter == "CZMIL (*.cpf)")
            {
              type = "CZMIL";
              nameFilter << "*.cpf";
            }
          else if (options->inputFilter == "HOF (*.hof)")
            {
              type = "HOF";
              nameFilter << "*.hof";
            }
          else if (options->inputFilter == "TOF (*.tof)")
            {
              type = "TOF";
              nameFilter << "*.tof";
            }
          else if (options->inputFilter == "Binary Feature Data (*.bfd)")
            {
              type = "BFD";
              nameFilter << "*.bfd";
            }
          else
            {
              nameFilter << options->inputFilter;
            }
          files_dir.setNameFilters (nameFilter);


          if (files_dir.cd (file))
            {
              QFileInfoList flist = files_dir.entryInfoList ();
              for (int32_t j = 0 ; j < flist.size () ; j++)
                {
                  //  Don't load HOF timing lines by default.  These can still be loaded using the file browser.

                  QString tst = flist.at (j).absoluteFilePath ();

                  if (!options->inputFilter.contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                    {
                      inputFiles->append (tst);
                    }
                }
            }


          //  Get all directories in this directory.

          QFileInfoList dlist = dirs.entryInfoList ();
          QStringList dirList;
          for (int32_t j = 0 ; j < dlist.size () ; j++)
            {
              if (dlist.at (j).fileName () != "." && dlist.at (j).fileName () != "..") 
                dirList.append (dlist.at (j).absoluteFilePath ());
            }


          //  Get all subordinate directories.

          for (int32_t j = 0 ; j < dirList.size () ; j++)
            {
              QString dirName = dirList.at (j);

              if (dirs.cd (dirName))
                {
                  QFileInfoList nlist = dirs.entryInfoList ();
                  for (int32_t k = 0 ; k < nlist.size () ; k++)
                    {
                      if (nlist.at (k).fileName () != "." && nlist.at (k).fileName () != "..") 
                        dirList.append (nlist.at (k).absoluteFilePath ());
                    }
                }
            }


          //  Get all matching files in all subordinate directories

          for (int32_t j = 0 ; j < dirList.size () ; j++)
            {
              files_dir.setFilter (QDir::Files | QDir::Readable);
              files_dir.setNameFilters (nameFilter);

              QString dirName = dirList.at (j);

              if (files_dir.cd (dirName))
                {
                  QFileInfoList sub_flist = files_dir.entryInfoList ();

                  for (int32_t k = 0 ; k < sub_flist.size () ; k++)
                    {

                      //  Don't load HOF timing lines by default.  These can still be loaded using the file browser.

                      QString tst = sub_flist.at (k).absoluteFilePath ();

                      if (!options->inputFilter.contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                        {
                          inputFiles->append (tst);
                        }
                    }
                }
            }
        }
    }
}



void 
inputPage::slotFileMaskTextChanged (const QString &text)
{
  options->inputFilter = text;
}
