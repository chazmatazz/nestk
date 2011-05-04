#ifndef STEADY_BUTTON_H
#define STEADY_BUTTON_H

#include <XnOpenNI.h>
#include "common.h"

// A class for the button.
// It is a broadcaster, with its internal steady detector and flow manager 
// connected to it.  The flow manager is connected to either the swipe detector(dummy) 
// or the steady detector.  This allows the button to activate steady detection 
// whenever a hand is being tracked inside its bounding box region.  This raises
// a select event.


class SteadyButton : public XnVPointControl
{
 public:
  XnBoundingBox3D m_BoundingBox;
  
  typedef void (XN_CALLBACK_TYPE *LeaveCB)(void* pUserCxt);
  typedef void (XN_CALLBACK_TYPE *SelectCB)(void* cxt);
  
  // Create the SteadyButton
 SteadyButton(const XnPoint3D& ptMins, const XnPoint3D& ptMaxs, const int type, 
              const int size, xn::DepthGenerator depthGenerator) : 
    XnVPointControl("SteadyButton"),
    m_type(type),
    m_size(size),
    m_DepthGenerator(depthGenerator)  
	{
		// 2 points (projective) define a box
		m_BoundingBox.LeftBottomNear = ptMins;
		m_BoundingBox.RightTopFar = ptMaxs;

		// Create internal objects
		m_pInnerFlowRouter = new XnVFlowRouter;
		m_pSwipeDetector = new XnVSwipeDetector;
		m_pSteadyDetector = new XnVSteadyDetector;

        // Swipe detector used as a dummy detector for the flow switch
        // no selection can be made while the swipe detector is active
		m_Broadcaster.AddListener(m_pInnerFlowRouter);

		// Listen to inner activateable's events:
		m_pSteadyDetector->RegisterSteady(this, &Steady_OnSteady);
        
        SetButtonMode(ButtonHidden);
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

    char* sButtonMode(ButtonMode m) {
        switch(m) {
            case ButtonHidden:
                return "ButtonHidden";
            case ButtonVisible:
                return "ButtonVisible";
            case ButtonHover:
                return "ButtonHover";
        }
        
    }
    void printState() {
        printf("button %s is in state %s\n", 
               sButton(m_type), sButtonMode(m_ButtonMode));
    }
	void SetButtonMode(ButtonMode mode) {
        m_ButtonMode = mode;
        switch(mode) {
            case ButtonHidden:
                m_pInnerFlowRouter->SetActive(m_pSwipeDetector);
                break;
            case ButtonVisible:
                m_pInnerFlowRouter->SetActive(m_pSwipeDetector);
                break;
            case ButtonHover:
                m_pInnerFlowRouter->SetActive(m_pSteadyDetector);
                break;
        }
    }
    int getType() {return m_type;}

	// Draw the button, with its frame
	void Draw()
	{
        static float rotation = 90;
        
        double rgba[4] = {0.0};
        rgba[3] = 1.0;

        switch (m_ButtonMode)
        {
            case ButtonVisible:
                // Grey
                rgba[0] = rgba[1] = rgba[2] = 0.5;
                break;
            case ButtonHover:
                // Red
                rgba[0] = 1;
                break;
        }
    
        DrawFrame(m_BoundingBox.LeftBottomNear, m_BoundingBox.RightTopFar, 5, rgba[0], rgba[1], rgba[2]);
    
        XnPoint3D ptTopLeft = m_BoundingBox.LeftBottomNear;
        XnPoint3D ptBottomRight = m_BoundingBox.RightTopFar;

        DrawTool(ptBottomRight.X, ptBottomRight.Y, 
                rotation, m_type, m_size*2, 1);
	}

    void updateCounter() {
        m_Counter++;
    }
    XnInt getCounter() {
        return m_Counter;
    }
    XnBool isActive()
    {
      return m_ButtonMode != ButtonHidden;
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
        // we might as well always listen for points (as in shapedrawer)
        XnVPointControl::Update(pMessage);
        
        // we only forward updates to the broadcaster if we are active.
        if (isActive())
        {
            m_Broadcaster.Update(pMessage);
            Draw();
        }
	}

    void OnPointUpdate(const XnVHandPointContext* pContext)
    {
        // we only care about point updates if we are active
        if(isActive()) {
          // Convert to a 2D projection
          XnPoint3D ptProjective(pContext->ptPosition);
          m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, 
                                                        &ptProjective);

          // Check if hand position is inside this button
          if(ptProjective.X < m_BoundingBox.LeftBottomNear.X &&
             ptProjective.Y < m_BoundingBox.LeftBottomNear.Y &&
             ptProjective.X > m_BoundingBox.RightTopFar.X &&
             ptProjective.Y > m_BoundingBox.RightTopFar.Y) {
              SetButtonMode(ButtonHover);
          } else {
              m_Counter = 0;
              SetButtonMode(ButtonVisible);
          }
        }
    }

private:
	// Callbacks for internal activateable's events:

	// Steady detector
	static void XN_CALLBACK_TYPE Steady_OnSteady(XnFloat fVelocity, void* cxt)
	{
        printf("button steady\n");
		SteadyButton* button = (SteadyButton*)(cxt);
        button->updateCounter();
        if(button->getCounter() == BUTTON_DELAY) {
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
    XnInt m_Counter;
};

#endif
