#include "MyOctant.h"

using namespace Simplex;

// debug helper method
void PrintVector(vector3 v)
{
	std::cout << "x: " << v.x << " y: " << v.y << " z: " << v.z << std::endl;
}

// initalize static variables
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 0;
uint MyOctant::m_uIdealEntityCount = 0;

// the big 3
// constructor for the root node
MyOctant::MyOctant(uint maxLevel, uint idealEntityCount)
{
	// init default values
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = maxLevel;
	m_uIdealEntityCount = idealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> minMaxList; // will hold all the min and max vectors of the bounding object

	int numObj = m_pEntityMngr->GetEntityCount();
	
	// add all entities to the list to find the center point 
	// and the extents to make the root of the tree
	for (int i = 0; i < numObj; i++)
	{
		MyEntity* entity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rb = entity->GetRigidBody();
		minMaxList.push_back(rb->GetMinGlobal());
		minMaxList.push_back(rb->GetMaxGlobal());
	}
	MyRigidBody* rb = new MyRigidBody(minMaxList);

	vector3 halfExtents = rb->GetHalfWidth();
	
	float maxPoint = halfExtents[0];
	// check y and z to see if they are bigger and if they are change maxPoint to it
	for (int i = 1; i < 3; i++)
	{
		if (maxPoint < halfExtents[i])
		{
			maxPoint = halfExtents[i];
		}
	}
	vector3 center = rb->GetCenterGlobal();
	minMaxList.clear();
	SafeDelete(rb);

	m_fSize = maxPoint * 2.0f;
	m_v3Center = center;
	m_v3Min = m_v3Center - (vector3) maxPoint;
	m_v3Max = m_v3Center + (vector3) maxPoint;

	m_uOctantCount++;

	std::cout << "Center: ";
	PrintVector(m_v3Center);

	std::cout << "Min: ";
	PrintVector(m_v3Min);

	std::cout << "Max: ";
	PrintVector(m_v3Max);

	ConstructTree(m_uMaxLevel);
}
// constructor for leaf nodes
MyOctant::MyOctant(vector3 center, float size)
{
	Init();
	m_v3Center = center;
	m_fSize = size;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}
// copy constructor
MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
}
// copy assignment operator
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
// destructor
MyOctant::~MyOctant() { Release(); }

// getters
float MyOctant::GetSize() { return m_fSize; }
bool MyOctant::IsLeaf() { return m_uChildren == 0; }
vector3 MyOctant::GetCenterGlobal() { return m_v3Center; }
vector3 MyOctant::GetMinGlobal() { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal() { return m_v3Max; }
uint MyOctant::GetOctantCount() { return m_uOctantCount; }
MyOctant* MyOctant::GetParent() { return m_pParent; }
MyOctant* MyOctant::GetChild(uint child)
{
	if (child > 7) return nullptr;
	return m_pChild[child];
}

// other methods
// init member values
void MyOctant::Init()
{
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}
// swap values between two octants
void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	for (int i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}
// reset variables of the octant, if used on root kills everything
void MyOctant::Release()
{
	// special release for root only
	if (m_uLevel == 0)
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}
// creates tree using subdivisions and adds dimensions to make collisions work
void MyOctant::ConstructTree(uint maxLevel)
{
	// only construct if at the root node
	if (m_uLevel != 0)
	{
		return;
	}

	Subdivide();

	AssignIDtoEntity();

	ConstructList();
}
// checks the space of the octant to see how many objects are inside of it
// returns true if there are more entities than the ideal count
bool MyOctant::ContainsMoreThan(uint entities)
{
	int count = 0;
	int objCount = m_pEntityMngr->GetEntityCount();
	for (int i = 0; i < objCount; i++)
	{
		if (IsColliding(i))
		{
			count++;
		}
		if (count > entities)
		{
			return true;
		}
	}
	return false;
}
// uses AABB collsion to check whether entity collides with the octant space
bool MyOctant::IsColliding(uint index)
{
	int objCount = m_pEntityMngr->GetEntityCount();
	
	// index out of bounds check
	if (index >= objCount)
	{
		return false;
	}

	MyEntity* entity = m_pEntityMngr->GetEntity(index);
	MyRigidBody* rb = entity->GetRigidBody();
	vector3 vMin = rb->GetMinGlobal();
	vector3 vMax = rb->GetMaxGlobal();
	
	// check for x
	if (m_v3Max.x < vMin.x || m_v3Min.x > vMax.x)
	{
		return false;
	}

	// check for y
	if (m_v3Max.y < vMin.y || m_v3Min.y > vMax.y)
	{
		return false;
	}

	// check for z
	if (m_v3Max.z < vMin.z || m_v3Min.z > vMax.z)
	{
		return false;
	}

	return true;
}

// recrusive methods
/*
   the meat and potatoes method of this Octant class, this is a powerful recursive method
   that divides up the space of the Octant into 8 smaller spaces and keeps dividing cases
   until one of the two base cases are reached: whether it contains more than ideal entity count
   or whether it's reached the max level depth
*/
void MyOctant::Subdivide()
{
	// If this node already has the ideal entity count or less return
	if (!ContainsMoreThan(m_uIdealEntityCount))
	{
		return;
	}

	// If this node has reached the maximum depth return
	if (m_uLevel >= m_uMaxLevel)
	{
		return;
	}

	// If this node already has been subdivided return
	if (m_uChildren != 0)
	{
		return;
	}

	m_uChildren = 8;

	float size = m_fSize / 4.0f;
	float size2 = size * 2.0f;

	vector3 center = m_v3Center;

	// leaf 0 - bottom left back
	center.x -= size;
	center.y -= size;
	center.z -= size;
	m_pChild[0] = new MyOctant(center, size2);

	// leaf 1 - bottom right back
	center.x += size2;
	m_pChild[1] = new MyOctant(center, size2);

	// leaf 2 - bottom right front
	center.z += size2;
	m_pChild[2] = new MyOctant(center, size2);

	// leaf 3 - bottom left front
	center.x -= size2;
	m_pChild[3] = new MyOctant(center, size2);

	// leaf 4 - top left front
	center.y += size2;
	m_pChild[4] = new MyOctant(center, size2);

	// leaf 5 - top left back
	center.z -= size2;
	m_pChild[5] = new MyOctant(center, size2);

	// leaf 6 - top right back
	center.x += size2;
	m_pChild[6] = new MyOctant(center, size2);

	// leaf 7 - top right front
	center.z += size2;
	m_pChild[7] = new MyOctant(center, size2);

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		m_pChild[i]->Subdivide();
	}
}
void MyOctant::AssignIDtoEntity()
{
	// traverse till you find a leaf
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0)
	{
		int numEntities = m_pEntityMngr->GetEntityCount();
		for (int i = 0; i < numEntities; i++)
		{
			if (IsColliding(i))
			{
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}
// constructs the list of all the children of the root node
void MyOctant::ConstructList()
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}
// display itself and its children
void MyOctant::Display(vector3 color)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), color, RENDER_WIRE);
}
// release memory of all the child branches
void MyOctant::KillBranches()
{
	// uses recursion to kill all of the children
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}
// sets up collision detection for the entity manager by using add dimension method

// unimplemented methods, couldn't find use for them
void MyOctant::Display(uint index, vector3 color)
{

}
void MyOctant::DisplayLeafs(vector3 color)
{

}
void MyOctant::ClearEntityList()
{

}