// Fluid Simulation (PTG)

#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLint initPlane(GLfloat, GLfloat, GLint, GLint, GLuint);
void drawPlane();

void loadSource(GLuint &shaderID, std::string name);
void printCompileInfoLog(GLuint shadID);
void printLinkInfoLog(GLuint programID);
void validateProgram(GLuint programID);

GLboolean init();

// Inicialización de las texturas y FBO
void initVelocityTexture();
void initPressureTexture();
void initInkTexture();
void initVorticityTexture();	
void initAuxTexture();
void initVelocityFBO();
void initPressureFBO();
void initInkFBO();
void initVorticityFBO();
void initAuxFBO();
void initAux2FBO();
// Inicialización de los programas
void initProgram();
void initAdvectionProgram();
void initJacobiProgram();
void initForceApplicationProgram();
void initProjectionDivergenceProgram();
void initProjectionGradientSubstractionProgram();
void initBoundaryConditionsProgram();
void initVorticityProgram();
void initGenVorticityProgram();
// Localización de las variables uniform
void locateUniforms();
// Función auxiliar de paso de texturas en FBO
void switchFBOs(GLuint FBO1, GLuint textureID);
//
void display();
void resize(GLint, GLint);
void idle();
void keyboard(GLubyte, GLint, GLint);
void specialKeyboard(GLint, GLint, GLint);
void mouse(GLint, GLint, GLint, GLint);
void mouseMotion(GLint, GLint);

void printFPS();

GLboolean fullscreen = false;
GLboolean mouseDown = false;
GLboolean animation = true;
 
GLfloat lastx = -40.0f;
GLfloat lasty = -40.0f;
GLfloat xdiff = 0.0f;
GLfloat ydiff = 0.0f;

GLint g_Width = 500;                          // Ancho inicial de la ventana
GLint g_Height = 500;                         // Altura incial de la ventana

GLuint planeVAOHandle;
GLint numVertPlane;

// Objetos programa
GLuint programID;
GLuint advectionProgramID, jacobiProgramID, forceApplicationProgramID,
projectionDivergenceProgramID, projectionGradientSubstractionProgramID, vorticityProgramID, genVorticityProgramID, boundaryConditionsProgramID;

// Variables uniform
GLuint locUniformTexelSize; // Tamaño de un texel para ajustar la escala
GLuint locUniformPressureMap, locUniformVelocityMap, locUniformColorMap, locUniformVorticityMap; // Texturas de los campos
GLuint locUniformTimestep; // Diferencial de tiempo
GLuint locUniformDissipation; // Factor de disipación
GLuint locUniformAlpha, locUniformBeta; // Variables para la difusión viscosa
GLuint locUniformAdvectedField, locUniformDiffusedField; // Campos a advectar y difundir
GLuint locUniformForce, locUniformXClick, locUniformYClick, locUniformRadius; // Fuerza aplicada
GLuint locUniformVorticityFactor; // Factor de vorticidad

// TEXTURAS y FBO //////////////////////////////////////////
GLuint velocityTexture, pressureTexture, inkTexture, vorticityTexture, auxTexture, aux2Texture;
GLuint velocityFBO, pressureFBO, inkFBO, vorticityFBO, auxFBO, aux2FBO;

// Variables globales ////////////////////////////
GLfloat timestep = 0.01f;
GLfloat texelSize = 1.0 / g_Width;
GLfloat dissipation = 0.998f;
GLfloat inkDissipation = 0.99f;
glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
GLfloat Xclick = -40.0f;
GLfloat Yclick = -40.0f;
//glm::vec3 inkColor = glm::vec3(0.435f, 0.996f, 1.0f);
glm::vec3 inkColor = glm::vec3(0.1256f,  0.2085f, 0.315f);  
GLfloat radius = 0.40f;
GLuint visMode = 0;
GLfloat vorticityFactor = 0.2f;
GLboolean applyVorticity = false;
GLboolean noColorInk = false;
GLboolean boundaryConditions = false;

