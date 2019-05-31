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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);			//告诉GLFW我们要使用的OpenGL的版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);		//告诉GLFW我们使用的是核心模式
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);	//参数说明：1、窗口的宽；2、窗口的高；3、窗口名称
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		//每当窗口调整大小时调用
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))		//初始化GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	/*---------------------------------------------编译着色器------------------------------------------------*/
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);		//创建着色器，参数为类型，顶点着色器
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);		//添加着色器源码
	glCompileShader(vertexShader);		//编译着色器
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);		//检查是否编译成功
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);		//获取错误信息
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/*--------------------------------------------片段着色器-------------------------------------------------*/
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	/*--------------------------------------------着色器程序-------------------------------------------------*/
	unsigned int shaderProgram = glCreateProgram();		//创建一个程序并返回ID引用
	glAttachShader(shaderProgram, vertexShader);		//将着色器附加到程序对象上
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}
	glDeleteShader(vertexShader);		//删除着色器对象 
	glDeleteShader(fragmentShader);

	while (!glfwWindowShouldClose(window))		//让GLFW退出前一直保持运行
	{
		unsigned int VAO, VBO;
		processInput(window); 
		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);		//设置清空屏幕所用的颜色
		glClear(GL_COLOR_BUFFER_BIT);		//清空颜色缓存，整个颜色缓冲被填充为glClearColor里所设置的颜色

		int length = control_point.size();
		float vertices[1000];
		for (int i = 0; i < length; ++i)
		{
			vertices[i] = control_point[i];
		}
		
		/*----------------------------------------------创建VBO--------------------------------------------------*/
		glGenBuffers(1, &VBO);		//创建缓冲对象，参数1：需要创建的缓存数量；参数2：储存缓冲ID的地址
		glBindBuffer(GL_ARRAY_BUFFER, VBO);		//将缓冲对象绑定到相应的缓冲上，参数1：缓冲类型（顶点数组数据or索引数组数据）
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * length, vertices, GL_STATIC_DRAW);		//将vertices复制到当前绑定缓冲，GL_STATIC_DRAW表示数据几乎不会改变

		/*---------------------------------------------创建VAO----------------------------------------------------*/
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		/*-------------------------------------------链接顶点属性-------------------------------------------------*/
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);		//参数1：要配置的顶点属性；参数2：顶点属性的大小
																							//参数3：数据的类型；参数4：步长；参数5：偏移量
		glEnableVertexAttribArray(0);		//启用顶点属性
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
		glfwSwapBuffers(window);		//交换颜色缓冲
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
	glfwTerminate();		//释放删除之前分配的所有资源
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
	/*----------------------------------------------创建VBO--------------------------------------------------*/
	glGenBuffers(1, &VBO);		//创建缓冲对象，参数1：需要创建的缓存数量；参数2：储存缓冲ID的地址
	glBindBuffer(GL_ARRAY_BUFFER, VBO);		//将缓冲对象绑定到相应的缓冲上，参数1：缓冲类型（顶点数组数据or索引数组数据）
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 202, bezier_vertices, GL_STATIC_DRAW);		//将vertices复制到当前绑定缓冲，GL_STATIC_DRAW表示数据几乎不会改变

	/*---------------------------------------------创建VAO----------------------------------------------------*/
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/*-------------------------------------------链接顶点属性-------------------------------------------------*/
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);		//参数1：要配置的顶点属性；参数2：顶点属性的大小
																						//参数3：数据的类型；参数4：步长；参数5：偏移量
	glEnableVertexAttribArray(0);		//启用顶点属性
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
