#include "chip8.h"
#include <stdio.h>
#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ch8Emulator{

	Chip8 ch8;//ch8 cpu object global to namespace

	const GLchar* vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 position;\n"
		"uniform mat4 transform;\n"
		"void main(){gl_Position = transform * vec4(position.x, position.y, position.z, 1.0);}\n";

	const GLchar* fragmentShaderSource =
		"#version 330 core\n"
		"out vec4 color;\n"
		"void main(){ color = vec4(1.0f, 1.0f, 1.0f, 1.0f);}\n";

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (action == GLFW_PRESS)
		{
			int hexKey = Chip8::getKeymap(key);
			if (hexKey != -1)
			{
				ch8.key[hexKey] = 1;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			int hexKey = Chip8::getKeymap(key);
			if (hexKey != -1)
			{
				ch8.key[hexKey] = 0;
			}
		}
	};

	int main(int argc, char* argv[])
	{
		//Initialize glfw
		glfwInit();
		//tell glfw to use OpenGL 3.3
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//throws invalid operation error if legacy OpenGL function is used
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);//don't let user resize window

		//create window object
		GLFWwindow* window = glfwCreateWindow(640, 320, "ChipGr8", nullptr, nullptr);
		glfwMakeContextCurrent(window);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window." << std::endl;
			glfwTerminate();
			return -1;
		}
		glfwSetKeyCallback(window, key_callback);//set key callback for input handling

		//Initialize GLEW
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW!" << std::endl;
			return -1;
		}

		glViewport(0, 0, 640, 320);//create viewport

		GLfloat vertices[] = //6 vertices making two triangles that make up one square
		{
			-0.5f, 0.5f, 0.0f,//top left 
			-0.5f, -0.5f, 0.0f,//bottom left 
			0.5f, -0.5f, 0.0f,//bottom right 
			0.5f, 0.5f, 0.0f//top right 

		};
		
		GLuint indices[] =
		{
			0, 1, 2, //first triangle
			0, 2, 3  //second triangle
		};

		glm::mat4 trans;
		
		//set up vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);//compile vertex shader
		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//check if vertex shader compiled
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		//set up fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//check if fragment shader compiled
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		//set up shader program
		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);//attach shaders to shaderProgram
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);//check if shader program linked
		if (!success)
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		else
		{
			glUseProgram(shaderProgram);
		}
		
		//once linked to shaderProgram, no longer need shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		


		//set up Vertex Array Object
		GLuint VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		//set up Vertex Buffer
		GLuint VBO;
		glGenBuffers(1, &VBO);
		//copy vertices array in a buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		//create EBO
		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		//tell OpenGL how to interpret vertex data, array of 3 GLfloats, starting at position 0
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);//location 0 as set in vertex shader
		glBindVertexArray(0);

		std::ostringstream os;
		os << "test osstream" << std::endl;
		std::cout << os.str();

		//Initialize chip8
		ch8.initialize(&os);
		//load game
		try
		{
			ch8.loadGame("D:\\My Documents\\c8games\\PONG2");
		}
		catch (char* e)
		{
			printf("%s\n", e);//print error
			glfwTerminate();//clean up
			return -1;//exit
		}
		//game loop
		while (!glfwWindowShouldClose(window))
		{
			
			if (ch8.getOpcode(ch8.getPc()) == 0)//exit for opcode of 0x0000/empty memory
			{
				printf("End of Program\n");
				break;
			}
			glfwPollEvents();//poll for input
			
			try
			{
				for (int i = 0; i < 3000000; i++){}//attempt to slow down emulation to match proper speed
				ch8.emulateCycle();//emulate one chip8 cpu cycle
			}
			catch (char* e)
			{
				printf("%s\n", e);//print error
				break;//exit while loop for clean up and exiting
			}
			if (ch8.drawFlag)//check if screen needs to be redrawn
			{
				

				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//set clear color to black
				glClear(GL_COLOR_BUFFER_BIT);//clear screen

				glUseProgram(shaderProgram);//use shader program
				glBindVertexArray(VAO);
				
				for (int i = 0; i < 32; i++)//iterate through pixels to draw
				{
					for (int j = 0; j < 64; j++)
					{
						if (ch8.gfx[j + 64 * i] == 1)//need to draw pixel
						{
							trans = glm::scale(glm::mat4(1.0f), glm::vec3(0.025, -0.045, 0.0));//scale block down to 1 "pixel" and flip y
							trans = glm::translate(trans, glm::vec3(j - 32 , i - 16, 0.0));//move pixel to correct location

							GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
							glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

							glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//draw rectangle(2 triangles)
						}
					}
				}
				glBindVertexArray(0);//unbind VAO


				glfwSwapBuffers(window);//swap buffers to draw screen
				

				ch8.drawFlag = false;//reset drawFlag
			}
		}
		ch8.~Chip8();//clean up ch8
		glfwTerminate();//clean up graphics
		return 0;

	};

	

	//runs game without use of opengl, for debug purposes, no input
	void debugRun(Chip8 ch8)
	{
		dByte opcode = ch8.getOpcode(ch8.getPc());//get current opcode
		printf("0x%X  0x%X", ch8.getPc(), opcode);
		while (opcode != 0x0000)//loop through opcodes until hit empty memory
		{
			ch8.emulateCycle();//emulate one cpu cycle
			if (ch8.drawFlag)//check if need to redraw screen
			{
				ch8.debugPrint();//draw
				ch8.drawFlag = false;//reset drawFlag

			}
			opcode = ch8.getOpcode(ch8.getPc());

		}
	};
}