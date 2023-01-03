#include <iostream>
#include <complex>
#include <vector>
#include <array>
#include <cassert>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <random> // for std::mt19937
#include <ctime> // for std::time
#include <cmath>

#include <random> // for std::mt19937
#include <ctime> // for std::time

#include "SHADER_H.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


glm::vec3 cameraPos = glm::vec3(0.0f, 3.0, 4.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//float lastX = 400, lastY = 300;
float yaw{ -90.0f }, pitch;
float lastX = 400, lastY = 300;
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
bool firstMove = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame



void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

namespace MyRandom
{
	// Initialize our mersenne twister with a random seed based on the clock (once at system startup)
	std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
}

int getRandomNumber()
{
	std::uniform_int_distribution die{ 0, 10 }; // we can create a distribution in any function that needs it
	return die(MyRandom::mersenne); // and then generate a random number from our global generator
}


template<typename T = float>

inline T linearInterPolation(const T& a, const T& b, const T& t)
{
	assert(t >= 0 && t <= 1);
	return (a * (1 - t) + b * t);
}

float smoothStepReMap(const float& a, const float& b, const float& t)
{
	assert(t >= 0 && t <= 1);
	float tRemapSmoothstep = t * t * (3 - 2 * t);
	return linearInterPolation(a, b, tRemapSmoothstep);
}


class Noise2D
{
public:
	static const unsigned int gridLength = 10;
	static const unsigned int gridLengthMask = gridLength - 1;
	static const unsigned int numberOfVertices = gridLength * gridLength;
	float grid[gridLength][gridLength]{};

	Noise2D()
	{
		// x0 = col0 and y0=rowmax
		for (int row = 0; row < gridLength; ++row)
		{
			std::srand(static_cast<unsigned int>(std::time(nullptr))); // set initial seed value to system clock
			std::rand(); // get rid of first result
			for (int column = 0; column < gridLength; ++column)
			{
				grid[row][column] = getRandomNumber()/10.0f;
			}
		}
	}

	float evaluate(const glm::vec2& c)
	{
		// x0 = col0 and y0=rowmax
		
		//Liner interpolation along Ox axis to get a and b.

		float c00 = grid[int(std::floor(c.y))][int(std::floor(c.x))];
		float c10 = grid[int(std::floor(c.y))][int(std::floor(c.x)) + 1];
		float c01 = grid[int(std::floor(c.y))+1][int(std::floor(c.x))];
		float c11 = grid[int(std::floor(c.y)) + 1][int(std::floor(c.x)) + 1];

		float tx = c.x - std::floor(c.x);
		float ty= c.y - std::floor(c.y);

		tx = tx * tx * (3 - 2 * tx);
		ty = ty * ty * (3 - 2 * ty);

		//getting a and b
		float a = linearInterPolation(c00, c10, tx);
		float b = linearInterPolation(c01, c11, tx);

		//getting noise at c
		return linearInterPolation(a, b, ty);
		

	}

	
};

const glm::vec3& normalCalc(const glm::vec3& one, const glm::vec3& two, const glm::vec3& three)
{
	glm::vec3 vec1 = two-one;
	glm::vec3 vec2 = three-one;
	glm::vec3 result = glm::cross(glm::normalize(vec2), glm::normalize(vec1));

	return result;
}


int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr))); // set initial seed value to system clock
	std::rand(); // get rid of first result



	glfwInit(); // Initializes glfW library
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "GLF Window", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Making this Window's context current for the calling thread


	// Glad manages function pointers to OpenGL functions --> Initializw GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Telling OpenGL the size of the rendering window
	glViewport(0, 0, 800, 600);
	//Once the user resized window, viewPort should readjust itself---->
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	Noise2D noise;

	int count = 0;
	float xOffSet = 0.1f;
	const float divisionFactor = 10.0f;
	const int numOfVert = noise.numberOfVertices;
	const int numberofSteps = divisionFactor * numOfVert;
	float coord[30000]{};
	float terrainWithNormals[30000]{};

	//int check{};

	for (int i = 0; i < 50; ++i)
	{
		for (int j = 0; j < 50; ++j)
		{

			glm::vec2 p{ i/divisionFactor ,j/divisionFactor  };
			//if (count % 6 == 0) { p.x = (p.x + 10.0f); }

			coord[count] = p.x;
			coord[count + 1] = noise.evaluate(p*0.9f+1.3f);
			coord[count + 2] = p.y;
			count += 3;


		}

	};
	int count1{};
	int count1RightF{};
	int count1RightS{3};

	std::vector<glm::vec3> dataOfNormalVecs{};


	for (int i = 0; i < 1000; ++i)
	{
		glm::vec3 one = { coord[count1],coord[count1 + 1],coord[count1 + 2] };
		glm::vec3 two = { coord[count1RightF + 150],coord[count1RightF + 151],coord[count1RightF + 152] };
		glm::vec3 three = { coord[count1 + 3],coord[count1 + 4],coord[count1 + 5] };

		if (count1 >= 6) { three = { coord[count1RightS + 150],coord[count1RightS + 151],coord[count1RightS + 152] }; }


		dataOfNormalVecs.push_back(normalCalc(one, two, three));
		count1 += 3;
		count1RightS += 3;
		if (count1 >= 6) { count1RightF += 3; }

	}
	int dot0x, dot0y, dot0z, dot1x, dot1y, dot1z, dot2x, dot2y, dot2z, dot3x, dot3y, dot3z, normalx, normaly, normalz;
	int check{ 0 };
	int checkForCoord{};
	int checkForN;
	int normal{};
	int offset = 10;
	for (int i = 0; i < 100; ++i)
	{
		//if (check % 18 == 0) { ++checkForN; }
		dot0x = check;
		dot0y = check + 1;
		dot0z = check + 2;
		normalx = check + 3;
		normaly = check + 4;
		normalz = check + 5;
		//normal = check;

		terrainWithNormals[dot0x] = coord[checkForCoord];
		terrainWithNormals[dot0y] = coord[checkForCoord + 1];
		terrainWithNormals[dot0z] = coord[checkForCoord + 2];
		terrainWithNormals[normalx] = dataOfNormalVecs[normal].x;
		terrainWithNormals[normaly] = dataOfNormalVecs[normal].y;
		terrainWithNormals[normalz] = dataOfNormalVecs[normal].z;

		dot1x = check + 6;
		dot1y = check + 7;
		dot1z = check + 8;

		terrainWithNormals[dot1x] = coord[offset];
		terrainWithNormals[dot1y] = coord[offset + 1];
		terrainWithNormals[dot1z] = coord[offset + 2];
		terrainWithNormals[normalx + 6] = dataOfNormalVecs[offset].x;
		terrainWithNormals[normaly + 7] = dataOfNormalVecs[offset].y;
		terrainWithNormals[normalz + 8] = dataOfNormalVecs[offset].z;

		dot2x = check + 12;
		dot2y = check + 13;
		dot2z = check + 14;

		terrainWithNormals[dot2x] = coord[checkForCoord + 3];
		terrainWithNormals[dot2y] = coord[checkForCoord + 4];
		terrainWithNormals[dot2z] = coord[checkForCoord + 5];
		terrainWithNormals[normalx + 12] = dataOfNormalVecs[normal].x;
		terrainWithNormals[normaly + 13] = dataOfNormalVecs[normal].y;
		terrainWithNormals[normalz + 14] = dataOfNormalVecs[normal].z;

		dot3x = check + 18;
		dot3y = check + 19;
		dot3z = check + 20;

		terrainWithNormals[dot3x] = coord[offset + 3];
		terrainWithNormals[dot3y] = coord[offset + 4];
		terrainWithNormals[dot3z] = coord[offset + 5];
		terrainWithNormals[normalx + 18] = dataOfNormalVecs[offset + 1].x;
		terrainWithNormals[normaly + 19] = dataOfNormalVecs[offset + 1].y;
		terrainWithNormals[normalz + 20] = dataOfNormalVecs[offset + 1].z;

		check += 24;
		checkForCoord += 6;
		++normal;


	}

	float newCoord[12000]{};
	int newCheck{};
	int newForCoord{};
	int newForDot13{};
	int newForNorm{};
	
	for (int i = 0; i < 1000; ++i)
	{
		//if (newCheck%)
		newCoord[newCheck] = coord[newForCoord];
		newCoord[newCheck+1] = coord[newForCoord+1];
		newCoord[newCheck+2] = coord[newForCoord+2];

		//for Normals
		newCoord[newCheck + 3] = dataOfNormalVecs[newForNorm].x;
		newCoord[newCheck + 4] = dataOfNormalVecs[newForNorm].y;
		newCoord[newCheck + 5] = dataOfNormalVecs[newForNorm].z;



		newCoord[newCheck+6] = coord[newForCoord+150];
		newCoord[newCheck + 7] = coord[newForCoord + 151];
		newCoord[newCheck + 8] = coord[newForCoord + 152];

		newCoord[newCheck + 9] = dataOfNormalVecs[newForNorm].x;
		newCoord[newCheck + 10] = dataOfNormalVecs[newForNorm].y;
		newCoord[newCheck + 11] = dataOfNormalVecs[newForNorm].z;


					

		newCheck += 12;
		newForCoord += 3;
		++newForNorm;
	

	}
	

	int countForVer{};
	float vertices[5000]{};
	for (int i = 0; i < 300; i+=12)
	{
		//if (i >= 12) { countForVer = i - 6; }

		vertices[i] = coord[countForVer];
		vertices[i + 1] = coord[countForVer + 1];
		vertices[i + 2] = coord[countForVer + 2];

		vertices[i + 3] = coord[countForVer + 151];
		vertices[i + 4] = coord[countForVer + 152];
		vertices[i + 5] = coord[countForVer + 153];

		vertices[i + 6] = coord[countForVer + 3];
		vertices[i + 7] = coord[countForVer + 4];
		vertices[i + 8] = coord[countForVer + 5];

		vertices[i + 9] = coord[countForVer + 154];
		vertices[i + 10] = coord[countForVer + 155];
		vertices[i + 11] = coord[countForVer + 156];

		countForVer += 6;

	}

	unsigned int indices[3000]{};
	int countForIndices0{};
	int countForIndices1{};
	int countForIndices2{};

	for (int i = 0; i < 300; i+=3)
	{
		indices[i] = countForIndices0;
		indices[i+1] = countForIndices1 + 1;
		indices[i+2] = countForIndices2 + 2;
		
		++countForIndices0;
		++countForIndices1;
		++countForIndices2;
	}

	
	unsigned int EBO;
	glGenBuffers(1, &EBO);



	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(newCoord), newCoord, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	Shader noiseT("C:/OpenGL/shaders/noiseT/vTer.txt", "C:/OpenGL/shaders/noiseT/fTer.txt");

	//Creating complete transformation
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 10.0f, 5.0f));
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (800.0f / 600.0f), 0.1f, 100.0f);
	unsigned int modelLoc = glGetUniformLocation(noiseT.ID, "model");
	unsigned int viewLoc = glGetUniformLocation(noiseT.ID, "view");
	unsigned int projectionLoc = glGetUniformLocation(noiseT.ID, "projection");



	glEnable(GL_DEPTH_TEST);


	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mouse_callback);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		noiseT.use();
		unsigned int modelLoc = glGetUniformLocation(noiseT.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(noiseT.ID, "view");
		unsigned int projectionLoc = glGetUniformLocation(noiseT.ID, "projection");
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//Set vecs for ViewPos,SourcePos, LightColor and ObjectColor;
		int objectColorLocation = glGetUniformLocation(noiseT.ID, "objectColor");
		int lightColorLocation = glGetUniformLocation(noiseT.ID, "lightColor");
		int sourcePosition = glGetUniformLocation(noiseT.ID, "sourcePosition");
		int viewPos = glGetUniformLocation(noiseT.ID, "viewPos");

		glUniform3f(objectColorLocation, 0.0f, 0.75f, 0.0f);
		glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);
		glUniform3f(sourcePosition, 3.0f, 7.0f, 5.0f);
		glUniform3f(viewPos, cameraPos.x, cameraPos.y, cameraPos.z);

		// D O N E ------>(Set vecs for ViewPos,SourcePos, LightColor and ObjectColor)

		noiseT.use();
		glBindVertexArray(VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLE_STRIP, 0,100);
		glDrawArrays(GL_TRIANGLE_STRIP, 100, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 200, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 300, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 400, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 500, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 600, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 700, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 800, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 900, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1000, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1100, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1200, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1300, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1400, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1500, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1600, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1700, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1800, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 1900, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2000, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2100, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2200, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2300, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2400, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2500, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2600, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2700, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2800, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 2900, 100);
		glDrawArrays(GL_TRIANGLE_STRIP, 3000, 100);








		//glDrawArrays(GL_TRIANGLE_STRIP, 10, 10);

		glBindVertexArray(0);






		glfwSwapBuffers(window);
		glfwPollEvents();



	}

	glfwTerminate();

	return 0;

}


void processInput(GLFWwindow* window)
{
	float cameraSpeed = 0.45f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//if (cameraPos.y = 0.3f)
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//if (cameraPos.y = 0.3f)
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::cross(cameraFront, cameraUp) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::cross(cameraFront, cameraUp) * cameraSpeed;


}

//Once the user resized window, viewPort should readjust itself---->
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMove = false;
	}

	float xoffset = xpos - lastX;
	float yoffest = lastY - ypos;

	lastX = xpos;
	lastY = ypos;
	const float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffest *= sensitivity;

	yaw += xoffset;
	pitch += yoffest;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw));
	cameraFront = glm::normalize(direction);


}

