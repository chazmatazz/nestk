/****************************************************************************
*                                                                           *
*   Nite 1.3 - Boxes                                                        *
*                                                                           *
*   Author:     Oz Magal                                                    *
*                                                                           *
****************************************************************************/

/****************************************************************************
*                                                                           *
*   Nite 1.3	                                                            *
*   Copyright (C) 2006 PrimeSense Ltd. All Rights Reserved.                 *
*                                                                           *
*   This file has been provided pursuant to a License Agreement containing  *
*   restrictions on its use. This data contains valuable trade secrets      *
*   and proprietary information of PrimeSense Ltd. and is protected by law. *
*                                                                           *
****************************************************************************
 
*/


#ifndef STEADY_BUTTON_H
#define STEADY_BUTTON_H

#include <XnOpenNI.h>
#include "common.h"

// A class for the button.
// It is a broadcaster, with its internal push detector and flow manager connected to it.
// The push detector allows detecting pushes regardless of is state (as long as it's active),
// and the flow manager is connected to either the swipe detector of the steady detector.
// This results in a button that responds to pushes (and raises the Leave event), and to swipes and steadys,
// in turn, which effect some internal state, which is reflected in the drawing.

const float ROT_SPEED = 0.1f;

class SteadyButton : public XnVPointControl
{
 public:
  XnBoundingBox3D m_BoundingBox;
  int SELECTED_STATE;
  
  typedef void (XN_CALLBACK_TYPE *LeaveCB)(void* pUserCxt);
  typedef void (XN_CALLBACK_TYPE *SelectCB)(void* cxt);
  
  // Create the SteadyButton
 SteadyButton(const XnPoint3D& ptMins, const XnPoint3D& ptMaxs, const int type, 
              const int size, xn::DepthGenerator depthGenerator) : 
    XnVPointControl("SteadyButton"),
    m_ButtonMode(ButtonInactive),
    m_type(type),
    m_size(size),
    m_DepthGenerator(depthGenerator)  
	{
		// 2 points (projective) define a box
		m_BoundingBox.LeftBottomNear = ptMins;
		m_BoundingBox.RightTopFar = ptMaxs;
        SELECTED_STATE = SteadyButton::ButtonUnselected;

		// Create internal objects
		m_pInnerFlowRouter = new XnVFlowRouter;
		m_pSwipeDetector = new XnVSwipeDetector;
		m_pSteadyDetector = new XnVSteadyDetector;

        // Swipe detector used as a dummy detector for the flow switch
        // no selection can be made while the swipe detector is active
		m_Broadcaster.AddListener(m_pInnerFlowRouter);
        m_pInnerFlowRouter->SetActive(m_pSwipeDetector);

		// Register to notifications as broadcaster
		RegisterActivate(this, &Broadcaster_OnActivate);
		RegisterDeactivate(this, &Broadcaster_OnDeactivate);
		RegisterPrimaryPointCreate(this, &Broadcaster_OnPrimaryCreate);
		RegisterPrimaryPointDestroy(this, &Broadcaster_OnPrimaryDestroy);

		// Listen to inner activateable's events:
		m_pSteadyDetector->RegisterSteady(this, &Steady_OnSteady);
	}

	~SteadyButton()
	{
      m_Broadcaster.RemoveListener(m_pInnerFlowRouter);
      m_Broadcaster.RemoveListener(m_pSteadyDetector);
      m_Broadcaster.RemoveListener(m_pSwipeDetector);

      delete m_pInnerFlowRouter;
      delete m_pSteadyDetector;
      delete &m_Broadcaster;
	}

	// Affects the existence and color of the Button in drawing
	typedef enum {
		ButtonNone,
		ButtonSelected,
		ButtonUnselected,
		ButtonInactive
	} ButtonMode;

	void SetButtonMode(ButtonMode mode) {m_ButtonMode = mode;}
    int getType() {return m_type;}

	// Draw the button, with its frame
	void Draw()
	{
        static float rotation = 90;
        
		double rgba[4] = {0.0};
		rgba[3] = 1.0;

		XnBool bFrame = true;
		switch (m_ButtonMode)
		{
		case SteadyButton::ButtonUnselected:
			// Red
			rgba[0] = 1; break;
		case SteadyButton::ButtonInactive:
			// Grey
			rgba[0] = rgba[1] = rgba[2] = 0.5; break;
		case SteadyButton::ButtonSelected:
			// Blue
			rgba[2] = 1; break;
		case SteadyButton::ButtonNone:
			bFrame = false; break;
		}
		
		if(bFrame) {
			DrawFrame(m_BoundingBox.LeftBottomNear, m_BoundingBox.RightTopFar, 5, rgba[0], rgba[1], rgba[2]);
        }
		XnPoint3D ptTopLeft = m_BoundingBox.LeftBottomNear;
		XnPoint3D ptBottomRight = m_BoundingBox.RightTopFar;

        DrawTool(ptBottomRight.X, ptBottomRight.Y, 
                rotation, m_type, m_size*2, 1);
                

	}


