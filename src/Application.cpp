#include "Application.h"
#include "utils.h"
#include "Cloth.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <utility>

Application::Application(std::string title, int w, int h) : app_title{std::move(title)}, window_width{w}, window_height{h} {

}

Application::~Application() {
    glDeleteVertexArrays(1, &sphere_vao);
    glDeleteBuffers(1, &sphere_vbo_position);
    glDeleteBuffers(1, &sphere_vbo_normal);
    glDeleteBuffers(1, &sphere_ibo);
    glDeleteBuffers(1, &axis_line_vao);
    glDeleteBuffers(1, &axis_line_vbo);
    glDeleteBuffers(1, &sphere_vao2);
    glDeleteBuffers(1, &sphere_vbo_position2);
    glDeleteBuffers(1, &grid_vao);
    glDeleteBuffers(1, &grid_vbo);
    glDeleteBuffers(1, &grid_ibo);
    glDeleteProgram(cloth_shader);
    glDeleteProgram(axis_shader);
    glDeleteProgram(grid_shader);
    if (cloth) {
        delete cloth;
        cloth = nullptr;
    }
}

bool Application::initApp() {
    if (!glfwInit()) {
        error("glfw init error");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(window_width, window_height, app_title.c_str(), nullptr, nullptr);
    if (!window) {
        error("glfw window init error");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwPollEvents();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        error("glew init error");
        glfwTerminate();
        return false;
    }

    if (!init()) {
        error("init failed");
        glfwTerminate();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return init();
}

bool Application::init() {
    cloth_shader = load_shader_from_file("../shaders/cloth_vs.glsl", "../shaders/cloth_fs.glsl");
    if (!cloth_shader)
        return false;
    axis_shader = load_shader_from_file("../shaders/axis_vs.glsl", "../shaders/axis_fs.glsl");
    if (!axis_shader)
        return false;
    grid_shader = load_shader_from_file("../shaders/grid_vs.glsl", "../shaders/grid_fs.glsl");
    if (!grid_shader)
        return false;

    const auto& [positions, normals, indices] = generate_ico_sphere(3);
    sphere_draw_call_count = indices.size();

    glGenVertexArrays(1, &sphere_vao);
    glBindVertexArray(sphere_vao);

    glGenBuffers(1, &sphere_vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_position);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &(positions.front()), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glGenBuffers(1, &sphere_vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_normal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &(normals.front()), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glGenBuffers(1, &sphere_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &(indices.front()), GL_STATIC_DRAW);

    glBindVertexArray(NULL);

    const auto& positions2 = generate_uv_sphere(sphere_radius, 20, 20);
    sphere_draw_call_count2 = positions2.size();

    glGenVertexArrays(1, &sphere_vao2);
    glBindVertexArray(sphere_vao2);

    glGenBuffers(1, &sphere_vbo_position2);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_position2);
    glBufferData(GL_ARRAY_BUFFER, positions2.size() * sizeof(glm::vec3), &(positions2.front()), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glBindVertexArray(NULL);

    const glm::vec3 vertices[] = {
            glm::vec3(0, 0, 0),
            glm::vec3(1, 0, 0), // color
            glm::vec3(1, 0, 0),
            glm::vec3(1, 0, 0), // color

            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0), // color
            glm::vec3(0, 1, 0),
            glm::vec3(0, 1, 0), // color

            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 1), // color
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 1), // color
    };

    glGenVertexArrays(1, &axis_line_vao);
    glBindVertexArray(axis_line_vao);

    glGenBuffers(1, &axis_line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axis_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void *)(sizeof(glm::vec3)));

    glBindVertexArray(NULL);

    std::vector<glm::vec3> vertices_grid{};
    unsigned int grid_scale = 100;
    for (int y = 0; y <= grid_scale; ++y) {
        for (int x = 0; x <= grid_scale; ++x) {
            vertices_grid.emplace_back(glm::vec3((float)x/(float)grid_scale, 0, (float)y/(float)grid_scale));
        }
    }
    std::vector<glm::uvec4> indices_grid{};
    for(int j=0; j < grid_scale; ++j) {
        for(int i=0; i < grid_scale; ++i) {
            int row1 =  j * (grid_scale+1);
            int row2 = (j+1) * (grid_scale+1);

            indices_grid.emplace_back(glm::uvec4(row1+i, row1+i+1, row1+i+1, row2+i+1));
            indices_grid.emplace_back(glm::uvec4(row2+i+1, row2+i, row2+i, row1+i));
        }
    }

    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_grid.size()*sizeof(*vertices_grid.data()), vertices_grid.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glGenBuffers(1, &grid_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_grid.size()*sizeof(*indices_grid.data()), indices_grid.data(), GL_STATIC_DRAW);

    glBindVertexArray(NULL);

    grid_draw_call_count = indices_grid.size() * 4;

    cloth = new Cloth(55, 45);

    return true;
}