// BEGIN: Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////
void loadSource(GLuint &shaderID, std::string name) 
{
	std::ifstream f(name.c_str());
	if (!f.is_open()) 
	{
		std::cerr << "File not found " << name.c_str() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// now read in the data
	std::string *source;
	source = new std::string( std::istreambuf_iterator<GLchar>(f),   
						std::istreambuf_iterator<GLchar>() );
	f.close();
   
	// add a null to the string
	*source += "\0";
	const GLchar * data = source->c_str();
	glShaderSource(shaderID, 1, &data, NULL);
	delete source;
	return;
}

void printCompileInfoLog(GLuint shadID) 
{
GLint compiled;
	glGetShaderiv( shadID, GL_COMPILE_STATUS, &compiled );
	if (compiled == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetShaderiv( shadID, GL_INFO_LOG_LENGTH, &infoLength );

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetShaderInfoLog( shadID, infoLength, &chsWritten, infoLog );

		std::cerr << "Shader compiling failed:" << infoLog << std::endl;
		system("pause");
		delete [] infoLog;

		exit(EXIT_FAILURE);
	}
	return;
}

void printLinkInfoLog(GLuint programID)
{
GLint linked;
	glGetProgramiv( programID, GL_LINK_STATUS, &linked );
	if(! linked)
	{
		GLint infoLength = 0;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLength );

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetProgramInfoLog( programID, infoLength, &chsWritten, infoLog );

		std::cerr << "Shader linking failed:" << infoLog << std::endl;
		system("pause");
		delete [] infoLog;

		exit(EXIT_FAILURE);
	}
	return;
}

void validateProgram(GLuint programID)
{
GLint status;
    glValidateProgram( programID );
    glGetProgramiv( programID, GL_VALIDATE_STATUS, &status );

    if( status == GL_FALSE ) 
	{
		GLint infoLength = 0;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLength );

        if( infoLength > 0 ) 
		{
			GLchar *infoLog = new GLchar[infoLength];
			GLint chsWritten = 0;
            glGetProgramInfoLog( programID, infoLength, &chsWritten, infoLog );
			std::cerr << "Program validating failed:" << infoLog << std::endl;
			system("pause");
            delete [] infoLog;

			exit(EXIT_FAILURE);
		}
    }
	return;
}

// END:   Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

