#include "ShapeDrawer.h"
#include "XnVDepthMessage.h"
#include <XnVHandPointContext.h>

#ifdef USE_GLUT
	#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
		#include <GLUT/glut.h>
	#else
		#include <GL/glut.h>
	#endif
#else
	#include "opengles.h"
#endif


#include "RectPrism.h"
#include "Cylinder.h"

#define MAX_DIST_SQ 100
#define MAX_DRIFT_DIST_SQ 100
#define ROT_SPEED 10
#define RESIZE_SPEED 10
#define SHAPE_SIZE 50


// Constructor. Receives the number of previous positions to store per hand,
// and a source for depth map
GktShapeDrawer::GktShapeDrawer(xn::DepthGenerator depthGenerator, XnBoundingBox3D& boundingBox) :
	XnVPointControl("GktShapeDrawer"),
	m_DepthGenerator(depthGenerator), 
    m_nHistorySize(2), 
    m_bActive(false),
    m_Tool(TRANSLATE),
    m_CurrentShape(NULL),
    m_ProspectiveShape(NULL),
    m_BoundingBox(boundingBox)
{
	m_pfPositionBuffer = new XnFloat[m_nHistorySize*3];
    
    // Create internal objects
    m_pInnerFlowRouter = new XnVFlowRouter;
    m_pSwipeDetector = new XnVSwipeDetector;
    m_pSteadyDetector = new XnVSteadyDetector;
    
    // Swipe detector used as a dummy detector for the flow switch
    // no selection can be made while the swipe detector is active
    m_Broadcaster.AddListener(m_pInnerFlowRouter);
        
    // Listen to inner activateable's events:
    m_pSteadyDetector->RegisterSteady(this, &ShapeSteady_OnSteady);
    
    m_pInnerFlowRouter->SetActive(m_pSteadyDetector);
}

// Destructor. Clear all data structures
GktShapeDrawer::~GktShapeDrawer()
{
	std::map<XnUInt32, std::list<XnPoint3D> >::iterator iter;
	for (iter = m_History.begin(); iter != m_History.end(); ++iter)
	{
		iter->second.clear();
	}
	m_History.clear();

	delete []m_pfPositionBuffer;
}

void GktShapeDrawer::SetActive(XnBool active) {
    
	m_bActive = active;
}

void GktShapeDrawer::SetTool(int tool)
{
	m_Tool = tool;
}

void GktShapeDrawer::Drop() {
    m_CurrentShape = NULL;
}
void GktShapeDrawer::AddShape(int shapeType)
{
    XnPoint3D ptCurr(m_History[GetPrimaryID()].front());
    
    Shape* shape;
    switch(shapeType) {
        case CUBE:
            shape = new RectPrism(ptCurr.X, 
                                  ptCurr.Y, 
                                  ptCurr.Z, 
                                  0, 0, SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE, m_BoundingBox);
            break;
        case CYLINDER:
            shape = new Cylinder(ptCurr.X, 
                                  ptCurr.Y, 
                                  ptCurr.Z, 
                                  0, 0, SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE, m_BoundingBox);
            break;
        case SHAPE3:
            shape = new RectPrism(ptCurr.X, 
                                  ptCurr.Y, 
                                  ptCurr.Z, 
                                  0, 0, SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE, m_BoundingBox);
            break;
        case SHAPE4:
            shape = new RectPrism(ptCurr.X, 
                                  ptCurr.Y, 
                                  ptCurr.Z, 
                                  0, 0, SHAPE_SIZE, SHAPE_SIZE, SHAPE_SIZE, m_BoundingBox);
            break;
    }

    m_Shapes.push_front(shape); // add the shape to the shape list
    m_CurrentShape = shape; // set it to the selected shape
    SetActive(true); // and now set the shapedrawer active so we can see it.
}


