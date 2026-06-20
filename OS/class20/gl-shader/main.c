/* ============================================================
 * OpenGL Shader Demo — CPU + GPU 实时渲染彩虹波纹动画
 *
 * 这个程序演示了现代 OpenGL 可编程管线的完整流程：
 *   1. 用 GLSL 编写顶点和片元着色器（以内嵌 C 字符串形式）
 *   2. 通过 GLFW 创建窗口和 OpenGL 上下文
 *   3. 用 glad 在运行时加载 OpenGL 3.3 函数指针
 *   4. 在渲染循环中每帧更新 uniform 变量并触发 GPU 绘制
 *
 * 本质上，这是一个用 OpenGL 伪装的 "CUDA 程序"：
 *   glDrawArrays  → 等价于 kernel<<<grid, block>>>()
 *   fragment shader → 等价于 __global__ kernel 函数
 *   每个像素    → 等价于每个 CUDA 线程
 *
 * 编译：make
 * 运行：./glshader
 * ============================================================ */

#include <glad/gl.h>       // OpenGL 3.3 函数指针声明（glad 生成）
#include <GLFW/glfw3.h>    // GLFW 窗口和输入库
#include <stdio.h>         // 错误输出 fprintf


/* ============================================================
 * 顶点着色器源码 (Vertex Shader)
 *
 * 作用：确定每个顶点在裁剪坐标系中的位置。
 * 这里不依赖顶点缓冲区（VBO），而是用 gl_VertexID（GPU 内建变量）
 * 直接计算一个大三角形的三个顶点，覆盖整个视口。
 *
 * 为什么 3 个顶点就能覆盖全屏？
 *   顶点 0: (-1, -1) = 左下角
 *   顶点 1: ( 3, -1) = 右下角外侧
 *   顶点 2: (-1,  3) = 左上角外侧
 *   → 这三个点围成的三角形刚好覆盖整个裁剪立方体
 *   → 比用 4 个顶点 + 2 个三角形更简洁
 * ============================================================ */
static const char *vert_src =
    "#version 330 core\n"
    "const vec2 pos[3] = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1,3));\n"
    "void main() { gl_Position = vec4(pos[gl_VertexID], 0, 1); }\n";


/* ============================================================
 * 片元着色器源码 (Fragment / Pixel Shader)
 *
 * 作用：对每个被三角形覆盖的像素，计算最终颜色。
 * GPU 会对 640×480 个像素并行执行这段代码。
 *
 * uniform 变量（由 CPU 端每帧传入）:
 *   u_resolution — 窗口宽高，用于将像素坐标归一化到 [0,1]
 *   u_time       — 当前时间，让颜色随时间变化产生动画
 *
 * 算法:
 *   uv = gl_FragCoord.xy / resolution    → 归一化到 [0,1]
 *   d  = length(uv - 0.5)                → 到画面中心的距离
 *   c  = 0.5 + 0.5 * sin(d * 20 - t * 3) → 波纹 (20 圈波纹，随时间旋转)
 *   color = (c * uv.r, c * uv.g, 1-c)    → 彩色渐变
 * ============================================================ */
static const char *frag_src =
    "#version 330 core\n"
    "uniform vec2 u_resolution;\n"
    "uniform float u_time;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    vec2 uv = gl_FragCoord.xy / u_resolution;\n"
    "    float d = length(uv - 0.5);\n"
    "    float c = 0.5 + 0.5 * sin(d * 20.0 - u_time * 3.0);\n"
    "    frag_color = vec4(vec3(c * uv, 1.0 - c), 1.0);\n"
    "}\n";


/* ============================================================
 * compile — 编译 GLSL 着色器
 *
 * 参数:
 *   type — GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER
 *   src  — GLSL 源码字符串
 *
 * 返回:
 *   编译成功 → shader 对象的 ID（非零整数）
 *   编译失败 → 打印错误日志，仍返回 ID（上层 glLinkProgram 会失败）
 *
 * 流程:
 *   1. glCreateShader: 在 GPU 驱动中分配一个 shader 对象
 *   2. glShaderSource:  把源码传给驱动
 *   3. glCompileShader: 驱动调用 GPU 编译器将 GLSL → GPU 机器码
 *   4. glGetShaderiv:   查询编译是否成功
 *   5. 若失败: glGetShaderInfoLog 获取编译器错误信息
 * ============================================================ */
