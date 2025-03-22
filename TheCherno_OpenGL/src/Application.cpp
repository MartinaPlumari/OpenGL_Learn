#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

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

    glfwTerminate();
    return 0;
}