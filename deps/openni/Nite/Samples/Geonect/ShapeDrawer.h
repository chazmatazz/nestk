#ifndef GKT_SHAPEDRAWER_H
#define GKT_SHAPEDRAWER_H

#include <map>
#include <list>
#include <XnCppWrapper.h>
#include <XnVFlowRouter.h>
#include <XnVSteadyDetector.h>
#include <XnVSwipeDetector.h>
#include <XnVBroadcaster.h>
#include <XnVPointControl.h>

#include "common.h"
#include "Shape.h"

/**
 * This stores all the shapes and controls the rotation of shapes
 */
class GktShapeDrawer : public XnVPointControl
{
public:
	GktShapeDrawer(xn::DepthGenerator depthGenerator);
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
     * Add a new shape and select it
     */
    void AddShape(int shapeType);

protected:
    static void XN_CALLBACK_TYPE ShapeSteady(XnFloat fVelocity, void* cxt) {
      printf("Steady detected for shape\n");
      GktShapeDrawer* drawer = (GktShapeDrawer*)(cxt);
     drawer->m_CurrentShape = drawer->m_ProspectiveShape; // warning: race condition?
        drawer->m_ProspectiveShape = NULL;
        // switch back to swipe detector
        drawer->m_pInnerFlowRouter->SetActive(drawer->m_pSwipeDetector);
        
    }
    
   XnVFlowRouter* m_pInnerFlowRouter;
	XnVSwipeDetector* m_pSwipeDetector;
	XnVSteadyDetector* m_pSteadyDetector;
	XnVBroadcaster m_Broadcaster;

	// Number of previous position to store for each hand
	XnUInt32 m_nHistorySize;
	// previous positions per hand
	std::map<XnUInt32, std::list<XnPoint3D> > m_History;
	// Source of the depth map
	xn::DepthGenerator m_DepthGenerator;
	XnFloat* m_pfPositionBuffer;

    XnBool m_bActive; // if the shapedrawer is active
	int m_Tool; // the tool that we're using
    
    std::list<Shape*> m_Shapes; // the list of shapes in the shapedrawer
    Shape* m_ProspectiveShape; // the shape that we are hovering
    Shape* m_CurrentShape; // the selected shape (after Steady)
};

#endif
