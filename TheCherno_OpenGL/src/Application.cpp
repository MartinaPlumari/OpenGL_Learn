#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLPrintError(GLenum code);

static void GLClearError() 
{
    while (glGetError() != GL_NO_ERROR); //si può scrivere anche come (!glGetError())

}

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        GLPrintError(error);
        std::cout << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) 
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) 
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else if(type != ShaderType::NONE)
        {
            ss[(int(type))] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };

}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); //torna il puntatore al primo elemento di src
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    //Error handling
    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        //funzione che alloca dinamicamente nello stack (invece che nell'heap)
        char* message = (char *) alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << 
            (type == GL_VERTEX_SHADER ? "vertex " : "fragment ") 
            << "shader!" << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));

        return 0;
    }

    return id;
}

/* Questa funzione linka i due shader in un'unica entità in modo da passarla ad OpenGL per la compilazione*/
//per semplicità scriviamo gli shader in delle stringhe (source code). Per farlo in modo più pulito e professionale conviene metterli in un file
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    //attach our shaders to our program
    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    //una volta creato il programma possiamo eliminare gli shader
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current -> creates a valid OpenGL rendering context */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Modern OpenGL -> creiamo un buffer che contiene i dati dei vertici */
    //Il buffer viene creato una volta sola, non serve crearlo ad ogni frame, faremo invece la draw ad ogni frame
    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f, 0.5f,
        -0.5f, 0.5f
    };

    unsigned int indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    //questo buffer contiene i vert)ici da renderizzare (vertex buffer)
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); //qui diciamo alla state machine come interpretare il buffer che abbiamo creato
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));
  
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

    //questo buffer contiene gli indici dei vertici (index buffer)
    unsigned int ibo; 
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)); //qui diciamo alla state machine come interpretare il buffer che abbiamo creato
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);

    float r = 0.0f;
    float increment = 0.01f;
    /* Loop until the user closes the window */
    //questo é come un game loop -> viene eseguito ad ogni frame
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        /*Legacy OpenGL*/
        /* glBegin(GL_TRIANGLES);
           glColor3f(0, 255, 255);
           glVertex2f(-0.5f, -0.5f);
           glVertex2f(0.0f, 0.5f);
           glVertex2f(0.5f, -0.5f);
           glEnd();                */

        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

        r += increment;

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;


        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}

static void GLPrintError(GLenum code)
{
    std::cout << "[OpenGL Error] (" << code << ")" << std::flush;
    switch (code)
    {
        case (GL_INVALID_ENUM):
            std::cout  << " Invalid ENUM" << std::endl;
            break;
        case (GL_INVALID_VALUE):
            std::cout << " Invalid value" << std::endl;
            break;
        case (GL_INVALID_OPERATION):
            std::cout << " Invalid operation" << std::endl;
            break;
        case (GL_INVALID_FRAMEBUFFER_OPERATION):
            std::cout << " Invalid frame buffer operation" << std::endl;
            break;
        case (GL_OUT_OF_MEMORY):
            std::cout << " Out of memory exception" << std::endl;
            break;
        case (GL_STACK_UNDERFLOW):
            std::cout << " Stack underflow" << std::endl;
            break;
        case (GL_STACK_OVERFLOW):
            std::cout << " Stack overflow" << std::endl;
            break;
        default:
            std::cout << " Unknown Error" << std::endl;
            break;
    }
}