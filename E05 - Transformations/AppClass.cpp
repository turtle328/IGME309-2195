#include "AppClass.h"

void Application::InitVariables(void)
{
	//Make MyMesh object
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_BLACK);

	//Move global pos to the left
	globalPos = glm::translate(globalPos, vector3(-10, 0, 0));

	// make the ship parts level by level

	// level 0
	blockPositions.push_back(vector3(1, 0, 0));
	blockPositions.push_back(vector3(2, 0, 0));

	// level 1
	blockPositions.push_back(vector3(3, 1, 0));
	blockPositions.push_back(vector3(5, 1, 0));

	// this for loop hands levels 2 and 3
	for (int i = 1; i <= 5; i++) {
		blockPositions.push_back(vector3(i, 3, 0));
		if (i != 4) {
			blockPositions.push_back(vector3(i, 2, 0));
		}
	}
	// level 4
	blockPositions.push_back(vector3(1, 4, 0));
	blockPositions.push_back(vector3(3, 4, 0));
	blockPositions.push_back(vector3(4, 4, 0));

	// level 5
	for (int i = 1; i <= 3; i++) {
		blockPositions.push_back(vector3(i, 5, 0));
	}

	// level 6
	blockPositions.push_back(vector3(2, 6, 0));

	// level 7
	blockPositions.push_back(vector3(3, 7, 0));

	// middle blocks
	blockPositions.push_back(vector3(0, 2, 0));
	blockPositions.push_back(vector3(0, 3, 0));
	blockPositions.push_back(vector3(0, 4, 0));
	blockPositions.push_back(vector3(0, 5, 0));
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// move the ship
	globalPos = glm::translate(globalPos, vector3(shipSpeed, glm::cos(curDeg) * amplitude, 0));
	curDeg += shipSpeed;

	// take the vector of positions and render 2 blocks for each, the original positon and a flipped X version of it, unless it's a middle block (x = 0)
	for (vector3 blockPosition : blockPositions) {
		m_pMesh->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), glm::translate(globalPos, blockPosition));
		if (blockPosition.x != 0) {
			vector3 flipped = vector3(blockPosition);
			flipped.x = -flipped.x;
			m_pMesh->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), glm::translate(globalPos, flipped));
		}
	}

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
	if (m_pMesh != nullptr)
	{
		delete m_pMesh;
		m_pMesh = nullptr;
	}
	SafeDelete(m_pMesh1);
	//release GUI
	ShutdownGUI();
}