GLint initPlane(GLfloat xsize, GLfloat zsize, GLint xdivs, GLint zdivs)
{
    
    GLfloat * v = new GLfloat[3 * (xdivs + 1) * (zdivs + 1)];
	GLfloat * n = new GLfloat[3 * (xdivs + 1) * (zdivs + 1)];
    GLfloat * tex = new GLfloat[2 * (xdivs + 1) * (zdivs + 1)];
    GLuint * el = new GLuint[6 * xdivs * zdivs];

    GLfloat x2 = xsize / 2.0f;
    GLfloat z2 = zsize / 2.0f;
    GLfloat iFactor = (GLfloat)zsize / zdivs;
    GLfloat jFactor = (GLfloat)xsize / xdivs;
    GLfloat texi = 1.0f / zdivs;
    GLfloat texj = 1.0f / xdivs;
    GLfloat x, z;
    GLint vidx = 0, tidx = 0;
    for( GLint i = 0; i <= zdivs; i++ ) {
        z = iFactor * i - z2;
        for( GLint j = 0; j <= xdivs; j++ ) {
            x = jFactor * j - x2;
            v[vidx] = x;
            v[vidx+1] = 0.0f;
            v[vidx+2] = z;
			n[vidx] = 0.0f;
			n[vidx+1] = 1.0f;
			n[vidx+2] = 0.0f;
            vidx += 3;
            tex[tidx] = j * texi;
            tex[tidx+1] = i * texj;
            tidx += 2;
        }
    }

    GLuint rowStart, nextRowStart;
    GLint idx = 0;
    for( GLint i = 0; i < zdivs; i++ ) {
        rowStart = i * (xdivs+1);
        nextRowStart = (i+1) * (xdivs+1);
        for( GLint j = 0; j < xdivs; j++ ) {
            el[idx] = rowStart + j;
            el[idx+1] = nextRowStart + j;
            el[idx+2] = nextRowStart + j + 1;
            el[idx+3] = rowStart + j;
            el[idx+4] = nextRowStart + j + 1;
            el[idx+5] = rowStart + j + 1;
            idx += 6;
        }
    }

    GLuint handle[4];
    glGenBuffers(4, handle);

	glGenVertexArrays( 1, &planeVAOHandle );
    glBindVertexArray(planeVAOHandle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs+1) * (zdivs+1) * sizeof(GLfloat), v, GL_STATIC_DRAW);
	GLuint loc1 = glGetAttribLocation(programID, "aPosition");   
    glVertexAttribPointer( loc1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(loc1);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs+1) * (zdivs+1) * sizeof(GLfloat), n, GL_STATIC_DRAW);
	GLuint loc2 = glGetAttribLocation(programID, "aNormal");   
    glVertexAttribPointer( loc2, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(loc2);  // Vertex normal

    glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
    glBufferData(GL_ARRAY_BUFFER, 2 * (xdivs+1) * (zdivs+1) * sizeof(GLfloat), tex, GL_STATIC_DRAW);
	GLuint loc3 = glGetAttribLocation(programID, "aTexCoord");   
    glVertexAttribPointer( loc3, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
    glEnableVertexAttribArray(loc3);  // texture coords

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * xdivs * zdivs * sizeof(GLuint), el, GL_STATIC_DRAW);

    glBindVertexArray(0);
    
    delete [] v;
	delete [] n;
    delete [] tex;
    delete [] el;

	return 6 * xdivs * zdivs;
}

// END: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

void drawPlane() {
    glBindVertexArray(planeVAOHandle);
    glDrawElements(GL_TRIANGLES, numVertPlane, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
	return;
}

// END: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

// FUNCIONES DE INICIALIZACIÓN DE TEXTURAS y FBO //////////
void generateRandomColorData(int width, int height, float* colorData) {
    srand(static_cast<unsigned int>(time(nullptr))); // Inicializar semilla aleatoria

    for (int i = 0; i < width * height * 4; i += 4) {
        // float r = static_cast<float>(rand()) / RAND_MAX; // Componente rojo (0.0 - 1.0)
        // float g = static_cast<float>(rand()) / RAND_MAX; // Componente verde (0.0 - 1.0)
        // float b = static_cast<float>(rand()) / RAND_MAX; // Componente azul (0.0 - 1.0)
		float r = 0.2;
		float g = 0.2;
		float b = 0.2;
        float a = 1.0f; // Alfa (opacidad)

        colorData[i] = r;
        colorData[i + 1] = g;
        colorData[i + 2] = b;
        colorData[i + 3] = a;
    }
}

void initVelocityFBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &velocityFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, velocityFBO);

	// Se genera la textura
	glGenTextures(1, &velocityTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, velocityTexture);

	float* colorData = new float[g_Width * g_Height * 4];
    generateRandomColorData(g_Width, g_Height, colorData);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, colorData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	delete[] colorData;

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}

void initPressureFBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &pressureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pressureFBO);

	// Se genera la textura
	glGenTextures(1, &pressureTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pressureTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pressureTexture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}

void initInkFBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &inkFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, inkFBO);

	// Se genera la textura
	glGenTextures(1, &inkTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, inkTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inkTexture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}

void initVorticityFBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &vorticityFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, vorticityFBO);

	// Se genera la textura
	glGenTextures(1, &vorticityTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, vorticityTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vorticityTexture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}

void initAuxFBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &auxFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);

	// Se genera la textura
	glGenTextures(1, &auxTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, auxTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, auxTexture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}

