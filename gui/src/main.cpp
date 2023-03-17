#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "raycaster.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int argc, char *argv[]) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        return 1;
    }

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
 
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Raytracer", 0, 0);
    if (window == 0) {
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    s32 cores = 8;
    s32 w = 1;
    s32 h = 1;
    s32 n = 1;

    Scene scene;
    scene.spheres = (Sphere **) malloc(n * sizeof(Sphere));
    scene.spheres[0] = make_sphere({0, 0, 1}, 1, make_matt(vec3(0.9, 0.4, 0.7)));
    scene.num_spheres = 1;

    Plane *floor = make_plane(0, make_matt(vec3(0.5)));
	scene.planes = &floor;
    scene.num_planes = 1;

    v3 cam_pos = { 0, 12, 5 };
    v3 look_at = { 0, 0, 1 };

    u32 *data = 0;

    bool show_config = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();

        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Image");

            w = ImGui::GetContentRegionMax().x;
            h = ImGui::GetContentRegionMax().y;

            ImGui::Image((void*)(intptr_t) texture, ImVec2((f32)w, (f32)h), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::End();
            ImGui::PopStyleVar();
        }

        if (show_config) { 
            ImGui::Begin("Config");

            for (s32 i = 0; i < scene.num_spheres; ++i) {
                Sphere *sp = scene.spheres[i];

                ImGui::Text("Sphere %d", i+1);
                ImGui::ColorEdit4("Albedo", (float*)&sp->material->albedo, ImGuiColorEditFlags_NoInputs);
                
                ImGui::RadioButton("Matt", &sp->material->kind, 0); ImGui::SameLine();
                ImGui::RadioButton("Metallic", &sp->material->kind, 1);
            }

            if (ImGui::Button("Update")) {

                scene.camera = make_camera(25, cam_pos, look_at, length(cam_pos - look_at), 0.15, w, h);

                free(data);
                data = raytrace(&scene, w, h, cores);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
