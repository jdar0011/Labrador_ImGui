// Source: https://github.com/ZenSepiol/Dear-ImGui-App-Framework/blob/main/src/lib/app_base/app_base.hpp
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdio.h>
#include "util.h"
#include <filesystem>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture,
    int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory(
	    (const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(
    const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret
        = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadImageFromMemory(const void* data, size_t data_size, GLFWimage* out_image)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory(
        (const unsigned char*)data, (int)data_size, &out_image->width, &out_image->height, NULL, 4);
    if (image_data == NULL)
        return false;

    out_image->pixels = image_data;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadImageFromFile(
    const char* file_name, GLFWimage* out_texture)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret
        = LoadImageFromMemory(file_data, file_size, out_texture);
    IM_FREE(file_data);
    return ret;
}



static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

const int window_width = 1200;
const int window_height = 800;

template <typename Derived>
class AppBase
{
  public:
    AppBase()
    {
        glfwSetErrorCallback(ErrorCallback);

        if (!glfwInit())
            std::exit(1);

        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // Create window with graphics context

        window = glfwCreateWindow(window_width, window_height, "Labrador Controller", nullptr, nullptr);
        if (window == NULL)
            std::exit(1);

        // glfwSetWindowSize(window, 1920, 1080);

        // Load Icon
        GLFWimage icons[2];
        LoadImageFromFile(".\\misc\\media\\icon128.png", &icons[0]);
        LoadImageFromFile(".\\misc\\media\\icon64.png", &icons[1]);

        glfwSetWindowIcon(window, 2, icons);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Add window based callbacks to the underlying app
        glfwSetMouseButtonCallback(window, &Derived::MouseButtonCallback);
        glfwSetCursorPosCallback(window, &Derived::CursorPosCallback);
        glfwSetKeyCallback(window, &Derived::KeyCallback);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();

        // Setup Dear ImGui style
		ImGuiStyle& style = ImGui::GetStyle();
		style.ChildRounding = 5.0f;

        ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.8f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        // Add custom fonts
        ImGuiIO& io = ImGui::GetIO();
        const char* font_path_default;
        const char* font_path_special;
        font_path_default = "./misc/fonts/Roboto-Medium.ttf";
        font_path_special = "./misc/fonts/arial.ttf";
        ImFont* default_font = io.Fonts->AddFontFromFileTTF(font_path_default, 18.0f,nullptr,io.Fonts->GetGlyphRangesDefault());
        if (!default_font)
        {
#ifndef NDEBUG
			printf("Error loading default font");
#endif
        }
        ImFontConfig config;
		config.MergeMode = true;
		ImWchar arrow_ranges[] = { 0x2190, 0x2206, 0 };
        ImFont* arrow_font = io.Fonts->AddFontFromFileTTF(font_path_special, 24.0f,&config,arrow_ranges);
        ImFont* greek_font = io.Fonts->AddFontFromFileTTF(font_path_special, 18.0f, &config, io.Fonts->GetGlyphRangesGreek());
        //ImFont* arrow_font = io.Fonts->AddFontFromFileTTF("./misc/fonts/arial.ttf", 24.0f, nullptr, arrow_ranges);
        if (!arrow_font)
        {
#ifndef NDEBUG
			printf("Error loading arrow font");
#endif
        }

        io.Fonts->Build();
        // Load Images

    }

    virtual ~AppBase()
    {
        // Cleanup after 
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Run()
    {
        // Initialize the underlying app
        StartUp();

        while (!glfwWindowShouldClose(window))
        {
            // Poll events like key presses, mouse movements etc.
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Main loop of the underlying app

            Update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                         clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        ShutDown();
    }

    void Update()
    {
        static_cast<Derived*>(this)->Update();
    }

    void StartUp()
    {
        static_cast<Derived*>(this)->StartUp();
    }

    void ShutDown()
	{
		static_cast<Derived*>(this)->ShutDown();
	}

  private:
    GLFWwindow* window = nullptr;
    ImVec4 clear_color = ImVec4(0.1058, 0.1137f, 0.1255f, 1.00f);
};
