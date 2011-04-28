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

#define MAX_DIST 100


// Constructor. Receives the number of previous positions to store per hand,
// and a source for depth map
GktShapeDrawer::GktShapeDrawer(xn::DepthGenerator depthGenerator) :
	XnVPointControl("GktShapeDrawer"),
	m_DepthGenerator(depthGenerator), 
    m_nHistorySize(2), 
m_bActive(false),
    m_Tool(TRANSLATE),
m_CurrentShape(NULL)    
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

void GktShapeDrawer::AddShape(int shapeType)
{
    Shape* shape;
    XnPoint3D ptProjective(m_History[GetPrimaryID()].front());
    m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, &ptProjective);
    ptProjective.Z = 0;
    printf("AddShape Point (%f,%f,%f)", ptProjective.X, ptProjective.Y, ptProjective.Z);
    
    // Create object for this button
    switch(shapeType) {
        case CUBE:
            printf("Created shape CUBE!\n");
            shape = new RectPrism(ptProjective.X, 
                                                   ptProjective.Y, ptProjective.Z, 0, 0, 50, 50, 50);
            break;
        case SPHERE:
            printf("Created shape SPHERE!\n");
            shape = new RectPrism(ptProjective.X, 
                                                   ptProjective.Y, ptProjective.Z, 0, 0, 50, 50, 50);          break;
        case CONE:
            printf("Created shape CONE!\n");
            shape = new RectPrism(ptProjective.X, 
                                                   ptProjective.Y, ptProjective.Z, 0, 0, 50, 50, 50);          break;
        case TORUS:
            printf("Created shape TORUS!\n");
            shape = new RectPrism(ptProjective.X, 
                                                   ptProjective.Y, ptProjective.Z, 0, 0, 50, 50, 50);
            break;
    }

    
    
    m_Shapes.push_front(shape);
    m_CurrentShape = shape;
    SetActive(true);
}

// Handle creation of a new hand
static XnBool bShouldPrint = false;
void GktShapeDrawer::OnPointCreate(const XnVHandPointContext* cxt)
{
	printf("** %d\n", cxt->nID);
	// Create entry for the hand
	m_History[cxt->nID].clear();
	bShouldPrint = true;
	OnPointUpdate(cxt);
	bShouldPrint = true;
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
    
    
    if(m_bActive) {
        if(!m_CurrentShape) {
            XnPoint3D curr = m_History[cxt->nID].front();
            XnFloat min_dist;
            Shape* min_shape = 0;
            
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
            if(min_shape != 0) {
                m_CurrentShape = min_shape;
            }
        } else {
            XnPoint3D a = m_History[cxt->nID].front();
            XnPoint3D b = m_History[cxt->nID].back();
            XnPoint3D d;
            d.X = a.X-b.X;
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
           
            shape->drawHighlighted();
        }  
        else {
            shape->draw();
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