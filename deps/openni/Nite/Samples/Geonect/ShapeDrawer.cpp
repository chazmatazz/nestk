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

#define MAX_DIST 20
#define MAX_DRIFT_DIST 20


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

void GktShapeDrawer::SetActive(XnBool  active) {
    
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
    printf("Create shape initialized\n");
    Shape* shape;
    XnPoint3D ptProjective(m_History[GetPrimaryID()].front());
    printf("get history point\n");
    m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, &ptProjective);
    printf("convert real to projective\n");
    ptProjective.Z = 0;
    printf("AddShape Point (%f,%f,%f)\n", ptProjective.X, ptProjective.Y, ptProjective.Z);
    
    // Create object for this button
    printf("Created shape %s!\n", sButton(shapeType));
    switch(shapeType) {
        case CUBE:
            shape = new RectPrism(ptProjective.X, 
                                  ptProjective.Y, 
                                  ptProjective.Z, 
                                  0, 0, 50, 50, 50, m_BoundingBox);
            break;
        case CYLINDER:
            shape = new Cylinder(ptProjective.X, 
                                  ptProjective.Y, 
                                  ptProjective.Z, 
                                  0, 0, 50, 50, 50, m_BoundingBox);
            break;
        case SHAPE3:
            shape = new RectPrism(ptProjective.X, 
                                  ptProjective.Y, 
                                  ptProjective.Z, 
                                  0, 0, 50, 50, 50, m_BoundingBox);
            break;
        case SHAPE4:
            shape = new RectPrism(ptProjective.X, 
                                  ptProjective.Y, 
                                  ptProjective.Z, 
                                  0, 0, 50, 50, 50, m_BoundingBox);
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
	// positions are kept in projective coordinates, since they are only used for drawing
	XnPoint3D ptProjective(cxt->ptPosition);

	if (bShouldPrint)printf("Point (%f,%f,%f)", ptProjective.X, ptProjective.Y, ptProjective.Z);
	//m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, &ptProjective);
	if (bShouldPrint)printf(" -> (%f,%f,%f)\n", ptProjective.X, ptProjective.Y, ptProjective.Z);
    
	// Add new position to the history buffer
	m_History[cxt->nID].push_front(ptProjective);
	// Keep size of history buffer
	if (m_History[cxt->nID].size() > m_nHistorySize)
		m_History[cxt->nID].pop_back();
	bShouldPrint = false;
    

    if(m_bActive) { // if the ShapeDrawer is currently active (e.g. displayed)
        if(!isSelected()) { // if there is not a currently selected shape
            XnBool bHover = false;
            XnPoint3D curr = m_History[cxt->nID].front();
            if(isHover()) { // if there is a prospective shape, see if we are still hovering it
                XnFloat dist = m_ProspectiveShape->getDist(curr.X, curr.Y, curr.Z);
                bHover = dist < MAX_DRIFT_DIST;
            }
            if(!bHover) { // if we are not hovering a prospective
                // see if we are over a shape
                XnFloat min_dist;
                Shape* min_shape = NULL;
                std::list<Shape*>::const_iterator ShapeIterator;
                for (ShapeIterator = m_Shapes.begin();
                     ShapeIterator != m_Shapes.end();
                     ++ShapeIterator)
                {
                    Shape* shape = *ShapeIterator;
                    XnFloat dist = shape->getDist(curr.X, curr.Y, curr.Z);
                    if(shape == m_Shapes.front() || (dist < MAX_DIST && dist < min_dist)) {
                        min_dist = dist;
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
                    m_CurrentShape->displace(d.X, -d.Y, d.Z);
                    break;
                case ROTATE:
                    m_CurrentShape->rotate(d.X, d.Y);
                    break;
                case STRETCH:
                    m_CurrentShape->resize(d.X, d.Y, d.Z);
                    break;
            }
             
        }
    }
}

void GktShapeDrawer::setHover(Shape* hover) {
    if(hover != NULL) {
        //printf("ShapeDrawer: Hovering\n");
    }
    m_ProspectiveShape = hover;
}
void GktShapeDrawer::selectShape() {
    printf("ShapeDrawer: Selecting\n");
    m_CurrentShape = m_ProspectiveShape; // warning: race condition?
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
            shape->draw(SELECTED);
        }  
        else if (m_ProspectiveShape == shape) {
            shape->draw(HOVER);
        }
        else {
            shape->draw(UNSELECTED);
        }
    }
    glFlush();
	
}

// Handle a new Message
void GktShapeDrawer::Update(XnVMessage* pMessage)
{
	// PointControl's Update calls all callbacks for each hand
	XnVPointControl::Update(pMessage);

	if (m_bActive)
	{
		Draw();
	}
}