void initAux2FBO()
{
	// Se genera el FBO
	glGenFramebuffers(1, &aux2FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, aux2FBO);

	// Se genera la textura
	glGenTextures(1, &aux2Texture);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, aux2Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, g_Width, g_Height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Se asocia la textura al FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aux2Texture, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( result == GL_FRAMEBUFFER_COMPLETE )
		std::cout << "Frame buffer complete" << std::endl;
	else
		std::cout << "Frame buffer is not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}


// LOCALIZACIÓN DE LAS VARIABLES UNIFORM //////////////////////////////////////////
void locateUniforms()
{
	locUniformTexelSize = 0;
	locUniformVelocityMap = 1;
	locUniformPressureMap = 2;
	locUniformColorMap = 3;
	locUniformVorticityMap = 4;
	locUniformAdvectedField = 5;
	locUniformDiffusedField = 6;
	locUniformTimestep = 7;
	locUniformDissipation = 8;
	locUniformAlpha = 9;
	locUniformBeta = 10;
	locUniformXClick = 11;
	locUniformYClick = 12;
	locUniformForce = 13;
	locUniformRadius = 14;
	locUniformVorticityFactor = 15;

	return;
}


// MAIN //////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	glutInit(&argc, argv); 
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Simulacion de Fluidos. Noe Fernandez");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	  system("pause");
	  exit(-1);
	}
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);
 
	glutMainLoop();
 
	return EXIT_SUCCESS;
}


