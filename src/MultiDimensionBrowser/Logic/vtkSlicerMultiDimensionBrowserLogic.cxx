/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MultiDimension Logic includes
#include "vtkSlicerMultiDimensionBrowserLogic.h"
#include "vtkMRMLMultiDimensionBrowserNode.h"

// MRML includes
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"


// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMultiDimensionBrowserLogic);

//----------------------------------------------------------------------------
vtkSlicerMultiDimensionBrowserLogic::vtkSlicerMultiDimensionBrowserLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerMultiDimensionBrowserLogic::~vtkSlicerMultiDimensionBrowserLogic()
{  
}

//----------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic::RegisterNodes()
{
  vtkMRMLMultiDimensionBrowserNode* browserNode = vtkMRMLMultiDimensionBrowserNode::New();
  this->GetMRMLScene()->RegisterNodeClass(browserNode);
  browserNode->Delete();

  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node==NULL)
  {
    vtkErrorMacro("An invalid node is attempted to be added");
    return;
  }
  if (node->IsA("vtkMRMLMultiDimensionBrowserNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Have a vtkMRMLMultiDimensionBrowserNode node");
    //vtkUnObserveMRMLNodeMacro(node); // remove any previous observation that might have been added
    vtkObserveMRMLNodeMacro(node);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node==NULL)
  {
    vtkErrorMacro("An invalid node is attempted to be removed");
    return;
  }
  if (node->IsA("vtkMRMLMultiDimensionBrowserNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeRemoved: Have a vtkMRMLMultiDimensionBrowserNode node");
    vtkUnObserveMRMLNodeMacro(node);
  } 
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerMultiDimensionBrowserLogic
::SetMultiDimensionBrowserRootNode(vtkMRMLNode* node)
{
  vtkMRMLHierarchyNode* rootNode = vtkMRMLHierarchyNode::SafeDownCast(node);
  if (!rootNode)
  {
    return NULL;
  }

  rootNode->AllowMultipleChildrenOn();
  rootNode->SetAttribute("HierarchyType", "MultiDimension");
  rootNode->SetAttribute("MultiDimension.Name", "Time");
  rootNode->SetAttribute("MultiDimension.Unit", "Sec");
  //rootNode->SetName( "MultiDimensionHierarchy" );

  if (!this->GetMRMLScene()->GetNodeByID(rootNode->GetID()))
  {
    this->GetMRMLScene()->AddNode(rootNode);
  }

  return rootNode;
}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic
::UpdateVirtualOutputNode(vtkMRMLMultiDimensionBrowserNode* browserNode)
{
  vtkMRMLHierarchyNode* virtualOutputNode=browserNode->GetVirtualOutputNode();
  if (virtualOutputNode==NULL)
  {
    // there is no valid output node, so there is nothing to update
    return;
  }

  vtkMRMLHierarchyNode* selectedSequenceNode=browserNode->GetSelectedSequenceNode();
  if (selectedSequenceNode==NULL)
  {
    // no selected sequence node
    return;
  }

  vtkMRMLScene* scene=virtualOutputNode->GetScene();
  if (scene==NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }

  std::vector<vtkMRMLHierarchyNode*> outputConnectorNodes;
  virtualOutputNode->GetAllChildrenNodes(outputConnectorNodes);
  int numberOfSourceChildNodes=selectedSequenceNode->GetNumberOfChildrenNodes();
  for (int sourceChildNodeIndex=0; sourceChildNodeIndex<numberOfSourceChildNodes; ++sourceChildNodeIndex)
  {    
    if (selectedSequenceNode->GetNthChildNode(sourceChildNodeIndex)==NULL || 
      selectedSequenceNode->GetNthChildNode(sourceChildNodeIndex)->GetAssociatedNode()==NULL)
    {
      vtkErrorMacro("Invalid sequence node");
      continue;
    }
    vtkMRMLNode* sourceNode=selectedSequenceNode->GetNthChildNode(sourceChildNodeIndex)->GetAssociatedNode();
    vtkMRMLNode* targetOutputNode=NULL;
    for (std::vector<vtkMRMLHierarchyNode*>::iterator outputConnectorNodeIt=outputConnectorNodes.begin(); outputConnectorNodeIt!=outputConnectorNodes.end(); ++outputConnectorNodeIt)
    {
      vtkMRMLNode* outputNode=(*outputConnectorNodeIt)->GetAssociatedNode();
      if (outputNode==NULL)
      {
        vtkWarningMacro("A connector node found without an associated node");
        continue;
      }
      if (strcmp(sourceNode->GetName(),outputNode->GetName())==0)
      {
        // found a node with the same name in the hierarchy => reuse that
        targetOutputNode=outputNode;
        break;
      }
    }

    if (targetOutputNode==NULL)
    {
      // haven't found a node in the virtual output node hierarchy that we could reuse,
      // so create a new targetOutputNode
      targetOutputNode=sourceNode->CreateNodeInstance();
      scene->AddNode(targetOutputNode);
      targetOutputNode->Delete(); // ownership transferred to the scene, so we can release the pointer
      // now connect this new node to the virtualOutput hierarchy with a new connector node
      vtkMRMLHierarchyNode* outputConnectorNode=vtkMRMLHierarchyNode::New();      
      scene->AddNode(outputConnectorNode);
      outputConnectorNode->Delete(); // ownership transferred to the scene, so we can release the pointer
      outputConnectorNode->SetParentNodeID(virtualOutputNode->GetID());
      outputConnectorNode->SetAssociatedNodeID(targetOutputNode->GetID());
      outputConnectorNodes.push_back(outputConnectorNode);
    }

    // Update the target node with the contents of the source node

    /*if (targetOutputNode->IsA("vtkMRMLScalarVolumeNode"))
    {
      vtkMRMLScalarVolumeNode* targetScalarVolumeNode=vtkMRMLScalarVolumeNode::SafeDownCast(targetOutputNode);
      char* targetOutputDisplayNodeId=NULL;
      if (targetScalarVolumeNode->GetDisplayNode()==NULL)
      {
        // there is no display node yet, so create one now
        vtkMRMLScalarVolumeDisplayNode* displayNode=vtkMRMLScalarVolumeDisplayNode::New();
        displayNode->SetDefaultColorMap();
        scene->AddNode(displayNode);
        displayNode->Delete(); // ownership transferred to the scene, so we can release the pointer
        targetOutputDisplayNodeId=displayNode->GetID();
        displayNode->SetHideFromEditors(false); // TODO: remove this line, just for testing        
      }
      else
      {
        targetOutputDisplayNodeId=targetScalarVolumeNode->GetDisplayNode()->GetID();
      }      
      targetOutputNode->CopyWithSingleModifiedEvent(sourceNode);    
      targetScalarVolumeNode->SetAndObserveDisplayNodeID(targetOutputDisplayNodeId);
    }
    else
    */
    {
      // for other (generic) nodes
      targetOutputNode->CopyWithSingleModifiedEvent(sourceNode);    
    }

    // Usually source nodes are hidden from editors, so make sure that they are visible
    targetOutputNode->SetHideFromEditors(false);
    // The following renaming a hack is for making sure the volume appears in the slice viewer GUI
    std::string name=targetOutputNode->GetName();
    targetOutputNode->SetName("");
    targetOutputNode->SetName(name.c_str());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMultiDimensionBrowserLogic::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkMRMLMultiDimensionBrowserNode *browserNode = vtkMRMLMultiDimensionBrowserNode::SafeDownCast(caller);
  if (browserNode==NULL)
  {
    vtkErrorMacro("Expected a vtkMRMLMultiDimensionBrowserNode");
    return;
  }

  UpdateVirtualOutputNode(browserNode);
}
