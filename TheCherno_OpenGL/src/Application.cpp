#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>


static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); //torna il puntatore al primo elemento di src
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //Error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        //funzione che alloca dinamicamente nello stack (invece che nell'heap)
        char* message = (char *) alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << 
            (type == GL_VERTEX_SHADER ? "vertex " : "fragment ") 
            << "shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);

        return 0;
    }

    return id;
}

/* Questa funzione linka i due shader in un'unica entità in modo da passarla ad OpenGL per la compilazione*/
//per semplicità scriviamo gli shader in delle stringhe (source code). Per farlo in modo più pulito e professionale conviene metterli in un file
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    //attach our shaders to our program
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    //una volta creato il programma possiamo eliminare gli shader
    glDeleteShader(vs);
    glDeleteShader(fs);

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

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Modern OpenGL -> creiamo un buffer che contiene i dati dei vertici */
    //Il buffer viene creato una volta sola, non serve crearlo ad ogni frame, faremo invece la draw ad ogni frame
    float positions[6] = {
        -0.5f, -0.5f,
         0.0f, 0.5f,
         0.5f, -0.5f
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); //qui diciamo alla state machine come interpretare il buffer che abbiamo creato
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    std::string vertexShader =
        R"glsl(#version 330 core
        
        layout(location = 0) in vec4 position;

        void main()
        {
            gl_Position = position;
        }  
        )glsl";

    /*Il vertex shader in questo caso setta semplicemente il colore dei pixel a rosso*/
    std::string fragmentShader =
        R"glsl(#version 330 core
        
        layout(location = 0) out vec4 color;

        void main()
        {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        }  
        )glsl";

    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader);

    /* Loop until the user closes the window */
    //questo é come un game loop -> viene eseguito ad ogni frame
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /*Legacy OpenGL*/
        /* glBegin(GL_TRIANGLES);
           glColor3f(0, 255, 255);
           glVertex2f(-0.5f, -0.5f);
           glVertex2f(0.0f, 0.5f);
           glVertex2f(0.5f, -0.5f);
           glEnd();                */

        glDrawArrays(GL_TRIANGLES, 0, 3);


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteShader(shader);
    glfwTerminate();

    return 0;
}