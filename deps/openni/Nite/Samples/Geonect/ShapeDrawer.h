#ifndef GKT_SHAPEDRAWER_H
#define GKT_SHAPEDRAWER_H

#include <map>
#include <list>
#include <XnCppWrapper.h>
#include <XnVPointControl.h>
#include "Shape.h"
#include "RectPrism.h"


#define CUBE 0
#define SPHERE 1
#define CONE 2
#define TORUS 3
#define TRANSLATE 4
#define ROTATE 5
#define STRETCH 6
#define SELECT 7

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
	 * Draw the points, each with its own color.
	 */
	void Draw() const;


	void SetActive(XnBool active);
	/**
	 * Change mode - set the active tool
	 */
	void SetTool(int tool);
    
    void AddShape(int shapeType);
protected:
	// Number of previous position to store for each hand
	XnUInt32 m_nHistorySize;
	// previous positions per hand
	std::map<XnUInt32, std::list<XnPoint3D> > m_History;
	// Source of the depth map
	xn::DepthGenerator m_DepthGenerator;
	XnFloat* m_pfPositionBuffer;

    XnBool m_bActive;
	int m_Tool;
    
    std::list<Shape*> m_Shapes;
    Shape* m_CurrentShape;
};

#endif
