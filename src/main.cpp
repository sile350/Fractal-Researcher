#include <SDL.h>
#include <glad/glad.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "Shader.h"

int WINDOW_WIDTH = 1200;
int WINDOW_HEIGHT = 800;

enum State
{
    MOND,
    JUL
};

int main(int argc, char* argv[])
{
    //Init SDL
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "Fractal Explorer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    Shader shaderMond("src/sh.vert", "src/mond.frag");
    Shader shaderJul("src/sh.vert", "src/jul.frag");

    float vertices[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    unsigned int indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float zoom = 2.0f;
    float offsetX = -0.5f;
    float offsetY = 0.0f;
    int maxIterations = 100;
    float juliaCX = -0.4f;
    float juliaCY = 0.6f;
    bool isDragging = false;
    float dragStartX, dragStartY;

    State fractals = State::JUL;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                WINDOW_WIDTH = event.window.data1;
                WINDOW_HEIGHT = event.window.data2;
                glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && !ImGui::GetIO().WantCaptureMouse) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    isDragging = true;
                    dragStartX = event.button.x;
                    dragStartY = event.button.y;
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                }
            }
            if (event.type == SDL_MOUSEMOTION && isDragging) {
                float dx = (event.motion.x - dragStartX) * zoom / WINDOW_HEIGHT;
                float dy = (event.motion.y - dragStartY) * zoom / WINDOW_HEIGHT;
                offsetX -= dx;
                offsetY += dy;
                dragStartX = event.motion.x;
                dragStartY = event.motion.y;
            }
            if (event.type == SDL_MOUSEWHEEL && !ImGui::GetIO().WantCaptureMouse) {
                float zoomFactor = 1.0f + event.wheel.y * 0.1f;
                zoom *= zoomFactor;
            }
        }

        // Рендеринг
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (fractals == State::MOND)
        {
            glUseProgram(shaderMond.Program);
            glUniform2f(glGetUniformLocation(shaderMond.Program, "resolution"), WINDOW_WIDTH, WINDOW_HEIGHT);
            glUniform2f(glGetUniformLocation(shaderMond.Program, "offset"), offsetX, offsetY);
            glUniform1f(glGetUniformLocation(shaderMond.Program, "zoom"), zoom);
            glUniform1i(glGetUniformLocation(shaderMond.Program, "maxIterations"), maxIterations);
        }
        else if (fractals == State::JUL)
        {
            glUseProgram(shaderJul.Program);
            glUniform2f(glGetUniformLocation(shaderJul.Program, "resolution"), WINDOW_WIDTH, WINDOW_HEIGHT);
            glUniform2f(glGetUniformLocation(shaderJul.Program, "offset"), offsetX, offsetY);
            glUniform1f(glGetUniformLocation(shaderJul.Program, "zoom"), zoom);
            glUniform1i(glGetUniformLocation(shaderJul.Program, "maxIterations"), maxIterations);
            glUniform2f(glGetUniformLocation(shaderJul.Program, "juliaC"), juliaCX, juliaCY);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // ImGui рендеринг
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Fractal Controls");
        if (ImGui::Button("Mondelbort")) {
            fractals = State::MOND;
        }
        if (ImGui::Button("Julia")) {
            fractals = State::JUL;
        }
        ImGui::SliderInt("Max Iterations", &maxIterations, 10, 1000);
        if (fractals == State::JUL)
        {
            ImGui::SliderFloat("juliaC.x", &juliaCX, -10.0f, 10.0f);
            ImGui::SliderFloat("juliaC.y", &juliaCY, -10.0f, 10.0f);
        }
        ImGui::SliderFloat("Zoom", &zoom, 0.1f, 10.0f);
        ImGui::SliderFloat("Offset X", &offsetX, -2.0f, 2.0f);
        ImGui::SliderFloat("Offset Y", &offsetY, -2.0f, 2.0f);
        if (ImGui::Button("Reset")) {
            zoom = 2.0f;
            offsetX = -0.5f;
            offsetY = 0.0f;
            maxIterations = 100;
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderMond.Program);
    glDeleteProgram(shaderJul.Program);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}