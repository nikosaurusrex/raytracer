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

    RayCastConfig config = ray_cast_config_default();
    config.cores = 8;
    u32 n = 10;

    Scene scene;
    scene.materials = (Material *) malloc((n+1) * sizeof(Material));
	scene.materials[0] = make_matt(vec3(0.5));

	for (u32 i = 1; i < n+1; ++i) {
		scene.materials[i] = make_matt(vec3(0.9, 0.4, 0.7));
	}

	scene.num_materials = 2;

    Plane floor = make_plane(0, 0);
	scene.planes = &floor;
    scene.num_planes = 1;

    scene.spheres = (Sphere *) malloc(n * sizeof(Sphere));
	for (u32 i = 0; i < n+0; ++i) {
		scene.spheres[i] = make_sphere({0, 0, 1}, 1, 1);
	}

    scene.num_spheres = 1;

    v3 cam_pos = { 0, 12, 5 };
    v3 look_at = { 0, 0, 1 };
    f32 fov = 25;
    f32 focus_dist = 10;
    f32 aperture = 0.15;

    u32 *data = 0;

    bool show_config = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Image");

            config.width = ImGui::GetContentRegionMax().x;
            config.height = ImGui::GetContentRegionMax().y;

            ImGui::Image((void*)(intptr_t) texture, ImVec2((f32)config.width, (f32)config.height), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::End();
            ImGui::PopStyleVar();
        }

		// ImGui::ShowDemoWindow();

        if (show_config) { 
            ImGui::Begin("Config");

			ImGui::SliderInt("Threads", (s32 *) &config.cores, 0, 20);
			ImGui::SliderInt("Spheres", (s32 *) &scene.num_spheres, 0, n);
			ImGui::SliderInt("Materials", (s32 *) &scene.num_materials, 0, n+1);
            ImGui::ColorEdit4("Sky Color", (float*)&config.sky_color, ImGuiColorEditFlags_NoInputs);
            ImGui::DragFloat3("Cam Pos", (f32 *) &cam_pos, 0.1f, -10.0f, 10.0f);
            ImGui::DragFloat3("Look At", (f32 *) &look_at, 0.1f, -10.0f, 10.0f);
            ImGui::DragFloat("FOV", &fov, 1.0f, 5.0f, 90.0f);
            ImGui::DragFloat("Focus Dist", &focus_dist, 1.0f, 5.0f, 40.0f);
            ImGui::DragFloat("Aperture", &aperture, 0.005f, 0.01f, 2.0f);
            ImGui::DragInt("Max Bounces", (s32 *) &config.max_bounces, 1.0f, 1, 20);
            ImGui::DragInt("Rpp", (s32 *) &config.rays_per_pixel, 2.0f, 16, 1024);

			for (s32 i = 0; i < scene.num_materials; ++i) {
				Material *mat = &scene.materials[i];

                ImGui::Text("Material %d", i);
                
				ImGui::PushID(i);

                ImGui::RadioButton("Matt", (s32 *) &mat->kind, 0); ImGui::SameLine();
                ImGui::RadioButton("Metallic", (s32 *) &mat->kind, 1);

                ImGui::ColorEdit4("Albedo", (float*)&mat->albedo, ImGuiColorEditFlags_NoInputs);

				ImGui::PopID();
			}

            for (s32 i = 0; i < scene.num_spheres; ++i) {
                Sphere *sp = &scene.spheres[i];

				ImGui::PushID(i);

                ImGui::Text("Sphere %d", i);

				ImGui::DragFloat3("Pos", (f32 *) &sp->center, 0.1f, -10.0f, 10.0f);
				ImGui::DragFloat("Radius", &sp->radius, 0.05f, 0.1f, 5.0f);
				ImGui::SliderInt("Material", (s32 *)&sp->material_index, 0, scene.num_materials - 1);

				ImGui::PopID();
            }

            if (ImGui::Button("Update")) {

                scene.camera = make_camera(fov, cam_pos, look_at, focus_dist, aperture, config.width, config.height);

                free(data);
                data = raytrace(&scene, &config);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, config.width, config.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }

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
