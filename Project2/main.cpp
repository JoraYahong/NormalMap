#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <../shader.h>
#include <../camera.h>
#include <../model.h>
#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"
#include "utils.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <unordered_map>

#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadCubemap(std::string faces[]);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
bool firstMouse = true;




// timing
float deltaTime = 0.0f;
float lastFrame = (float)glfwGetTime();
// lighting
glm::vec3 lightPos(1.0f, -0.35f, 2.0f);
//transformation components
float rotate_step = 0.0f;
float translat_step = 0.05f;
float translat_step2 = 0.0f;
float run_speed = 1.0f;
float current_speed = 0.0f;
float current_rotate_speed = 0.0f;
float rotate_speed = 200.0f;
float camera_zoomin = 75.0f;
float dx = 0.0f;
float dz = 0.0f;
float dy = 0.0f;
float x_position = 0.35f;
float y_position = -0.35f;
float z_position = 2.0f;

float cameraHeight = 0.3f;
float selfRotation = 0.0f;
float ambientStrength;
float specularStrength;
// camera
Camera camera1(true,rotate_step,x_position, y_position, z_position,glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;



int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	
	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	// Set OpenGL options
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// load models
	// -----------
	Model ourModel("../Project2/resources/ball.fbx");
	Model stone("../Project2/resources/roling_stone.fbx");

	// Load textures
	GLuint diffuseMap = loadTexture("../Project2/resources/rock.png");
	GLuint normalMap = loadTexture("../Project2/resources/rockNormal.png");

	GLuint diffuseMap2 = loadTexture("../Project2/resources/woodd.png");
	GLuint normalMap2 = loadTexture("../Project2/resources/woodn.png");

	//load Shader
	Shader lightingShader("../Project2/effect.vs", "../Project2/effect.fs");
	lightingShader.use();
	lightingShader.setInt("diffuseMap", 0);
	lightingShader.setInt("normalMap", 1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	const char* glsl_version = "#version 330";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		
		float elapsedTime = (float)glfwGetTime();
		deltaTime = elapsedTime - lastFrame;
		float dAngle = elapsedTime * 0.002;
		lastFrame = elapsedTime;
		processInput(window);
		rotate_step += current_rotate_speed * deltaTime;
		float distance = current_speed * deltaTime;

		dx = -(float)(distance * glm::sin(glm::radians(rotate_step)));
		dz = (float)(distance * glm::cos(glm::radians(rotate_step)));
		x_position += dx;
		z_position += dz;
		y_position += dy;
		selfRotation += deltaTime * 70.0f;
		
		camera1.changePosition(rotate_step, x_position, y_position, z_position);
		// Our state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Parameters");
			ImGui::SliderFloat("Rotation", &rotate_step, 0.0f, 180.0f);
			ImGui::SliderFloat3("Light Position", &lightPos.x, -10.0f, 10.0f);
			ImGui::End();
		}
		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 1200, 900);
		lightingShader.use();
		lightingShader.setVec3("lightPos", lightPos);
		lightingShader.setVec3("viewPos", camera1.Position);


		//projection view and model matrix
		glm::mat4 projectionMatrix = glm::perspective(camera_zoomin, (float)SCR_WIDTH / SCR_HEIGHT, 0.01f, 100.0f);
		glm::mat4 viewMatrix = camera1.GetViewMatrix();
		glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
		lightingShader.setMat4("projection", projectionMatrix);
		lightingShader.setMat4("view", viewMatrix);

		//brick
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.6f, 2.5f, -10.0f));
		model = glm::scale(model, glm::vec3(0.1f,0.1f,0.1f));	
		model = glm::rotate(model, selfRotation, glm::vec3(0, 1, 0));
		model = glm::rotate(model, rotate_step, glm::vec3(1, 0, 0));
		model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
		lightingShader.setMat4("model", model);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		ourModel.Draw(lightingShader);

		//wood
		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(-3.6f, 2.5f, -10.0f));
		model2 = glm::scale(model2, glm::vec3(0.1f, 0.1f, 0.1f));
		model2 = glm::rotate(model2, selfRotation, glm::vec3(0, 1, 0));
		model2 = glm::rotate(model2, rotate_step, glm::vec3(1, 0, 0));
		model2 = glm::rotate(model2, 90.0f, glm::vec3(1, 0, 0));
		lightingShader.setMat4("model", model2);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap2);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap2);
		ourModel.Draw(lightingShader);


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		current_speed = -run_speed;
	}else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {	
		current_speed = run_speed;
	}
	else
	{
		current_speed = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		current_rotate_speed = -rotate_speed;
		translat_step2 -= 0.003f;
	}else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translat_step2 += 0.003f;
		current_rotate_speed = rotate_speed;
	}
	else
	{
		current_rotate_speed = 0.0f;
	}
	
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera1.ProcessMouseScroll(yoffset);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera1.ProcessMouseMovement(xoffset, yoffset);
}
unsigned int loadCubemap(std::string faces[])
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		cout << faces[i] << std::endl;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
};
