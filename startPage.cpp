
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/datumShiftWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The datumShift program is used to apply a local datum offset to GSF, CZMIL, CHARTS, or Binary Feature "
                                  "Data (BFD) files.  Corrections can be made using a CHRTR2 file created by the datumSurface program "
                                  "(the default), by applying a single, fixed value offset (very old school), or using EGM2008 (if you "
                                  "just have no other choice).  Select the type of correction from the <b>Correction type</b> combo box "
                                  "below then, if not using EGM2008, select a CHRTR2 file or set the fixed value depending on the correction "
                                  "type chosen.<br><br>"
                                  "<b>IMPORTANT NOTE: GSF files can <i>only</i> be corrected using a CHRTR2 grid file.</b><br><br>" 
                                  "Click the Next button to go to the <b>Input files</b> page.  Context sensitive help is available "
                                  "by clicking on the Help button and then clicking, with the Question Arrow cursor, on the "
                                  "field of interest."));


  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QHBoxLayout *typeLayout = new QHBoxLayout ();
  vbox->addLayout (typeLayout);


  QGroupBox *corBox = new QGroupBox (tr ("Correction method"), this);
  QHBoxLayout *corBoxLayout = new QHBoxLayout;
  corBox->setLayout (corBoxLayout);
  corType = new QComboBox (corBox);
  corType->setToolTip (tr ("Set the method of the datum correction"));
  corType->setWhatsThis (corTypeText);
  corType->setEditable (false);
  corType->addItem (tr ("CHRTR2 grid"));
  corType->addItem (tr ("EGM2008", "Earth Gravitational Model 2008"));
  corType->addItem (tr ("Fixed value"));
  corType->setCurrentIndex (options->type);
  connect (corType, SIGNAL (currentIndexChanged (int)), this, SLOT (slotCorTypeChanged (int)));
  corBoxLayout->addWidget (corType);


  typeLayout->addWidget (corBox);


  QGroupBox *datumBox = new QGroupBox (tr ("Datum Type"), this);
  QHBoxLayout *datumBoxLayout = new QHBoxLayout;
  datumBox->setLayout (datumBoxLayout);
  datum = new QComboBox (datumBox);
  datum->setToolTip (tr ("Change the vertical datum type"));
  datum->setWhatsThis (tr ("Change the vertical datum type"));
  datum->setEditable (false);
  for (uint16_t i = 0 ; i < 15 ; i++)
    {
      char datum_string[128];
      czmil_get_local_vertical_datum_string (i, datum_string);
      datum->addItem (QString (datum_string));
    }

  QListView *datumView = (QListView *) datum->view ();
  datumView->setRowHidden (0, true);

  datum->setCurrentIndex (options->datum_type);
  connect (datum, SIGNAL (currentIndexChanged (int)), this, SLOT (slotDatumChanged (int)));
  datumBoxLayout->addWidget (datum);
  typeLayout->addWidget (datumBox);


  QHBoxLayout *corLayout = new QHBoxLayout ();
  vbox->addLayout (corLayout);


  QGroupBox *chrtr2Box = new QGroupBox (tr ("CHRTR2 File"), this);
  QHBoxLayout *chrtr2BoxLayout = new QHBoxLayout;
  chrtr2Box->setLayout (chrtr2BoxLayout);

  chrtr2_file_edit = new QLineEdit (this);
  chrtr2_file_edit->setReadOnly (true);
  chrtr2BoxLayout->addWidget (chrtr2_file_edit, 10);

  chrtr2_file_browse = new QPushButton (tr ("Browse..."), this);
  chrtr2BoxLayout->addWidget (chrtr2_file_browse, 1);

  chrtr2_file_edit->setWhatsThis (chrtr2_fileText);
  chrtr2_file_edit->setToolTip (chrtr2_fileTip);
  chrtr2_file_browse->setWhatsThis (chrtr2_fileBrowseText);
  chrtr2_file_browse->setToolTip (chrtr2_fileBrowseTip);
  connect (chrtr2_file_browse, SIGNAL (clicked ()), this, SLOT (slotChrtr2FileBrowse ()));

  if (options->type)
    {
      chrtr2_file_edit->setEnabled (false);
      chrtr2_file_browse->setEnabled (false);
    }


  corLayout->addWidget (chrtr2Box);


  QGroupBox *offBox = new QGroupBox (tr ("Fixed Value Offset"), this);
  QHBoxLayout *offBoxLayout = new QHBoxLayout;
  offBox->setLayout (offBoxLayout);
  offset = new QDoubleSpinBox (offBox);
  offset->setDecimals (2);
  offset->setRange (-1000.0, 1000.0);
  offset->setSingleStep (1.0);
  offset->setToolTip (tr ("Fixed value offset"));
  offset->setWhatsThis (offsetText);
  offset->setValue (options->offset);
  offBoxLayout->addWidget (offset);

  if (options->type != 2) offset->setEnabled (false);


  corLayout->addWidget (offBox);


  registerField ("chrtr2_file_edit", chrtr2_file_edit);
  registerField ("offset", offset, "value", "valueChanged");
}



void 
startPage::slotCorTypeChanged (int index)
{
  options->type = index;

  offset->setEnabled (false);
  offset->setValue (0.0);
  chrtr2_file_edit->setEnabled (false);
  chrtr2_file_browse->setEnabled (false);

  switch (options->type)
    {
    case 0:
      chrtr2_file_edit->setEnabled (true);
      chrtr2_file_browse->setEnabled (true);
      break;

    case 1:
      chrtr2_file_edit->setText ("");
      break;

    case 2:
      offset->setEnabled (true);
      chrtr2_file_edit->setText ("");
      break;
    }
}



void 
startPage::slotDatumChanged (int index)
{
  options->datum_type = index;
}



void startPage::slotChrtr2FileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("datumShift CHRTR2 File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->chrtr2_dir);


  QStringList filters;
  filters << tr ("CHRTR2 file (*.ch2 *.CH2)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("CHRTR2 file (*.ch2 *.CH2)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString chrtr2_file_name = files.at (0);

      options->chrtr2_dir = fd->directory ().absolutePath ();

      if (!chrtr2_file_name.isEmpty()) chrtr2_file_edit->setText (chrtr2_file_name);
    }
}



bool 
startPage::validatePage ()
{
  if (!options->type && (chrtr2_file_edit->text ().isEmpty ()))
    {
      QMessageBox::critical (this, "datumShift", tr ("You have selected <b>CHRTR2 grid</b> as the correction type but haven't selected a CHRTR2 file!"));

      return (false);
    }

  return (true);
}