GLboolean init()
{
	glClearColor(0.43f, 0.43f, 0.43f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

	// Inicialización de los programas
	initProgram();
	initAdvectionProgram();
	initJacobiProgram();
	initForceApplicationProgram();
	initProjectionDivergenceProgram();
	initProjectionGradientSubstractionProgram();
	initVorticityProgram();
	initGenVorticityProgram();
	initBoundaryConditionsProgram();

	numVertPlane = initPlane(2.0f, 2.0f, 1, 1);

	// Localización de las variables uniform
	locateUniforms();

	// Inicialización de las texturas y FBO
	initVelocityFBO();
	initPressureFBO();
	initInkFBO();
	initVorticityFBO();
	initAuxFBO();
	initAux2FBO();

	return true;
}


// Función auxiliar para pasar texturas de un FBO a otro
void switchFBOs(GLuint destinyFBO, GLuint textureID)
{
	glBindFramebuffer(GL_FRAMEBUFFER, destinyFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(programID);
	glUniform1i(locUniformColorMap, textureID);
	drawPlane();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return;
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pasos de la simulación
	// Advección de la velocidad
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(advectionProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 0);
	glUniform1i(locUniformAdvectedField, 0);
	glUniform1f(locUniformTimestep, timestep);
	glUniform1f(locUniformDissipation, dissipation);
	drawPlane();
	switchFBOs(velocityFBO, 4);

	// Advección de la tinta
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(advectionProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 0);
	glUniform1i(locUniformAdvectedField, 2);
	glUniform1f(locUniformTimestep, timestep);
	glUniform1f(locUniformDissipation, inkDissipation);
	drawPlane();
	switchFBOs(inkFBO, 4);

	if(applyVorticity)
	{
		// Generación de la voriticidad
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(genVorticityProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformVelocityMap, 0);
		drawPlane();
		switchFBOs(aux2FBO, 4);

		// Voriticidad
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(vorticityProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformVelocityMap, 0);
		glUniform1i(locUniformVorticityMap, 5);
		glUniform1f(locUniformVorticityFactor, vorticityFactor);
		glUniform1f(locUniformTimestep, timestep);
		drawPlane();
		switchFBOs(velocityFBO, 4);
	}

	// Aplicación de fuerzas
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(forceApplicationProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 0);
	glUniform1f(locUniformTimestep, timestep);
	glUniform3f(locUniformForce, (force.x * 2.0) * texelSize, (-force.y * 2.0) * texelSize, 0.1f);
	glUniform1f(locUniformXClick, Xclick * texelSize);
	glUniform1f(locUniformYClick, (g_Height - Yclick) * texelSize);
	glUniform1f(locUniformRadius, radius * texelSize);
	drawPlane();
	switchFBOs(velocityFBO, 4);

	// Aplicación de fuerzas a la tinta
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(forceApplicationProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 2);
	glUniform1f(locUniformTimestep, timestep);
	if(noColorInk)
		glUniform3f(locUniformForce, inkColor.x, inkColor.x, inkColor.x);
	else
		glUniform3f(locUniformForce, inkColor.x, inkColor.y, inkColor.z);
	glUniform1f(locUniformXClick, Xclick * texelSize);
	glUniform1f(locUniformYClick, (g_Height - Yclick) * texelSize);
	glUniform1f(locUniformRadius, radius * texelSize);
	drawPlane();
	switchFBOs(inkFBO, 4);

	// Difusión viscosa de la velocidad
	float alpha = (texelSize * texelSize) / (0.001 * timestep);
    float beta = alpha + 4.0f;

	for(int i = 0; i < 10; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(jacobiProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformVelocityMap, 0);
		glUniform1i(locUniformDiffusedField, 0);
		glUniform1f(locUniformAlpha, alpha);
		glUniform1f(locUniformBeta, beta);
		drawPlane();
		switchFBOs(velocityFBO, 4);
	}

	// Difusión viscosa de la tinta
	alpha = (texelSize * texelSize) / (0.00001 * timestep);
    beta = alpha + 4.0f;

	for(int i = 0; i < 10; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(jacobiProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformVelocityMap, 2);
		glUniform1i(locUniformDiffusedField, 2);
		glUniform1f(locUniformAlpha, alpha);
		glUniform1f(locUniformBeta, beta);
		drawPlane();
		switchFBOs(inkFBO, 4);
	}

	// Proyección 
	// Divergencia
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(projectionDivergenceProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 0);
	drawPlane();
	switchFBOs(aux2FBO, 4);

	alpha = -(texelSize * texelSize);
	beta = 4.0f;
	// Iteración de Jacobi para la presión
	for(int i = 0; i < 30; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(jacobiProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformVelocityMap, 1);
		glUniform1i(locUniformDiffusedField, 5);
		glUniform1f(locUniformAlpha, alpha);
		glUniform1f(locUniformBeta, beta);
		drawPlane();
		switchFBOs(pressureFBO, 4);
	}

	// Resta del Gradiente
	glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_Width, g_Height);
	glUseProgram(projectionGradientSubstractionProgramID);
	glUniform1f(locUniformTexelSize, texelSize);
	glUniform1i(locUniformVelocityMap, 0);
	glUniform1i(locUniformPressureMap, 1);
	drawPlane();
	switchFBOs(velocityFBO, 4);

	// Aplicación de las condiciones de contorno
	if(boundaryConditions)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(boundaryConditionsProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformColorMap, 0);
		drawPlane();
		switchFBOs(velocityFBO, 4);

		glBindFramebuffer(GL_FRAMEBUFFER, auxFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_Width, g_Height);
		glUseProgram(boundaryConditionsProgramID);
		glUniform1f(locUniformTexelSize, texelSize);
		glUniform1i(locUniformColorMap, 1);
		drawPlane();
		switchFBOs(pressureFBO, 4);
	}

	if(visMode == 1)
	{
		// Dibujado final (Velocity)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);
		glUniform1i(locUniformTexelSize, texelSize);	
		glUniform1i(locUniformColorMap, 0);
		drawPlane();
	}
	if(visMode == 0)
	{
		// Dibujado final (Ink)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);
		glUniform1i(locUniformTexelSize, texelSize);	
		glUniform1i(locUniformColorMap, 2);
		drawPlane();
	}

	glUseProgram(0);
	printFPS();
	glutSwapBuffers();
	return;
}
 
