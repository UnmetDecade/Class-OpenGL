#include "engine/Window/Window.h"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

#include "engine/Core/GLCall.h"
#include "engine/Core/Log.h"

namespace
{
void GLFWErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW error " << error << ": " << description);
}
} // namespace

Window::Window(int width, int height, const std::string& title, bool vsync, int msaaSamples)
{
    glfwSetErrorCallback(GLFWErrorCallback);

    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    if (msaaSamples > 0)
    {
        glfwWindowHint(GLFW_SAMPLES, msaaSamples);
    }

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_Window);

    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) == 0)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        glfwTerminate();
        throw std::runtime_error("Failed to initialize glad (OpenGL loader)");
    }

    glfwSwapInterval(vsync ? 1 : 0);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallbackBridge);
    glfwSetKeyCallback(m_Window, KeyCallbackBridge);
    glfwSetCursorPosCallback(m_Window, CursorPosCallbackBridge);
    glfwSetScrollCallback(m_Window, ScrollCallbackBridge);

    if (msaaSamples > 0)
    {
        GLCall(glEnable(GL_MULTISAMPLE));
    }

    GLint actualSamples = 0;
    GLCall(glGetIntegerv(GL_SAMPLES, &actualSamples));
    m_MSAASamples = actualSamples;

    glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
    GLCall(glViewport(0, 0, m_Width, m_Height));

    const GLubyte* version = nullptr;
    const GLubyte* renderer = nullptr;
    GLCall(version = glGetString(GL_VERSION));
    GLCall(renderer = glGetString(GL_RENDERER));
    LOG_INFO("OpenGL " << version << " | Renderer: " << renderer
                       << " | MSAA samples: " << m_MSAASamples);
}

Window::~Window()
{
    Destroy();
}

Window::Window(Window&& other) noexcept
    : m_Window(other.m_Window),
      m_Width(other.m_Width),
      m_Height(other.m_Height),
      m_MSAASamples(other.m_MSAASamples),
      m_FramebufferSizeCallback(std::move(other.m_FramebufferSizeCallback)),
      m_KeyCallback(std::move(other.m_KeyCallback)),
      m_CursorPosCallback(std::move(other.m_CursorPosCallback)),
      m_ScrollCallback(std::move(other.m_ScrollCallback))
{
    other.m_Window = nullptr;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_MSAASamples = 0;

    if (m_Window != nullptr)
    {
        glfwSetWindowUserPointer(m_Window, this);
    }
}

Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other)
    {
        Destroy();

        m_Window = other.m_Window;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_MSAASamples = other.m_MSAASamples;
        m_FramebufferSizeCallback = std::move(other.m_FramebufferSizeCallback);
        m_KeyCallback = std::move(other.m_KeyCallback);
        m_CursorPosCallback = std::move(other.m_CursorPosCallback);
        m_ScrollCallback = std::move(other.m_ScrollCallback);

        other.m_Window = nullptr;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_MSAASamples = 0;

        if (m_Window != nullptr)
        {
            glfwSetWindowUserPointer(m_Window, this);
        }
    }
    return *this;
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_Window) != 0;
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(m_Window);
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::SetFramebufferSizeCallback(FramebufferSizeCallback callback)
{
    m_FramebufferSizeCallback = std::move(callback);
}

void Window::SetKeyCallback(KeyCallback callback)
{
    m_KeyCallback = std::move(callback);
}

void Window::SetCursorPosCallback(CursorPosCallback callback)
{
    m_CursorPosCallback = std::move(callback);
}

void Window::SetScrollCallback(ScrollCallback callback)
{
    m_ScrollCallback = std::move(callback);
}

void Window::FramebufferSizeCallbackBridge(GLFWwindow* window, int width, int height)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self == nullptr)
    {
        return;
    }

    self->m_Width = width;
    self->m_Height = height;
    GLCall(glViewport(0, 0, width, height));

    if (self->m_FramebufferSizeCallback)
    {
        self->m_FramebufferSizeCallback(width, height);
    }
}

void Window::KeyCallbackBridge(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr && self->m_KeyCallback)
    {
        self->m_KeyCallback(key, scancode, action, mods);
    }
}

void Window::CursorPosCallbackBridge(GLFWwindow* window, double xPos, double yPos)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr && self->m_CursorPosCallback)
    {
        self->m_CursorPosCallback(xPos, yPos);
    }
}

void Window::ScrollCallbackBridge(GLFWwindow* window, double xOffset, double yOffset)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr && self->m_ScrollCallback)
    {
        self->m_ScrollCallback(xOffset, yOffset);
    }
}

void Window::Destroy()
{
    if (m_Window == nullptr)
    {
        return;
    }

    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
    glfwTerminate();
}
