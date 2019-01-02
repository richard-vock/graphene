#include <graphene/application.hpp>

#include <condition_variable>

#include <baldr/baldr.hpp>
#include <GLFW/glfw3.h>
using namespace baldr;

#include <imgui.h>
#include <impl/imgui.hpp>

#include <graphene/events.hpp>
#include <graphene/camera.hpp>
#include <graphene/orbit_camera_model.hpp>
#include <graphene/window.hpp>
#include <graphene/range.hpp>
#include <graphene/section.hpp>

#include <impl/renderer.hpp>

namespace graphene {

namespace {

static void
gl_error_callback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei,
                  const GLchar* message, const void*)
{
    bool error = false;
    if (type == GL_DEBUG_TYPE_ERROR ||
        type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ||
        severity == GL_DEBUG_SEVERITY_HIGH) {
        error = true;
    }
    bool is_verbose = severity == GL_DEBUG_SEVERITY_LOW ||
                      severity == GL_DEBUG_SEVERITY_NOTIFICATION;
    std::string src;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            src = "OpenGL API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            src = "Window system";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            src = "Shader compilation";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            src = "GL application";
            break;
        default:
            src = "Misc";
            break;
    }

    if (error) {
        perror("{} error:\n{}", src, message);
        std::terminate();
    } else {
        if (is_verbose) {
            if constexpr (debug_verbose) {
                pverbose("{} notification:\n{}", src, message);
            }
        } else {
            pdebug("{} info:\n{}", src, message);
        }
    }
}

}

struct application::impl
{
    impl(const parameters& params);

    ~impl();

    void start();

    void run_loop(std::function<void()> init, std::function<void()> render_ui);

    void terminate();

    bool
    loop_finished()
    {
        std::lock_guard<std::mutex> lg(render_mutex_);
        return quit_ || glfwWindowShouldClose(window_);
    }

    vec2i_t
    window_size() const
    {
        vec2i_t size;
        glfwGetFramebufferSize(window_, &size[0], &size[1]);
        return size;
    }

    float
    framerate() const
    {
        return ImGui::GetIO().Framerate;
    }

    parameters params;

    bool quit_;
    GLFWwindow* window_;

    std::shared_ptr<event_manager> events_;

    std::unique_ptr<std::thread> app_thread_;
    std::function<void()> render_ui_;
    std::function<void()> init_;

    std::mutex loop_mutex_;
    std::condition_variable loop_cv_;
    bool start_loop_;

    std::mutex render_mutex_;
};

application::impl::impl(const parameters& params)
    : params(params), quit_(false), start_loop_(false)
{
    start();
}

application::impl::~impl()
{
    if (app_thread_) {
        app_thread_->join();
    }
}

void
application::impl::start()
{
    app_thread_ = std::make_unique<std::thread>([&]() {
        glfwSetErrorCallback([](int error, const char* description) {
            perror("GLFW Error {}: {}", error, description);
        });

        terminate_unless(glfwInit(), "Unable to initialize GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window_ = glfwCreateWindow(params.window_size[0], params.window_size[1],
                                   params.window_title.c_str(), NULL, NULL);
        terminate_unless(window_ != NULL, "Unable to create window");
        glfwMakeContextCurrent(window_);
        if (params.vsync) {
            glfwSwapInterval(1);  // vsync
        }

        terminate_unless(GLEW_OK == glewInit(), "Unable to initialize GLEW");

        if constexpr (debug) {
            if (GLFW_TRUE ==
                glfwGetWindowAttrib(window_, GLFW_OPENGL_DEBUG_CONTEXT)) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(&gl_error_callback, nullptr);
            }
        }

        events_ = std::make_shared<event_manager>();

        imgui::init_application(window_, params.font.path.string(),
                                params.font.size, events_);

        auto cam = camera::create<orbit_camera_model>(
            looking_at{ vec3f_t(5.f, -20.f, 5.f), vec3f_t(0.f, 0.f, 0.f) },
            events_,
            vec4i_t(0, 0, params.window_size[0], params.window_size[1]),
            60.f
        );

        shared<float> occlusion_threshold = 0.5f;
        detail::renderer renderer(events_, cam, occlusion_threshold);
        init_();

        // clang-format off
        property::window vis_window(
            "graphene",
            property::section(
                "Rendering",
                property::range("Occlusion Threshold", occlusion_threshold, bounds{0.f, 1.f})
            )
        );
        // clang-format on

        std::unique_lock<std::mutex> lk(loop_mutex_);
        while (!start_loop_) {
            loop_cv_.wait(lk);
        }

        while (!loop_finished()) {
            glfwPollEvents();

            imgui::init_frame();

            //vis_window.render();
            //render_ui_();

            //ImGui::Render();
            glfwMakeContextCurrent(window_);

            renderer.render();

            //imgui::draw_frame();

            //glfwMakeContextCurrent(window_);
            glfwSwapBuffers(window_);
        }

        vertex_array::release();

        imgui::shutdown_application();

        glfwDestroyWindow(window_);
        glfwTerminate();
    });
}

void
application::impl::run_loop(std::function<void()> init, std::function<void()> render_ui)
{
    init_ = std::move(init);
    render_ui_ = std::move(render_ui);
    std::unique_lock lk(loop_mutex_);
    start_loop_ = true;
    loop_cv_.notify_one();
}

void
application::impl::terminate()
{
    std::lock_guard<std::mutex> lg(render_mutex_);
    quit_ = true;
}

application::application(const parameters& params) : impl_(new impl(params)) {}

application::~application() {}

vec2i_t
application::window_size() const
{
    return impl_->window_size();
}

float
application::framerate() const
{
    return impl_->framerate();
}

std::shared_ptr<event_manager>
application::events()
{
    terminate_unless(static_cast<bool>(impl_->events_), "Accessing event manager before starting application");
    return impl_->events_;
}

namespace detail {

void
run_main_loop(application::impl& inst, std::function<void()> init, std::function<void()> render_ui)
{
    inst.run_loop(std::move(init), std::move(render_ui));
}

}  // namespace detail

}  // namespace graphene