void resize(GLint w, GLint h)
{
	// g_Width = w;
	// g_Height = h;
	glViewport(0, 0, g_Width, g_Height);
	return;
}
 
void idle()
{
	glutPostRedisplay();
	return;
}
 
void keyboard(GLubyte key, GLint x, GLint y)
{
	switch(key)
	{
	case 27 : case 'q': case 'Q':
		exit(EXIT_SUCCESS); 
		break;
	case 'i': case 'I':
		dissipation += 0.001f;
		if(dissipation > 1.0f)
			dissipation = 1.0f;
		std::cout << "Dissipation: " << dissipation << std::endl;
		break;
	case 'k': case 'K':	
		dissipation -= 0.001f;
		if(dissipation < 0.0f)
			dissipation = 0.0f;
		std::cout << "Dissipation: " << dissipation << std::endl;
		break;
	case '+':	
		inkDissipation += 0.001f;
		if(inkDissipation > 1.0f)
			inkDissipation = 1.0f;
		std::cout << "Ink Dissipation: " << inkDissipation << std::endl;
		break;
	case '-':
		inkDissipation -= 0.001f;
		if(inkDissipation < 0.0f)
			inkDissipation = 0.0f;
		std::cout << "Ink Dissipation: " << inkDissipation << std::endl;
		break;	
	case 'w': case 'W':
		vorticityFactor += 0.01f;
		if(vorticityFactor > 0.35f)
			vorticityFactor = 0.35f;
		std::cout << "Vorticity: " << vorticityFactor << std::endl;
		break;
	case 's': case 'S':
		vorticityFactor -= 0.01f;
		if(vorticityFactor < 0.0f)
			vorticityFactor = 0.0f;	
		std::cout << "Vorticity: " << vorticityFactor << std::endl;
		break;
	case 'm': case 'M':
		visMode = (visMode + 1) % 2;
		break;
	case 'c': case 'C':
		inkColor.x = static_cast<float>(rand()) / RAND_MAX;
		inkColor.y = static_cast<float>(rand()) / RAND_MAX;
		inkColor.z = static_cast<float>(rand()) / RAND_MAX;
		std::cout << "Ink color: " << inkColor.x << " " << inkColor.y << " " << inkColor.z << std::endl;
		break;
	case 'v': case 'V':
		applyVorticity = !applyVorticity;
		break;
	case 'b': case 'B':
		noColorInk = !noColorInk;
		break;
	case 'p': case 'P':
		boundaryConditions = !boundaryConditions;
		break;
	}
	return;
}
 
void specialKeyboard(GLint key, GLint x, GLint y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;
 
		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(g_Width, g_Height);
			glutPositionWindow(50, 50);
		}
	}
	return;
}
 
void mouse(GLint button, GLint state, GLint x, GLint y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;
		lastx = x;
		lasty = y;		
	}
	else
	{
		mouseDown = false;
		Xclick = -40.0f;
		Yclick = -40.0f;
	}
	return;
}
 
void mouseMotion(GLint x, GLint y)
{
	if (mouseDown)
	{
			Xclick = x;
			Yclick = y;
			force.x = x - lastx;
			force.y = y - lasty;
			glutPostRedisplay();
	}
	else
	{
		force.x = 0.0f;
		force.y = 0.0f;
		Xclick = -40.0f;
		Yclick = -40.0f;
	}
	return;
}

void printFPS()
{
	static GLint frameCount = 0;			//  Número de frames
	static GLfloat fps = 0;
	static GLint currentTime = 0, previousTime = 0;

	frameCount++;

	currentTime = glutGet(GLUT_ELAPSED_TIME);

	GLint timeInterval = currentTime - previousTime;

	if (timeInterval > 1000)
	{
		fps = frameCount / (timeInterval / 1000.0f);

		previousTime = currentTime;

		frameCount = 0;

		std::cout.precision(4);
		std::cout << "FPS: " << fps << std::endl;
	}
	return;
}

