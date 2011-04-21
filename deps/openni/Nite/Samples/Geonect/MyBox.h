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
****************************************************************************/

#ifndef MY_BOX_H
#define MY_BOX_H

// A class for the box.
// It is a broadcaster, with its internal push detector and flow manager connected to it.
// The push detector allows detecting pushes regardless of is state (as long as it's active),
// and the flow manager is connected to either the swipe detector of the steady detector.
// This results in a box that responds to pushes (and raises the Leave event), and to swipes and steadys,
// in turn, which effect some internal state, which is reflected in the drawing.

const float ROT_SPEED = 0.1f;

class MyBox : public XnVPointControl
{
public:
	XnBoundingBox3D m_BoundingBox;
    XnBool SELECTED_STATE;

	typedef void (XN_CALLBACK_TYPE *LeaveCB)(void* pUserCxt);
	typedef void (XN_CALLBACK_TYPE *SelectCB)(void* cxt);

	// Create the MyBox
	MyBox(const XnPoint3D& ptMins, const XnPoint3D& ptMaxs, const int tool, 
          const int size) : 
        XnVPointControl("MyBox"),
        m_Bold(BoldNone), 
        m_FrameMode(FrameNone),
        m_tool(tool),
        m_size(size)
	{
		// 2 points (projective) define a box
		m_BoundingBox.LeftBottomNear = ptMins;
		m_BoundingBox.RightTopFar = ptMaxs;
        SELECTED_STATE = false;

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

	~MyBox()
	{
      m_Broadcaster.RemoveListener(m_pInnerFlowRouter);
      m_Broadcaster.RemoveListener(m_pSteadyDetector);
      
      delete m_pInnerFlowRouter;
      delete m_pSteadyDetector;
      delete &m_Broadcaster;
	}

	// Affects the existence and color of the frame in drawing
	typedef enum {
		FrameNone,
		FrameOver,
		FrameIn,
		FrameInactive
	} FrameMode;

	// Affects the color of each side in drawing
	typedef enum {
		BoldNone,
		BoldUp,
		BoldDown,
		BoldLeft,
		BoldRight
	} BoldSide;

	void SetFrameMode(FrameMode mode) {m_FrameMode = mode;}
	void SetBold(BoldSide bold) {m_Bold = bold;}

	// Draw the box, with its frame
	void Draw()
	{
        static float rotation = 90;
        
		double rgba[4] = {0.0};
		rgba[3] = 1.0;

		XnBool bFrame = true;
		switch (m_FrameMode)
		{
		case MyBox::FrameIn:
			// Red
			rgba[0] = 1; break;
		case MyBox::FrameInactive:
			// Grey
			rgba[0] = rgba[1] = rgba[2] = 0.5; break;
		case MyBox::FrameOver:
			// Blue
			rgba[2] = 1; break;
		case MyBox::FrameNone:
			bFrame = false; break;
		}
		
		if(bFrame) {
			DrawFrame(m_BoundingBox.LeftBottomNear, m_BoundingBox.RightTopFar, 5, rgba[0], rgba[1], rgba[2]);
            rotation+=ROT_SPEED;
        }
		XnPoint3D ptTopLeft = m_BoundingBox.LeftBottomNear;
		XnPoint3D ptBottomRight = m_BoundingBox.RightTopFar;

		// Top line
		DrawLine(ptTopLeft.X+10, ptTopLeft.Y-10, 0,
			ptBottomRight.X-10, ptTopLeft.Y-10, 0,
			1, 1, m_Bold==BoldUp?0:1, 1);
		// Right Line
		DrawLine(ptBottomRight.X-10, ptTopLeft.Y-10, 0,
			ptBottomRight.X-10, ptBottomRight.Y+10,0,
			1, 1, m_Bold==BoldRight?0:1, 1);
		// Bottom Line
		DrawLine(ptBottomRight.X-10, ptBottomRight.Y+10,0,
			ptTopLeft.X+10, ptBottomRight.Y+10,0,
			1, 1, m_Bold==BoldDown?0:1, 1);
		// Left Line
		DrawLine(ptTopLeft.X+10, ptBottomRight.Y+10,0,
			ptTopLeft.X+10, ptTopLeft.Y-10,0,
			1, 1, m_Bold==BoldLeft?0:1, 1);
        
        DrawTool((ptTopLeft.X+ptBottomRight.X)/2, 
                (ptTopLeft.Y+ptBottomRight.Y)/2, 
                rotation, m_tool, m_size, 1);
        

#if 0
		XnPoint3D ptTopLeft, ptTopRight, ptBottomRight, ptBottomLeft;

		ptTopLeft.X = m_BoundingBox.LeftBottomNear.X-10;
		ptTopLeft.Y = m_BoundingBox.RightTopFar.Y+10;
		ptTopLeft.Z = 0;

		ptTopRight.X = m_BoundingBox.RightTopFar.X+10;
		ptTopRight.Y = m_BoundingBox.RightTopFar.Y+10;
		ptTopRight.Z = 0;

		ptBottomRight.X = m_BoundingBox.RightTopFar.X+10;
		ptBottomRight.Y = m_BoundingBox.LeftBottomNear.Y-10;
		ptBottomRight.Z = 0;

		ptBottomLeft.X = m_BoundingBox.LeftBottomNear.X-10;
		ptBottomLeft.Y = m_BoundingBox.LeftBottomNear.Y-10;
		ptBottomLeft.Z = 0;


		// Bottom line
		DrawLine(ptBottomLeft, ptBottomRight,
			5, 1, m_Bold==BoldDown?0:1, 1);
		//			m_Bold==BoldUp?20:5);
		// Right Line
		DrawLine(ptBottomRight, ptTopRight,
			5, 1, m_Bold==BoldRight?0:1, 1);
		// top Line
		DrawLine(ptTopRight, ptTopLeft,
			5, 1, m_Bold==BoldUp?0:1, 1);
		// Left Line
		DrawLine(
			ptTopLeft, ptBottomLeft,
			5, 1, m_Bold==BoldLeft?0:1, 1);
#endif
	}


	// Change flow state to steady to see if there is a selection of this box
	void SetSteadyActive() {m_pInnerFlowRouter->SetActive(m_pSteadyDetector);}

    // Change flow away from steady, no selection can be made
	void SetSwipeActive() {m_pInnerFlowRouter->SetActive(m_pSwipeDetector);}

    void SetSelected() 
    {
      SELECTED_STATE = true;
      SetFrameMode(FrameOver);
    }
    void SetUnselected() 
    {
      SELECTED_STATE = false;
      SetFrameMode(FrameIn);
    }


	// Register/Unregister for MyBox's event - Select
	XnCallbackHandle RegisterSelect(void* UserContext, LeaveCB pCB)
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

private:
	// Callbacks for internal activateable's events:


	// The broadcaster (the MyBox itself)
	static void XN_CALLBACK_TYPE Broadcaster_OnActivate(void* cxt)
	{
		MyBox* box = (MyBox*)(cxt);
		box->SetUnselected();
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnDeactivate(void* cxt)
	{
		MyBox* box = (MyBox*)(cxt);
		box->SetFrameMode(MyBox::FrameInactive);
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnPrimaryCreate(const XnVHandPointContext* hand, const XnPoint3D& ptFocus, void* cxt)
	{
		MyBox* box = (MyBox*)(cxt);
        if(box->SELECTED_STATE) {box->SetSelected();}
        else {box->SetUnselected();}
	}

	static void XN_CALLBACK_TYPE Broadcaster_OnPrimaryDestroy(XnUInt32 nID, void* cxt)
	{
		MyBox* box = (MyBox*)(cxt);
		box->SetFrameMode(MyBox::FrameInactive);
	}


	// Steady detector
	static void XN_CALLBACK_TYPE Steady_OnSteady(XnFloat fVelocity, void* cxt)
	{
		MyBox* box = (MyBox*)(cxt);
        box->SetSelected();
        box->Select();
	}

	// Inform all - box selected
	void Select()
	{
      m_SelectCBs.Raise();
	}

private:
    int m_tool;
    int m_size;
	BoldSide m_Bold;
	FrameMode m_FrameMode;

	XnVFlowRouter* m_pInnerFlowRouter;
	XnVSwipeDetector* m_pSwipeDetector;
	XnVSteadyDetector* m_pSteadyDetector;
	XnVBroadcaster m_Broadcaster;

	XnVEvent m_SelectCBs;
};

#endif
