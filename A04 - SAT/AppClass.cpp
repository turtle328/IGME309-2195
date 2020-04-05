#include "AppClass.h"
using namespace Simplex;

void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Alex Rosenbach - ajr6974@rit.edu";

	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 3.0f, 13.0f), //Position
		vector3(0.0f, 3.0f, 12.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light(0 is reserved for global light)

	//creeper
	m_pCreeper = new Model();
	m_pCreeper->Load("Minecraft\\Creeper.obj");
	m_pCreeperRB = new MyRigidBody(m_pCreeper->GetVertexList());

	//steve
	m_pSteve = new Model();
	m_pSteve->Load("Minecraft\\Steve.obj");
	m_pSteveRB = new MyRigidBody(m_pSteve->GetVertexList());
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Set model matrix to the creeper
	matrix4 mCreeper = glm::translate(m_v3Creeper) * ToMatrix4(m_qCreeper) * ToMatrix4(m_qArcBall);
	m_pCreeper->SetModelMatrix(mCreeper);
	m_pCreeperRB->SetModelMatrix(mCreeper);
	m_pMeshMngr->AddAxisToRenderList(mCreeper);

	//Set model matrix to Steve
	matrix4 mSteve = glm::translate(vector3(2.25f, 0.0f, 0.0f)) * glm::rotate(IDENTITY_M4, glm::radians(-55.0f), AXIS_Z);
	m_pSteve->SetModelMatrix(mSteve);
	m_pSteveRB->SetModelMatrix(mSteve);
	m_pMeshMngr->AddAxisToRenderList(mSteve);

	int result = m_pCreeperRB->IsColliding(m_pSteveRB);

	//GeneratePlane(result);

	m_pCreeper->AddToRenderList();
	m_pCreeperRB->AddToRenderList();

	m_pSteve->AddToRenderList();
	m_pSteveRB->AddToRenderList();

	m_pMeshMngr->Print("Colliding: ");
	if (result == eSATResults::SAT_NONE) {
		m_pMeshMngr->PrintLine("YES!", C_RED);
	}

	// number for sphere collision
	else if (result == 20) {
		m_pMeshMngr->PrintLine("No. Spheres aren't colliding.", C_YELLOW);
	}

	else {
		m_pMeshMngr->PrintLine("No. Plane found: " + SATString[result], C_YELLOW);
	}
}

//void Application::GeneratePlane(uint result) {
//	int planeIndex = m_pMeshMngr->GeneratePlane(5.0, C_RED);
//	matrix4 planeLoc = m_pCreeperRB->GetModelMatrix();
//	// calculate mid point
//	vector3 midPoint = (m_pSteveRB->GetCenterGlobal() - m_pCreeperRB->GetCenterGlobal()) / 2;
//	// translate said matrix to the mid point
//	planeLoc = glm::translate(planeLoc, midPoint);
//
//	if (result == eSATResults::SAT_AX) {
//		planeLoc = glm::rotate(planeLoc, glm::radians(90.0f), AXIS_Y);
//	}
//
//	m_pMeshMngr->AddMeshToRenderList(planeIndex, planeLoc);
//
//	planeLoc = glm::rotate(planeLoc, glm::radians(180.0f), AXIS_Y);
//	m_pMeshMngr->AddMeshToRenderList(planeIndex, planeLoc);
//}

void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}

void Application::Release(void)
{

	//release the model
	SafeDelete(m_pCreeper);

	//release the rigid body for the model
	SafeDelete(m_pCreeperRB);

	//release the model
	SafeDelete(m_pSteve);

	//release the rigid body for the model
	SafeDelete(m_pSteveRB);

	//release GUI
	ShutdownGUI();
}