#pragma warning(push, 0)
#include <glad/glad.h>
#include <glfw/glfw3.h>
#pragma warning(pop)

#include <iostream>
#include <array>

/********************************
 * OpenGL utilities and helpers *
 ********************************/
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void*)window;
    glViewport(0, 0, width, height);
}

struct Window {
    GLFWwindow* const handler;
    const int width;
    const int height;
};

GLuint CreateShader(const char* source, GLenum shader_type) {
    const GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint CreateProgram(GLuint vertex_shader, GLuint fragment_shader) {
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    return program;
}


/******************************
 * Math utilities and helpers *
 ******************************/
constexpr float PI = 3.1415926535897931f;

using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;
using mat4 = std::array<vec4, 4>;

bool AlmostEqual(float a, float b) {
    return std::abs(a - b) < std::numeric_limits<float>::epsilon();
}

constexpr float ToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

constexpr float ToDegrees(float radians) {
    return radians * 180.0f / PI;
}

vec3 Normalize(const vec3& vec) {
    float mag = static_cast<float>(sqrt(pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2)));

    if (!AlmostEqual(mag, 1.0f)) {
        return { vec[0] / mag, vec[1] / mag, vec[2] / mag };
    } else {
        return vec;
    }
}

constexpr vec3 Cross(const vec3& first, const vec3& second) {
    return {
        first[1] * second[2] - first[2] * second[1],
        first[2] * second[0] - first[0] * second[2],
        first[0] * second[1] - first[1] * second[0]
    };
}

constexpr mat4 Mul(const mat4& first, const mat4& second) {
    mat4 result{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result[i][j] += static_cast<float>(first[i][k] * second[k][j]);
            }
        }
    }

    return result;
}

mat4 Perspective(float fov, float aspect, float near, float far) {
    const float top = tan(ToRadians(fov) / 2.0f) * near;
    const float right = top * aspect;

    return {
        near / right, 0.0f,       0.0f,                           0.0f,
        0.0f,         near / top, 0.0f,                           0.0f,
        0.0f,         0.0f,       -(far + near) / (far - near),   -1.0f,
        0.0f,         0.0f,       -2 * far * near / (far - near), 0.0f
    };
}

mat4 LookAt(const vec3& pos, const vec3& target, const vec3& up) {
    const vec3 z_axis = Normalize({ pos[0] - target[0], pos[1] - target[1], pos[2] - target[2] });
    const vec3 x_axis = Normalize(Cross(Normalize(up), z_axis));
    const vec3 y_axis = Cross(z_axis, x_axis);

    mat4 translation{
        1.0f,    0.0f,    0.0f,    0.0f,
        0.0f,    1.0f,    0.0f,    0.0f,
        0.0f,    0.0f,    1.0f,    0.0f,
        -pos[0], -pos[1], -pos[2], 1.0f
    };

    mat4 rotation{
        x_axis[0], y_axis[0], z_axis[0], 0.0f,
        x_axis[1], y_axis[1], z_axis[1], 0.0f,
        x_axis[2], y_axis[2], z_axis[2], 0.0f,
        0.0f,      0.0f,      0.0f,      1.0f
    };

    return Mul(translation, rotation);
}

constexpr mat4 Translate(const mat4& matrix, const vec3& vec) {
    return {
        matrix[0][0],          matrix[0][1],          matrix[0][2],          matrix[0][3],
        matrix[1][0],          matrix[1][1],          matrix[1][2],          matrix[1][3],
        matrix[2][0],          matrix[2][1],          matrix[2][2],          matrix[2][3],
        matrix[3][0] + vec[0], matrix[3][1] + vec[1], matrix[3][2] + vec[2], matrix[3][3]
    };
}

constexpr mat4 Scale(const mat4& matrix, const vec3& vec) {
    return {
        matrix[0][0] * vec[0], matrix[0][1],          matrix[0][2],          matrix[0][3],
        matrix[1][0],          matrix[1][1] * vec[1], matrix[1][2],          matrix[1][3],
        matrix[2][0],          matrix[2][1],          matrix[2][2] * vec[2], matrix[2][3],
        matrix[3][0],          matrix[3][1],          matrix[3][2],          matrix[3][3]
    };
}


/******************************************
 * Sources of shaders used in the program *
 ******************************************/
const char* VertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"uniform mat4 model;\n"
"uniform mat4 pv;\n"
"out vec4 VertexColor;\n"
"void main() {\n"
"    VertexColor = vec4(aColor, 1.0);\n"
"    gl_Position = pv * model * vec4(aPos, 1.0);\n"
"}\n\0";

const char* FragmentShaderSource =
"#version 330 core\n"
"in vec4 VertexColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = VertexColor;\n"
"}\n\0";


/*************************
 * Constants and globals *
 *************************/
constexpr int WindowWidth = 800;
constexpr int WindowHeight = 600;
constexpr GLfloat CubeVertices[] = {
    // back
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    // front
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    // left
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    // right
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

     // down
     -0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f, -0.5f,  0.5f,
     -0.5f, -0.5f,  0.5f,
     -0.5f, -0.5f, -0.5f,

     // top
     -0.5f,  0.5f, -0.5f,
      0.5f,  0.5f, -0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
     -0.5f,  0.5f,  0.5f,
     -0.5f,  0.5f, -0.5f
};


/***************************************
 * Visualizations forward declarations *
 ***************************************/
void CubeWave(const Window* window, GLuint shader_program);
void PenroseStairs(const Window* window, GLuint shader_program);


