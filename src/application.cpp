#include <application.hpp>

namespace fs = std::filesystem;

#include <condition_variable>

#include <GLFW/glfw3.h>
#include <baldr/baldr.hpp>
using namespace baldr;

#include <imgui.h>
#include <imgui.hpp>

#include <boolean.hpp>
#include <button.hpp>
#include <camera.hpp>
#include <events.hpp>
#include <orbit_camera_model.hpp>
#include <range.hpp>
#include <section.hpp>
#include <visible_if.hpp>
#include <window.hpp>

#include <renderer.hpp>

namespace graphene {

namespace {

static void
gl_error_callback(GLenum source,
                  GLenum type,
                  GLuint,
                  GLenum severity,
                  GLsizei,
                  const GLchar* message,
                  const void*)
{
    bool error = false;
    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ||
        severity == GL_DEBUG_SEVERITY_HIGH) {
        error = true;
    }
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
    }
    else {
#ifdef DEBUG_VERBOSE
        bool is_verbose =
            severity == GL_DEBUG_SEVERITY_LOW || severity == GL_DEBUG_SEVERITY_NOTIFICATION;
        if (is_verbose) {
            if constexpr (debug_verbose) {
                pverbose("{} notification:\n{}", src, message);
            }
        }
        else {
            pdebug("{} info:\n{}", src, message);
        }
#endif  // DEBUG_VERBOSE
    }
}

}  // namespace

struct application::impl {
    impl(const parameters& params) : params(params), quit_(false), start_loop_(false)
    {
        start();
    }

    ~impl()
    {
        if (app_thread_) {
            app_thread_->join();
        }
    }

    void
    start()
    {
        app_thread_ = std::make_unique<std::thread>([&]() {
            glfwSetErrorCallback([](int error, const char* description) {
                perror("GLFW Error {}: {}", error, description);
            });

            terminate_unless(glfwInit(), "Unable to initialize GLFW");

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

            window_ = glfwCreateWindow(params.window_size[0],
                                       params.window_size[1],
                                       params.window_title.c_str(),
                                       NULL,
                                       NULL);
            terminate_unless(window_ != NULL, "Unable to create window");
            glfwMakeContextCurrent(window_);
            if (params.vsync) {
                glfwSwapInterval(1);  // vsync
            }

            terminate_unless(GLEW_OK == glewInit(), "Unable to initialize GLEW");

            if constexpr (debug) {
                if (GLFW_TRUE == glfwGetWindowAttrib(window_, GLFW_OPENGL_DEBUG_CONTEXT)) {
                    glEnable(GL_DEBUG_OUTPUT);
                    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                    glDebugMessageCallback(&gl_error_callback, nullptr);
                }
            }

            events_ = std::make_shared<event_manager>();

            events_->connect<events::add_image>(
                [&](std::string key, std::shared_ptr<const baldr::texture> tex) {
                    images_[key] = tex;
                });

            events_->connect<events::remove_image>([&](std::string key) {
                auto it = images_.find(key);
                if (it != images_.end()) {
                    images_.erase(it);
                }
            });

            imgui::init_application(window_, params.font.path.string(), params.font.size, events_);

            shared<float> fov = 60.f;
            view_t cam_view = looking_towards{vec3f_t(0.f, 0.f, 0.f), vec3f_t(0.f, 0.f, -1.f)};
            cam_ = camera::create<orbit_camera_model>(
                cam_view,
                events_,
                vec4i_t(0, 0, params.window_size[0], params.window_size[1]),
                fov);

            detail::renderer::parameters params;
            renderer_ = std::make_shared<detail::renderer>(events_, cam_, params);

            // clang-format off
            property::window vis_window(
                "graphene",
                property::section(
                    "Rendering",
                    property::boolean("Show Normals", params.show_normals),
                    property::range("Splat Radius", params.splat_radius, bounds<float>{0.01f, 20.f}),
                    property::boolean("Show Depth Buffer", params.render_depth_buffer),
                    property::boolean("debug", params.debug)
                ),
                property::section(
                    "Camera",
                    property::range("FOV", fov, bounds{30.f, 120.f}),
                    property::button("Store Camera Position",
                                     [&](){
                                       cam_->store("stored.cam");
                                     }),
                    property::button("Load Camera Position",
                                     [&](){
                                       cam_->restore("stored.cam");
                                     })
                ),
                property::section(
                    "Tonemapping",
                    property::boolean("Enabled", params.tonemapping_enabled),
                    property::visible_if(
                        params.tonemapping_enabled,
                        property::range("Film Slope",
                                        params.film_slope,
                                        bounds<float>{0.0f, 1.f},
                                        [=](float) mutable { params.rebuild_lut = true; }),
                        property::range("Film Shoulder",
                                        params.film_shoulder,
                                        bounds<float>{0.0f, 1.f},
                                        [=](float) mutable { params.rebuild_lut = true; }),
                        property::range("Film Toe",
                                        params.film_toe,
                                        bounds<float>{0.0f, 1.f},
                                        [=](float) mutable { params.rebuild_lut = true; }),
                        property::range("Film Black Clip",
                                        params.film_black_clip,
                                        bounds<float>{0.0f, 0.1f},
                                        [=](float) mutable { params.rebuild_lut = true; }),
                        property::range("Film White Clip",
                                        params.film_white_clip,
                                        bounds<float>{0.0f, 0.1f},
                                        [=](float) mutable { params.rebuild_lut = true; })
                    )
                ),
                property::section(
                    "Selection",
                    property::range("Pick Radius", params.pick_radius, bounds{0.01f, 0.3f})
                )
            );
            // clang-format on

            std::unique_lock<std::mutex> lk(loop_mutex_);
            while (!start_loop_) {
                loop_cv_.wait(lk);
            }

            init_();

            while (!loop_finished()) {
                glfwPollEvents();
                glfwMakeContextCurrent(window_);

                imgui::init_frame();

                vis_window.render();
                render_ui_();

                for (auto const& img : images_) {
                    ImGui::Begin(img.first.c_str());
                    if (ImGui::ImageButton((void*)(intptr_t)img.second->handle(),
                                           ImVec2(img.second->width(), img.second->height()),
                                           ImVec2(0, 1),
                                           ImVec2(1, 0),
                                           0)) {
                        vec2i_t pos = vec2i_t(ImGui::GetMousePos().x - ImGui::GetItemRectMin().x,
                                              ImGui::GetMousePos().y - ImGui::GetItemRectMin().y);
                        events_->emit<events::image_clicked>(img.first, pos);
                    }
                    ImGui::End();
                }

                ImGui::Render();

                renderer_->render();

                imgui::draw_frame();

                glfwSwapBuffers(window_);
            }

            vertex_array::release();

            imgui::shutdown_application();

            glfwDestroyWindow(window_);
            glfwTerminate();
        });
    }