// Handle creation of a new hand
static XnBool bShouldPrint = true;
void GktShapeDrawer::OnPointCreate(const XnVHandPointContext* cxt)
{
    printf("create point shapedrawer\n");
	printf("** %d\n", cxt->nID);
	// Create entry for the hand
	m_History[cxt->nID].clear();
	bShouldPrint = true;
	OnPointUpdate(cxt);
	bShouldPrint = true;
    printf("Create point end shapedrawer\n");
}
// Handle new position of an existing hand
void GktShapeDrawer::OnPointUpdate(const XnVHandPointContext* cxt)
{
    // we want to track points all the time for history purposes
    // for now, positions are kept in 2D Coordinates
    XnPoint3D ptProjective(cxt->ptPosition);
    m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, 
                                                  &ptProjective);
    ptProjective.Z = 0;
    
    
    // Add new position to the history buffer
	m_History[cxt->nID].push_front(ptProjective);
	// Keep size of history buffer
	if (m_History[cxt->nID].size() > m_nHistorySize)
		m_History[cxt->nID].pop_back();
    
    if(m_bActive) { // if the ShapeDrawer is currently active (e.g. displayed)
        if(!isSelected()) { // if there is not a currently selected shape
            XnBool bHover = false;
            XnPoint3D curr = m_History[cxt->nID].front();
            if(isHover()) { // if there is a prospective shape, see if we are still hovering it
                XnFloat dist_sq = m_ProspectiveShape->getDistSq(curr.X, curr.Y, curr.Z);
                bHover = dist_sq < MAX_DRIFT_DIST_SQ;
            }
            if(!bHover) { // if we are not hovering a prospective
                // see if we are over a shape
                XnFloat min_dist_sq = MAX_DIST_SQ+1;
                Shape* min_shape = NULL;
                std::list<Shape*>::const_iterator ShapeIterator;
                for (ShapeIterator = m_Shapes.begin();
                     ShapeIterator != m_Shapes.end();
                     ++ShapeIterator)
                {
                    Shape* shape = *ShapeIterator;
                    XnFloat dist_sq = shape->getDistSq(curr.X, curr.Y, curr.Z);
                    if(dist_sq < MAX_DIST_SQ && dist_sq < min_dist_sq) {
                        min_dist_sq = dist_sq;
                        min_shape = shape;
                    }
                }
                setHover(min_shape);
            } 
        } else { // we have a currently selected shape
            XnPoint3D a = m_History[cxt->nID].front();
            XnPoint3D b = m_History[cxt->nID].back();
            XnPoint3D d;
            d.X = a.X - b.X;
            d.Y = a.Y - b.Y;
            d.Z = a.Z - b.Z;
            switch(m_Tool) {
                case TRANSLATE:
                    m_CurrentShape->displace(d.X, d.Y, d.Z);
                    break;
                case ROTATE:
                    m_CurrentShape->rotate(a.X/ROT_SPEED, a.Y/ROT_SPEED);
                    break;
                case STRETCH:
                    m_CurrentShape->resize(a.X/RESIZE_SPEED, a.Y/RESIZE_SPEED, a.Z/RESIZE_SPEED);
                    break;
            }
             
        }
    }
}

void GktShapeDrawer::setHover(Shape* hover) {
    if(hover == NULL) {
        //printf("unset hover\n");
    } else {
        printf("set hover\n");
    }
    m_Counter = 0;
    m_ProspectiveShape = hover;
}
void GktShapeDrawer::selectShape() {
    printf("ShapeDrawer: Selecting\n");
    m_CurrentShape = m_ProspectiveShape;
    setHover(NULL);
}
// Handle destruction of an existing hand
void GktShapeDrawer::OnPointDestroy(XnUInt32 nID)
{
	// No need for the history buffer
	m_History.erase(nID);
}




void GktShapeDrawer::Draw() const
{
    std::list<Shape*>::const_iterator ShapeIterator;
    for (ShapeIterator = m_Shapes.begin();
         ShapeIterator != m_Shapes.end();
         ++ShapeIterator)
	{
        Shape* shape = *ShapeIterator;
        if(m_CurrentShape == shape) {
            shape->draw(DRAWSTATE_SELECTED);
        }  
        else if (m_ProspectiveShape == shape) {
            shape->draw(DRAWSTATE_HOVER);
        }
        else {
            shape->draw(DRAWSTATE_UNSELECTED);
        }
    }
    glFlush();
	
}

// Handle a new Message
void GktShapeDrawer::Update(XnVMessage* pMessage)
{
    // we want to track points all the time for history purposes
    // PointControl's Update calls all callbacks for each hand
    XnVPointControl::Update(pMessage);

	if (m_bActive)
	{
        // we only want to broadcast to our events if we are active
        m_Broadcaster.Update(pMessage);
		Draw();
	}
}

// Register/Unregister for SteadyButton's event - Select
XnCallbackHandle GktShapeDrawer::RegisterSelect(void* UserContext, SelectCB pCB)
{
    XnCallbackHandle handle;
    m_SelectCBs.Register(pCB, this, &handle);
    return handle;
}
void GktShapeDrawer::UnregisterSelect(XnCallbackHandle handle)
{
    m_SelectCBs.Unregister(handle);
}


void GktShapeDrawer::Select()
{
    m_SelectCBs.Raise();
}
