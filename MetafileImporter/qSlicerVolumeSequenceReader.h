/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerVolumeSequenceReader_h
#define __qSlicerVolumeSequenceReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerVolumeSequenceReaderPrivate;

// Slicer includes
class vtkSlicerMetafileImporterLogic;

//-----------------------------------------------------------------------------
class qSlicerVolumeSequenceReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerVolumeSequenceReader( vtkSlicerMetafileImporterLogic* newMetafileImporterLogic = 0, QObject* parent = 0 );
  virtual ~qSlicerVolumeSequenceReader();

  void setMetafileImporterLogic( vtkSlicerMetafileImporterLogic* newMetafileImporterLogic);
  vtkSlicerMetafileImporterLogic* metafileImporterLogic() const;

  virtual QString description() const;
  virtual IOFileType fileType() const;
  virtual QStringList extensions() const;

  virtual bool load( const IOProperties& properties );
  
protected:
  QScopedPointer< qSlicerVolumeSequenceReaderPrivate > d_ptr;

private:
  Q_DECLARE_PRIVATE( qSlicerVolumeSequenceReader );
  Q_DISABLE_COPY( qSlicerVolumeSequenceReader );
};

#endif