int Application::loop() {
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float fixed_timestamp = 0.25f;
        update(0.f);
        fixedUpdate(fixed_timestamp);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}

void Application::fixedUpdate(float dt) {
    cloth->add_wind_force(wind_dir);
    cloth->update(dt);
    cloth->collision_detection_with_sphere(sphere_pos, sphere_radius);
}

void Application::update(float dt) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) viewPos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) viewPos += -forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) viewPos += -right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) viewPos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) viewPos += up * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) viewPos += -up * speed;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) is_wireframe = !is_wireframe;
    if (is_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) sphere_pos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) sphere_pos += -forward * speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) sphere_pos += -right * speed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) sphere_pos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) sphere_pos += up * speed;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) sphere_pos += -up * speed;
}

void Application::render() {
    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::translate(model, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 view = glm::lookAt(viewPos, viewPos + forward, up);
    glm::mat4 perspective = glm::perspective(fieldOfView, (float)window_width / (float)window_height, nearClipPlane, farClipPlane);
    float attenuation = 0.05f, intensity = 0.5f, shininess = 128.f;
    glm::vec3 color = glm::vec3(1.f, 1.f, 1.f), lightPos = glm::vec3(3.17f, 2.34f, -4.184f);

    glUseProgram(cloth_shader);
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "projection"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "viewPos"), 1, GL_FALSE, glm::value_ptr(viewPos));
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "pointLights[0].position"), 1, GL_FALSE, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(cloth_shader, "pointLights[0].color"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(cloth_shader, "pointLights[0].attenuation"), attenuation);
    glUniform1f(glGetUniformLocation(cloth_shader, "pointLights[0].intensity"), intensity);
    glUniform1f(glGetUniformLocation(cloth_shader, "material.shininess"), shininess);
    cloth->render();

    model = glm::identity<glm::mat4>();
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(sphere_vao);
    glDrawElements(GL_TRIANGLES, sphere_draw_call_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(NULL);

    model = glm::identity<glm::mat4>();
    model = glm::translate(model, sphere_pos);
    glUniformMatrix4fv(glGetUniformLocation(cloth_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(sphere_vao2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere_draw_call_count2);
    glBindVertexArray(NULL);

    glUseProgram(grid_shader);
    model = glm::identity<glm::mat4>();
    model = glm::scale(model, glm::vec3(50.f, 50.f, 50.f));
    model = glm::translate(model, glm::vec3(-0.5f, 0.f, -0.5f));
    glUniformMatrix4fv(glGetUniformLocation(grid_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(grid_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(grid_shader, "projection"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniform4f(glGetUniformLocation(grid_shader, "aFragColor"), grid_color.x, grid_color.y, grid_color.z, grid_color.w);
    glBindVertexArray(grid_vao);
    glDrawElements(GL_LINES, grid_draw_call_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(NULL);

    glDisable(GL_DEPTH_TEST); // to make axis line always front of all the objects.
    glUseProgram(axis_shader);
    model = glm::identity<glm::mat4>();
    glUniformMatrix4fv(glGetUniformLocation(axis_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(axis_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(axis_shader, "projection"), 1, GL_FALSE, glm::value_ptr(perspective));
    glUniform4f(glGetUniformLocation(axis_shader, "aFragColor"), grid_color.x, grid_color.y, grid_color.z, grid_color.w);
    glBindVertexArray(axis_line_vao);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(NULL);
    glEnable(GL_DEPTH_TEST);
}
