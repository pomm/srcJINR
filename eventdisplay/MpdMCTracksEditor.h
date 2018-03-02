/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#ifndef ROOT_FAIREMCTRACKSEDITOR
#define ROOT_FAIREMCTRACKSEDITOR

#include "TGedFrame.h"                  // for TGedFrame

#include "GuiTypes.h"                   // for Pixel_t
#include "Rtypes.h"                     // for MpdMCTracksEditor::Class, etc
#include "TGFrame.h"                    // for EFrameType::kChildFrame

class MpdEventManager;
class TGWindow;
class TObject;

class MpdMCTracksEditor : public TGedFrame
{
    MpdMCTracksEditor(const MpdMCTracksEditor&);            // Not implemented
    MpdMCTracksEditor& operator=(const MpdMCTracksEditor&); // Not implemented

  protected:
    TObject* fObject;
    MpdEventManager*  fManager;
  public:
    MpdMCTracksEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                       UInt_t options = kChildFrame, Pixel_t back=GetDefaultFrameBackground());
    virtual ~MpdMCTracksEditor() {}

    virtual void SetModel( TObject* obj);


    ClassDef(MpdMCTracksEditor, 0); // Specialization of TGedEditor for proper update propagation to TEveManager.
};

#endif