// FUNCIONES DE INICIALIZACIÓN DE OBJETOS PROGRAMA //////////////////////////////////////////////////////////////////
void initProgram()
{
	programID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(programID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/fragment.frag");
	std::cout << "Compiling fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(programID, fragmentShaderID);

	glLinkProgram(programID);
	printLinkInfoLog(programID);
	validateProgram(programID);

	return;

}

void initAdvectionProgram()
{
	advectionProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(advectionProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/advection.frag");
	std::cout << "Compiling advection fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(advectionProgramID, fragmentShaderID);

	glLinkProgram(advectionProgramID);
	printLinkInfoLog(advectionProgramID);
	validateProgram(advectionProgramID);

	return;
}

void initJacobiProgram()
{
	jacobiProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(jacobiProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/jacobi.frag");
	std::cout << "Compiling jacobi iterative fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(jacobiProgramID, fragmentShaderID);

	glLinkProgram(jacobiProgramID);
	printLinkInfoLog(jacobiProgramID);
	validateProgram(jacobiProgramID);

	return;
}

void initForceApplicationProgram()
{
	forceApplicationProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(forceApplicationProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/force_application.frag");
	std::cout << "Compiling force application fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(forceApplicationProgramID, fragmentShaderID);

	glLinkProgram(forceApplicationProgramID);
	printLinkInfoLog(forceApplicationProgramID);
	validateProgram(forceApplicationProgramID);

	return;
}

void initProjectionDivergenceProgram()
{
	projectionDivergenceProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(projectionDivergenceProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/divergence.frag");
	std::cout << "Compiling projection divergence fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(projectionDivergenceProgramID, fragmentShaderID);

	glLinkProgram(projectionDivergenceProgramID);
	printLinkInfoLog(projectionDivergenceProgramID);
	validateProgram(projectionDivergenceProgramID);

	return;
}

void initProjectionGradientSubstractionProgram()
{
	projectionGradientSubstractionProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(projectionGradientSubstractionProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/gradient_substraction.frag");
	std::cout << "Compiling projection gradient substraction fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(projectionGradientSubstractionProgramID, fragmentShaderID);

	glLinkProgram(projectionGradientSubstractionProgramID);
	printLinkInfoLog(projectionGradientSubstractionProgramID);
	validateProgram(projectionGradientSubstractionProgramID);

	return;
}

void initVorticityProgram()
{
	vorticityProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(vorticityProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/vorticity.frag");
	std::cout << "Compiling vorticity fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(vorticityProgramID, fragmentShaderID);

	glLinkProgram(vorticityProgramID);
	printLinkInfoLog(vorticityProgramID);
	validateProgram(vorticityProgramID);

	return;
}

void initGenVorticityProgram()
{
	genVorticityProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(genVorticityProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/gen_vorticity.frag");
	std::cout << "Compiling gen vorticity fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(genVorticityProgramID, fragmentShaderID);

	glLinkProgram(genVorticityProgramID);
	printLinkInfoLog(genVorticityProgramID);
	validateProgram(genVorticityProgramID);

	return;

}

void initBoundaryConditionsProgram()
{
	boundaryConditionsProgramID = glCreateProgram();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "../shaders/vertex.vert");
	std::cout << "Compiling vertex shader ..." << std::endl;
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(boundaryConditionsProgramID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "../shaders/boundary_conditions.frag");
	std::cout << "Compiling boundary conditions fragment shader ..." << std::endl;
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(boundaryConditionsProgramID, fragmentShaderID);

	glLinkProgram(boundaryConditionsProgramID);
	printLinkInfoLog(boundaryConditionsProgramID);
	validateProgram(boundaryConditionsProgramID);

	return;
}
