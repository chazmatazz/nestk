#ifndef GKT_SHAPEDRAWER_H
#define GKT_SHAPEDRAWER_H

#include <map>
#include <list>

// Headers for OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

#include <XnVHandPointContext.h>
#include <XnVSessionManager.h>
#include <XnVFlowRouter.h>
#include <XnVCircleDetector.h>
#include <XnVPushDetector.h>
#include <XnVSelectableSlider1D.h>
#include <XnVSteadyDetector.h>
#include <XnVBroadcaster.h>
#include <XnVPushDetector.h>
#include <XnVPointArea.h>

// Header for NITE
#include "XnVNite.h"

#include "common.h"
#include "Shape.h"

/**
 * This stores all the shapes and controls the rotation of shapes
 */
class GktShapeDrawer : public XnVPointControl
{
public:
    typedef void (XN_CALLBACK_TYPE *LeaveCB)(void* pUserCxt);
    typedef void (XN_CALLBACK_TYPE *SelectCB)(void* cxt);
    
	GktShapeDrawer(xn::DepthGenerator depthGenerator, XnBoundingBox3D& boundingBox);
	virtual ~GktShapeDrawer();


	/**
	 * Handle a new message.
	 * Calls other callbacks for each point
	 */
	void Update(XnVMessage* pMessage);

	/**
	 * Handle creation of a new point
	 */
	void OnPointCreate(const XnVHandPointContext* cxt);
	/**
	 * Handle new position of an existing point
	 */
	void OnPointUpdate(const XnVHandPointContext* cxt);
	/**
	 * Handle destruction of an existing point
	 */
	void OnPointDestroy(XnUInt32 nID);

	/**
	 * Draw the objects
	 */
	void Draw() const;
    /**
     * Set the ShapeDrawer to active - this means we draw it
     */
	void SetActive(XnBool active);
	/**
	 * Change mode - set the active tool
	 */
	void SetTool(int tool);
    /**
     * Drop the current object, if any
     */
    void Drop();
    
    /**
     * Remove the currently selected shape from the shapes list
     */
    void RemoveCurrentShape();


    /**
     * Add a new shape and select it
     */
    void AddShape(int shapeType);

    /** 
     * do we have a current shape
     */
    XnBool isSelected() {
        return m_CurrentShape != NULL;
    }
    /**
     * Are we hovering a shape
     */
    XnBool isHover() {
        return m_ProspectiveShape != NULL;
    }
    void setHover(Shape* hover);
    void selectShape();
    
    XnCallbackHandle RegisterSelect(void* UserContext, SelectCB pCB);
    void UnregisterSelect(XnCallbackHandle handle);
    void Select();
    
    void updateCounter() {
        m_Counter++;
    }
    XnInt getCounter() {
        return m_Counter;
    }
    
protected:
    static void XN_CALLBACK_TYPE ShapeSteady_OnSteady(XnFloat fVelocity, void* cxt) {
        //printf("Steady detected for shape\n");
        GktShapeDrawer* drawer = (GktShapeDrawer*)(cxt);
        drawer->updateCounter();
        if(drawer->getCounter() == DISPLAY_DELAY) {
            if(drawer->isHover()) {
                printf("selecting shape\n");
                drawer->selectShape();
                drawer->Select();
            }
        }
    }
    
	// Number of previous position to store for each hand
	XnUInt32 m_nHistorySize;
	// previous positions per hand
	std::map<XnUInt32, std::list<XnPoint3D> > m_History;
	// Source of the depth map
	xn::DepthGenerator m_DepthGenerator;
	XnFloat* m_pfPositionBuffer;

    XnBool m_bActive; // if the shapedrawer is active (visible)
	int m_Tool; // the tool that we're using
    
    std::list<Shape*> m_Shapes; // the list of shapes in the shapedrawer
    Shape* m_ProspectiveShape; // the shape that we are hovering
    Shape* m_CurrentShape; // the selected shape (after Steady)
    XnBoundingBox3D m_BoundingBox;
    
    XnVFlowRouter* m_pInnerFlowRouter;
	XnVSwipeDetector* m_pSwipeDetector;
	XnVSteadyDetector* m_pSteadyDetector;
	XnVBroadcaster m_Broadcaster;
    
    XnVEvent m_SelectCBs;
    XnInt m_Counter;
};

#endif
