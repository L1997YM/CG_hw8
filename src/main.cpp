#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>

#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "camera.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button,int action,int mods);
int jiecheng(int n);
void renderBezier(float vertices[],int n);

const char* glsl_version = "#version 330";
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
double mouse_x;
double mouse_y;
vector<float> control_point;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos,0.0,1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f,1.0f,1.0f,1.0f);\n"
"}\0";

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);			//����GLFW����Ҫʹ�õ�OpenGL�İ汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);		//����GLFW����ʹ�õ��Ǻ���ģʽ
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);	//����˵����1�����ڵĿ�2�����ڵĸߣ�3����������
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		//ÿ�����ڵ�����Сʱ����
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))		//��ʼ��GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	/*---------------------------------------------������ɫ��------------------------------------------------*/
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);		//������ɫ��������Ϊ���ͣ�������ɫ��
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);		//�����ɫ��Դ��
	glCompileShader(vertexShader);		//������ɫ��
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);		//����Ƿ����ɹ�
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);		//��ȡ������Ϣ
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/*--------------------------------------------Ƭ����ɫ��-------------------------------------------------*/
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/*--------------------------------------------��ɫ������-------------------------------------------------*/
	unsigned int shaderProgram = glCreateProgram();		//����һ�����򲢷���ID����
	glAttachShader(shaderProgram, vertexShader);		//����ɫ�����ӵ����������
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}
	glDeleteShader(vertexShader);		//ɾ����ɫ������ 
	glDeleteShader(fragmentShader);

	while (!glfwWindowShouldClose(window))		//��GLFW�˳�ǰһֱ��������
	{
		unsigned int VAO, VBO;
		processInput(window); 
		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);		//���������Ļ���õ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);		//�����ɫ���棬������ɫ���屻���ΪglClearColor�������õ���ɫ

		int length = control_point.size();
		float vertices[1000];
		for (int i = 0; i < length; ++i)
		{
			vertices[i] = control_point[i];
		}
		
		/*----------------------------------------------����VBO--------------------------------------------------*/
		glGenBuffers(1, &VBO);		//����������󣬲���1����Ҫ�����Ļ�������������2�����滺��ID�ĵ�ַ
		glBindBuffer(GL_ARRAY_BUFFER, VBO);		//���������󶨵���Ӧ�Ļ����ϣ�����1���������ͣ�������������or�����������ݣ�
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * length, vertices, GL_STATIC_DRAW);		//��vertices���Ƶ���ǰ�󶨻��壬GL_STATIC_DRAW��ʾ���ݼ�������ı�

		/*---------------------------------------------����VAO----------------------------------------------------*/
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		/*-------------------------------------------���Ӷ�������-------------------------------------------------*/
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);		//����1��Ҫ���õĶ������ԣ�����2���������ԵĴ�С
																							//����3�����ݵ����ͣ�����4������������5��ƫ����
		glEnableVertexAttribArray(0);		//���ö�������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		glUseProgram(shaderProgram);
		glPointSize(10.0f);
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, length / 2);
		glDrawArrays(GL_LINE_STRIP, 0, length / 2);
		glBindVertexArray(0);

		glPointSize(1.0f);
		renderBezier(vertices, length / 2);

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);		//������ɫ����
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	glfwTerminate();		//�ͷ�ɾ��֮ǰ�����������Դ
	return 0;
}

int jiecheng(int n)
{
	int result = 1;
	for (int i = n; i > 0; --i)
	{
		result *= i;
	}
	return result;
}

void renderBezier(float vertices[], int n)
{
	if (n < 2)
		return;
	float bezier_vertices[202];
	int num = 0;
	for (float t = 0; t <= 1; t += 0.01)
	{
		bezier_vertices[num * 2] = 0;
		bezier_vertices[num * 2 + 1] = 0;
		for (int i = 0; i < n; ++i)
		{
			bezier_vertices[num * 2] += jiecheng(n - 1) / (jiecheng(i) * jiecheng(n - 1 - i)) * pow(1 - t, n - 1 - i) * pow(t, i) * vertices[i * 2];
			bezier_vertices[num * 2 + 1] += jiecheng(n - 1) / (jiecheng(i) * jiecheng(n - 1 - i)) * pow(1 - t, n - 1 - i) * pow(t, i) * vertices[i * 2 + 1];
		}
		num++;
	}
	unsigned VAO, VBO;
	/*----------------------------------------------����VBO--------------------------------------------------*/
	glGenBuffers(1, &VBO);		//����������󣬲���1����Ҫ�����Ļ�������������2�����滺��ID�ĵ�ַ
	glBindBuffer(GL_ARRAY_BUFFER, VBO);		//���������󶨵���Ӧ�Ļ����ϣ�����1���������ͣ�������������or�����������ݣ�
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 202, bezier_vertices, GL_STATIC_DRAW);		//��vertices���Ƶ���ǰ�󶨻��壬GL_STATIC_DRAW��ʾ���ݼ�������ı�

	/*---------------------------------------------����VAO----------------------------------------------------*/
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/*-------------------------------------------���Ӷ�������-------------------------------------------------*/
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);		//����1��Ҫ���õĶ������ԣ�����2���������ԵĴ�С
																						//����3�����ݵ����ͣ�����4������������5��ƫ����
	glEnableVertexAttribArray(0);		//���ö�������
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, 101);
	glDrawArrays(GL_LINE_STRIP, 0, 101);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void mouse_button_callback(GLFWwindow* window, int button, int action,int mods)
{
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			glfwGetCursorPos(window, &mouse_x,&mouse_y);
			float pointX = (mouse_x - SCR_WIDTH / 2) / (SCR_WIDTH / 2);
			float pointY = -(mouse_y - SCR_HEIGHT / 2) / (SCR_HEIGHT / 2);
			control_point.push_back(pointX);
			control_point.push_back(pointY);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && control_point.size() > 0)
		{
			control_point.pop_back();
			control_point.pop_back();
		}
	}
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