static GLuint compile(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);           // 分配 shader 对象
    glShaderSource(s, 1, &src, NULL);          // 传入源码（1 个字符串，自动计算长度）
    glCompileShader(s);                        // 驱动内部的 GLSL → GPU 指令 编译

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);  // 查询编译结果
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);  // 拿错误信息
        fprintf(stderr, "shader error: %s\n", log);
    }
    return s;                                  // 返回 shader ID（失败也返回，由上层处理）
}


/* ============================================================
 * main — 程序入口
 *
 * 完整的 OpenGL 程序生命周期:
 *   初始化阶段 → 编译着色器 → 渲染循环 → 清理
 * ============================================================ */
int main(void) {
    /* ---- 第 1 步：初始化 GLFW + 创建窗口 ---- */
    glfwInit();                                                    // 初始化 GLFW 库（建立与 X11/Wayland 的连接）

    // 请求创建 OpenGL 3.3 Core Profile 上下文
    // Core Profile 移除了旧版固定管线（glBegin/glEnd 等）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建 640×480 窗口，第二个参数 NULL 表示不共享上下文
    GLFWwindow *win = glfwCreateWindow(640, 480, "shader", NULL, NULL);
    glfwMakeContextCurrent(win);                                   // 将窗口的 OpenGL 上下文绑定到当前线程

    /* ---- 第 2 步：加载 OpenGL 函数指针 ---- */
    gladLoadGL(glfwGetProcAddress);                                // 运行时加载所有 OpenGL 3.3 函数的地址（glad 通过 glfw 提供的回调向驱动查询）

    /* ---- 第 3 步：编译并链接着色器 ---- */
    // 创建着色器程序对象，依次挂载顶点和片元着色器，然后链接
    GLuint prog = glCreateProgram();                               // 创建一个空的着色器 program 对象
    glAttachShader(prog, compile(GL_VERTEX_SHADER, vert_src));     // 编译顶点着色器并挂载到 program
    glAttachShader(prog, compile(GL_FRAGMENT_SHADER, frag_src));   // 编译片元着色器并挂载到 program
    glLinkProgram(prog);                                           // 链接：将两个 shader 合并为可执行的可编程管线

    /* ---- 第 4 步：准备顶点数组对象 ---- */
    // 现代 OpenGL（Core Profile）要求必须绑定一个 VAO 才能绘制
    // 即使我们不用顶点缓冲区来传数据，这个调用也是必要的
    GLuint vao;
    glGenVertexArrays(1, &vao);                                    // 生成 1 个 VAO（顶点数组对象）

    /* ---- 第 5 步：主渲染循环 ---- */
    // 每帧执行一次，直到用户关闭窗口
    // 帧率由 glfwSwapBuffers 的垂直同步（VSync）控制，通常为 60 FPS
    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);                       // 获取实际帧缓冲尺寸（可能和窗口尺寸不同，例如 Retina 屏）
        glViewport(0, 0, w, h);                                    // 告诉 OpenGL 渲染区域覆盖整个窗口
        glClear(GL_COLOR_BUFFER_BIT);                              // 清空颜色缓冲为黑色（避免上一帧残留）

        /* ---- 设置 uniform 变量并触发绘制 ---- */
        glUseProgram(prog);                                        // 激活着色器程序（之后的绘制使用这个 shader）
        glUniform2f(glGetUniformLocation(prog, "u_resolution"), (float)w, (float)h);  // 传入窗口分辨率
        glUniform1f(glGetUniformLocation(prog, "u_time"), (float)glfwGetTime());       // 传入当前时间（秒）

        glBindVertexArray(vao);                                    // 绑定 VAO（虽然是空的，但必须要绑）
        glDrawArrays(GL_TRIANGLES, 0, 3);                          // 绘制 3 个顶点 → GPU 执行着色器

        /* ---- 双缓冲交换 + 处理事件 ---- */
        glfwSwapBuffers(win);                                      // 交换前后缓冲：绘制到后台，然后显示（避免屏幕闪烁）
        glfwPollEvents();                                          // 处理窗口事件（关闭、按键、鼠标等）
    }

    /* ---- 第 6 步：清理 ---- */
    glfwTerminate();                                               // 销毁窗口，释放 GLFW 资源
    return 0;
}
