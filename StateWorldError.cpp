#include "StateWorldError.h"
#include <fstream>

StateWorldError StateWorldError::instanceObj = StateWorldError();

void StateWorldError::updateProjection(const glm::ivec2& size)
{
	glm::mat4 projection2D = glm::ortho(0.f, (float)size.x, (float)size.y, 0.f, -1.0f, 1.0f);
	glViewport(0, 0, size.x, size.y);

	// update all 2D shaders
	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

}

void StateWorldError::viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the viewport
	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { scroll.x, scroll.y, 0 });

	// update all 2D shaders
	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);
}

void StateWorldError::init(StateManager& s)
{
	int width, height;
	glfwGetWindowSize(s.window, &width, &height);

	qs = ShaderManager::get("quadShader");

	font = { ResourceManager::get("pixelfont.png"), ShaderManager::get("textShader") };

	qr.shader = qs;
	qr.init();
	// Main UI
	{ 
		errorMessage.alignX(ALIGN_CENTER_X);
		errorMessage.alignY(ALIGN_CENTER_Y);
		errorMessage.size = 2;
		errorMessage.shadow = true;
		errorMessage.color = { 1,.1,.1,1 };

		ui.window = s.window;
		ui.viewportCallback = viewportCallback;
		ui.viewportUser = s.window;
		ui.font = &font;
		ui.qr = &qr;
		ui.cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		returnButton.setText("Return");
		returnButton.width = 144;
		returnButton.offsetY(-50);
		returnButton.offsetX(50);
		returnButton.alignX(ALIGN_LEFT);
		returnButton.alignY(ALIGN_BOTTOM);
		returnButton.user = &s;
		returnButton.callback = returnButtonCallback;

		ui.addElement(&errorMessage);
		ui.addElement(&returnButton);
	}

	updateProjection({ width, height });
}

void StateWorldError::render(StateManager& s)
{
	glEnable(GL_DEPTH_TEST);
	StateTitleScreen::instanceObj.render(s);

	// get width and height of window
	int w, h;
	glfwGetWindowSize(s.window, &w, &h);

	glDisable(GL_DEPTH_TEST); 

	// render a background using the QuadRenderer
	qr.setColor(0, 0, 0, 0.6f);
	qr.setPos(0, 0, w, h);
	qr.setQuadRendererMode(QuadRenderer::MODE_FILL);
	qr.render();

	// render the ui
	ui.render();

	glEnable(GL_DEPTH_TEST);
}

// Button callbacks

inline void StateWorldError::returnButtonCallback(void* user)
{
	StateManager* stateManager = (StateManager*)user;
	stateManager->popState();
}

// Inputs and updates
void StateWorldError::mouseInput(StateManager& s, double xPos, double yPos)
{
	ui.mouseInput(xPos, yPos);
}
void StateWorldError::scrollInput(StateManager& s, double xOff, double yOff)
{
	ui.scrollInput(xOff, yOff);
}
void StateWorldError::keyInput(StateManager& s, int key, int scancode, int action, int mods)
{
	ui.keyInput(key, scancode, action, mods);
}
void StateWorldError::charInput(StateManager& s, uint32_t codepoint)
{
	ui.charInput(codepoint);
}
void StateWorldError::mouseButtonInput(StateManager& s, int btn, int action, int mods)
{
	ui.mouseButtonInput(btn, action, mods);
}
void StateWorldError::update(StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);
}
void StateWorldError::close(StateManager& s)
{
	ui.clear();
}
void StateWorldError::windowResize(StateManager& s, int w, int h)
{
	updateProjection({ w, h });
}