	// Change flow state to steady to see if there is a selection of this button
	void SetSteadyActive() {
      m_pInnerFlowRouter->SetActive(m_pSteadyDetector);
    }

    // Change flow away from steady, no selection can be made
	void SetSwipeActive() {
      m_pInnerFlowRouter->SetActive(m_pSwipeDetector);
    }

    void SetSelected() 
    {
      printf("button %s is active\n", sButton(m_type));
      SELECTED_STATE = ButtonSelected;
      SetButtonMode(ButtonSelected);
    }
    void SetUnselected() 
    {
      printf("button %s is active\n", sButton(m_type));
      SELECTED_STATE = ButtonUnselected;
      SetButtonMode(ButtonUnselected);
      SetSwipeActive();
    }
    void TurnOff() 
    {
      printf("button %s is off\n", sButton(m_type));
      SetButtonMode(SteadyButton::ButtonNone);
      SetSwipeActive();
    }
    XnBool isActive()
    {
      return !(m_ButtonMode == SteadyButton::ButtonNone);
    }

	// Register/Unregister for SteadyButton's event - Select
	XnCallbackHandle RegisterSelect(void* UserContext, SelectCB pCB)
	{
		XnCallbackHandle handle;
		m_SelectCBs.Register(pCB, this, &handle);
		return handle;
	}
	void UnregisterSelect(XnCallbackHandle handle)
	{
		m_SelectCBs.Unregister(handle);
	}

	void Update(XnVMessage* pMessage)
	{
		XnVPointControl::Update(pMessage);
		m_Broadcaster.Update(pMessage);
	}

    void OnPointUpdate(const XnVHandPointContext* pContext)
    {
      // Convert to a 2D projection
      XnPoint3D ptProjective(pContext->ptPosition);
      m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, 
                                                    &ptProjective);

      // Check if hand position is inside this button
      if(ptProjective.X < m_BoundingBox.LeftBottomNear.X &&
         ptProjective.Y < m_BoundingBox.LeftBottomNear.Y &&
         ptProjective.X > m_BoundingBox.RightTopFar.X &&
         ptProjective.Y > m_BoundingBox.RightTopFar.Y && 
         isActive())
      {
        // Allows selection of this box if hand is steady
        SetSteadyActive();
      }
      else {SetSwipeActive();};
    }

private:
	// Callbacks for internal activateable's events:

	// The broadcaster (the SteadyButton itself)
	static void XN_CALLBACK_TYPE Broadcaster_OnActivate(void* cxt)
	{
		SteadyButton* button = (SteadyButton*)(cxt);
        if(button->isActive()) {button->SetUnselected();}
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnDeactivate(void* cxt)
	{
		SteadyButton* button = (SteadyButton*)(cxt);
        if(button->isActive()) {
          button->SetButtonMode(SteadyButton::ButtonInactive);
        }
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnPrimaryCreate(const XnVHandPointContext* hand, const XnPoint3D& ptFocus, void* cxt)
	{
		SteadyButton* button = (SteadyButton*)(cxt);
        if(button->isActive()) {
          if(button->SELECTED_STATE == ButtonSelected) {button->SetSelected();}
          else {button->SetUnselected();}
        }
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnPrimaryDestroy(XnUInt32 nID, void* cxt)
	{
		SteadyButton* button = (SteadyButton*)(cxt);
        if(button->isActive()) {
          button->SetButtonMode(SteadyButton::ButtonInactive);
        }
	}


	// Steady detector
	static void XN_CALLBACK_TYPE Steady_OnSteady(XnFloat fVelocity, void* cxt)
	{
		SteadyButton* button = (SteadyButton*)(cxt);
        //printf("Steady on Steady - button %d\n", button->getType());
        if(button->isActive()) {
          button->SetSelected();
          button->Select();
        }
	}

	// Inform all - button selected
	void Select()
	{
      m_SelectCBs.Raise();
	}

private:
    xn::DepthGenerator m_DepthGenerator;
    int m_type;
    int m_size;
	ButtonMode m_ButtonMode;

	XnVFlowRouter* m_pInnerFlowRouter;
	XnVSwipeDetector* m_pSwipeDetector;
	XnVSteadyDetector* m_pSteadyDetector;
	XnVBroadcaster m_Broadcaster;
   
	XnVEvent m_SelectCBs;
};

#endif