    void
    run_loop(std::function<void()> init, std::function<void()> render_ui)
    {
        init_ = std::move(init);
        render_ui_ = std::move(render_ui);
        std::unique_lock lk(loop_mutex_);
        start_loop_ = true;
        loop_cv_.notify_one();
    }

    void
    terminate()
    {
        std::lock_guard<std::mutex> lg(render_mutex_);
        quit_ = true;
    }

    void
    render_to_texture(std::shared_ptr<texture> color_output, std::shared_ptr<texture> depth_output)
    {
        renderer_->render(color_output, depth_output);
    }

    void
    render_to_texture(mat4f_t const& view_matrix,
                      mat4f_t const& proj_matrix,
                      std::shared_ptr<texture> color_output,
                      std::shared_ptr<texture> depth_output)
    {
        renderer_->render(view_matrix, proj_matrix, color_output, depth_output);
    }

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

    std::shared_ptr<camera> cam_;
    std::shared_ptr<event_manager> events_;
    std::shared_ptr<detail::renderer> renderer_;

    std::unique_ptr<std::thread> app_thread_;
    std::function<void()> render_ui_;
    std::function<void()> init_;

    std::map<std::string, std::shared_ptr<const texture>> images_;

    std::mutex loop_mutex_;
    std::condition_variable loop_cv_;
    bool start_loop_;

    std::mutex render_mutex_;
};

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
    terminate_unless(static_cast<bool>(impl_->events_),
                     "Accessing event manager before starting application");
    return impl_->events_;
}

std::shared_ptr<camera>
application::cam()
{
    terminate_unless(static_cast<bool>(impl_->cam_),
                     "Accessing camera before starting application");
    return impl_->cam_;
}

void
application::render_to_texture(std::shared_ptr<texture> color_output,
                               std::shared_ptr<texture> depth_output)
{
    impl_->render_to_texture(color_output, depth_output);
}

void
application::render_to_texture(mat4f_t const& view_matrix,
                              mat4f_t const& proj_matrix,
                              std::shared_ptr<baldr::texture> color_output,
                              std::shared_ptr<baldr::texture> depth_output)
{
    impl_->render_to_texture(view_matrix, proj_matrix, color_output, depth_output);
}

namespace detail {

void
run_main_loop(application::impl& inst, std::function<void()> init, std::function<void()> render_ui)
{
    inst.run_loop(std::move(init), std::move(render_ui));
}

}  // namespace detail

}  // namespace graphene