int main() {
    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    const Window window = { glfwCreateWindow(WindowWidth, WindowHeight, "Cubes!", nullptr, nullptr), WindowWidth, WindowHeight };
    if (!window.handler) {
        std::cout << "Failed to initialize window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window.handler);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window.handler, FramebufferSizeCallback);

    // Load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Shader program
    const GLuint vertex_shader = CreateShader(VertexShaderSource, GL_VERTEX_SHADER);
    const GLuint fragment_shader = CreateShader(FragmentShaderSource, GL_FRAGMENT_SHADER);
    const GLuint shader_program = CreateProgram(vertex_shader, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Different scenes
    switch(1) {
        case 0:
            CubeWave(&window, shader_program);
            break;

        case 1:
            PenroseStairs(&window, shader_program);
            break;

        default:
            break;
    }

    // End of application
    glfwTerminate();
    return EXIT_SUCCESS;
}


void CubeWave(const Window* window, GLuint shader_program) {
    constexpr int ROWS = 15;
    constexpr int COLUMNS = 15;
    constexpr float MIN_CUBE_HEIGHT = 5.0f;
    constexpr float CUBE_HEIGHT_MULTIPLIER = 3.0f;
    constexpr float SIN_MULTIPLIER = 2.0f;

    // Verticies
    constexpr GLfloat colors[] = {
        // back
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        // front
        0.0f,  0.0f,  0.18f,
        0.0f,  0.0f,  0.18f,
        0.0f,  0.0f,  0.18f,
        0.0f,  0.0f,  0.18f,
        0.0f,  0.0f,  0.18f,
        0.0f,  0.0f,  0.18f,

        // left
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        // right
        0.65f,  0.8f,  0.6f,
        0.65f,  0.8f,  0.6f,
        0.65f,  0.8f,  0.6f,
        0.65f,  0.8f,  0.6f,
        0.65f,  0.8f,  0.6f,
        0.65f,  0.8f,  0.6f,

        // down
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        // top
        0.4f,  0.6f,  0.65f,
        0.4f,  0.6f,  0.65f,
        0.4f,  0.6f,  0.65f,
        0.4f,  0.6f,  0.65f,
        0.4f,  0.6f,  0.65f,
        0.4f,  0.6f,  0.65f
    };

    // Buffer objects
    GLuint vertex_buffer, color_buffer, vao;
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &color_buffer);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Camera
    const mat4 projection = Perspective(45.0f, static_cast<float>(window->width / window->height), 0.1f, 100.0f);
    const mat4 view = LookAt(
        { 20.0f, 22.5f, 20.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    );

    const mat4 pv = Mul(view, projection);
    const GLint pv_loc = glGetUniformLocation(shader_program, "pv");

    // Load uniforms
    glUseProgram(shader_program);
    glUniformMatrix4fv(pv_loc, 1, GL_FALSE, &pv[0][0]);

    // OpenGL settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window->handler)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);

        const float time = static_cast<float>(glfwGetTime());
        for (int i = -ROWS / 2; i < ROWS / 2; ++i) {
            for (int j = -COLUMNS / 2; j < COLUMNS / 2; ++j) {
                const float distance_factor = static_cast<float>(sqrt(pow(i, 2) + pow(j, 2))) * 0.9f;
                const float height = CUBE_HEIGHT_MULTIPLIER * sin(SIN_MULTIPLIER * time + distance_factor) + MIN_CUBE_HEIGHT;

                mat4 model{
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                };
                model = Translate(model, { static_cast<float>(i), 0.0f, static_cast<float>(j) });
                model = Scale(model, { 1.0f, height, 1.0f });
                const GLint model_loc = glGetUniformLocation(shader_program, "model");
                glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model[0][0]);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        glfwSwapBuffers(window->handler);
        glfwPollEvents();
    }

    // Free memory
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &color_buffer);
}


void PenroseStairs(const Window* window, GLuint shader_program) {
    constexpr GLfloat colors[] = {
        // back
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,

        // front
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,

        // left
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,

        // right
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,
        0.37f, 0.0f,  0.73f,

        // down
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,

        // top
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f,
        0.63f, 0.61f,  0.91f
    };

    // Buffer objects
    GLuint vertex_buffer, color_buffer, vao;
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &color_buffer);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Camera
    const mat4 projection = Perspective(45.0f, static_cast<float>(window->width / window->height), 0.1f, 100.0f);
    const mat4 view = LookAt(
        { 10.9f, 7.8f, 4.2f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    );

    const mat4 pv = Mul(view, projection);

    // Scene
    const vec3 positions[] = {
        {2.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 1.0f},
        {2.0f, 0.0f, 2.0f},
        {1.0f, 0.0f, 2.0f},
        {0.0f, 0.0f, 2.0f},
        {-1.0f, 0.0f, 2.0f},
        {-2.0f, 0.0f, 2.0f},
        {-2.0f, 0.0f, 1.0f},
        {-2.0f, 0.0f, 0.0f},
        {-2.0f, 0.0f, -1.0f},
        {-1.0f, 0.0f, -1.0f},
    };
    const float height_modifier = -0.1f;

    // Load uniforms
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "pv"), 1, GL_FALSE, &pv[0][0]);

    // OpenGL settings
    glClearColor(0.87f, 0.95f, 1.0f, 0.9f);
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window->handler)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);

        for (int i = 0; i < sizeof(positions) / sizeof(positions[0]); i++) {
            vec3 position = positions[i];
            position[1] = height_modifier * i / 2.0f;

            vec3 scale{1.0f, 5.0f + height_modifier * i, 1.0f};

            mat4 model{
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            model = Translate(model, position);
            model = Scale(model, scale);

            glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window->handler);
        glfwPollEvents();
    }

    // Free memory
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &color_buffer);
}
