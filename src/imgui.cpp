#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER <= 1500  // MSVC 2008 or earlier
#include <stddef.h>                        // intptr_t
#else
#include <stdint.h>  // intptr_t
#endif
#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#include <baldr/baldr.hpp>
using namespace baldr;
#include <impl/imgui.hpp>

// GLFW
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>  // for glfwGetWin32Window
#endif
#define GLFW_HAS_WINDOW_TOPMOST                              \
    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= \
     3200)  // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED                              \
    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= \
     3300)  // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA                                \
    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= \
     3300)  // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI                             \
    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= \
     3300)  // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN                                      \
    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= \
     3200)  // 3.2+ glfwCreateWindowSurface

namespace graphene::imgui {

using namespace ::graphene::detail;

static std::shared_ptr<shader_program> g_vert_shader = nullptr;
static std::shared_ptr<shader_program> g_frag_shader = nullptr;
static std::shared_ptr<shader_pipeline> g_pipeline = nullptr;
static event_manager* g_events = nullptr;

// OpenGL Data
static GLuint g_FontTexture = 0;
static int g_AttribLocationPosition = 0, g_AttribLocationUV = 0,
           g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;

static GLFWwindow* g_Window = NULL;
static double g_Time = 0.0;
static bool g_MouseJustPressed[5] = {false, false, false, false, false};
static bool g_pressed[5] = {false, false, false, false, false};
static bool g_dragged[5] = {false, false, false, false, false};
static vec2f_t g_cursor = vec2f_t(-1.f, -1.f);
static GLFWcursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {0};

namespace detail {

static void update_mouse_state();

static void update_mouse_cursor();

static const char* get_clipboard_text(void* user_data);

static void set_clipboard_text(void* user_data, const char* text);

void on_mouse_button_event(GLFWwindow*, int button, int action,
                                        int /*mods*/);

void on_mouse_move(GLFWwindow*, double x, double y);

void on_mouse_enter(GLFWwindow*, int entered);

void on_scroll(GLFWwindow*, double xoffset, double yoffset);

void on_window_resize(GLFWwindow*, int width, int height);

bool create_fonts_texture();

bool create_device_objects();

modifier modifier_state() {
    ImGuiIO& io = ImGui::GetIO();

    return {
        .ctrl  = io.KeyCtrl,
        .alt   = io.KeyAlt,
        .shift = io.KeyShift,
        .super = io.KeySuper
    };
}

}  // namespace detail

// Functions
bool
init_application(GLFWwindow* window, const std::string& font_path,
                 float font_size, std::shared_ptr<event_manager> events)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable
    // Keyboard Controls io.ConfigFlags |=
    // ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    g_Window = window;
    g_Time = 0.0;
    g_events = events.get();

    // Setup back-end capabilities flags
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor()
                                            // values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos
                                           // requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the
    // io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = detail::set_clipboard_text;
    io.GetClipboardTextFn = detail::get_clipboard_text;
    io.ClipboardUserData = g_Window;
#if defined(_WIN32)
    io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
#endif

    g_MouseCursors[ImGuiMouseCursor_Arrow] =
        glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] =
        glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(
        GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] =
        glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] =
        glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(
        GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(
        GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
    // g_MouseCursors[ImGuiMouseCursor_Hand] =
    // glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    glfwSetMouseButtonCallback(window,
                               detail::on_mouse_button_event);

    glfwSetCursorPosCallback(window, detail::on_mouse_move);
    glfwSetScrollCallback(window, detail::on_scroll);
    glfwSetCursorEnterCallback(window, detail::on_mouse_enter);
    glfwSetWindowSizeCallback(window, detail::on_window_resize);

    glfwSetKeyCallback(window, [](GLFWwindow*, int key, int, int action, int mods) {
        ImGuiIO& io = ImGui::GetIO();
        if (action == GLFW_PRESS) io.KeysDown[key] = true;
        if (action == GLFW_RELEASE) io.KeysDown[key] = false;

        (void)mods;  // Modifiers are not reliable across systems
        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] ||
                     io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift =
            io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt =
            io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper =
            io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    });

    glfwSetCharCallback(window, [] (GLFWwindow*, unsigned int c) {
        ImGuiIO& io = ImGui::GetIO();
        if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
    });



    // Setup style
    ImGui::StyleColorsDark();
    ImFont* imgui_font =
        io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
    terminate_unless(imgui_font, "Unable to load font");

    return true;
}

void
shutdown_application()
{
    if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
    g_VboHandle = g_ElementsHandle = 0;

    if (g_FontTexture) {
        ImGuiIO& io = ImGui::GetIO();
        glDeleteTextures(1, &g_FontTexture);
        io.Fonts->TexID = 0;
        g_FontTexture = 0;
    }

    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT;
         cursor_n++) {
        glfwDestroyCursor(g_MouseCursors[cursor_n]);
        g_MouseCursors[cursor_n] = NULL;
    }

    g_pipeline.reset();
    g_vert_shader.reset();
    g_frag_shader.reset();

    ImGui::DestroyContext();
}

void
init_frame()
{
    if (!g_FontTexture) {
        detail::create_device_objects();
    }

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt());  // Font atlas needs to be built, call
                                     // renderer _NewFrame() function e.g.
                                     // ImGui_ImplOpenGL3_NewFrame()

    // Setup display size
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                        h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime =
        g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;

    detail::update_mouse_state();
    detail::update_mouse_cursor();

    // Gamepad navigation mapping [BETA]
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) {
// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)                                      \
    {                                                                      \
        if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) \
            io.NavInputs[NAV_NO] = 1.0f;                                   \
    }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1)                     \
    {                                                           \
        float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0;  \
        v = (v - V0) / (V1 - V0);                               \
        if (v > 1.0f) v = 1.0f;                                 \
        if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; \
    }
        int axes_count = 0, buttons_count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
        const unsigned char* buttons =
            glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
        MAP_BUTTON(ImGuiNavInput_Activate, 0);    // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel, 1);      // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu, 2);        // Square / X
        MAP_BUTTON(ImGuiNavInput_Input, 3);       // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);   // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight, 11);  // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp, 10);     // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown, 12);   // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);   // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext, 5);   // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);   // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast, 5);   // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
        MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
        if (axes_count > 0 && buttons_count > 0)
            io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
        else
            io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    }

    ImGui::NewFrame();
}

void
draw_frame()
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width =
        (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height =
        (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin
    // is typically (0,0) for single viewport apps.
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    mat4f_t ortho_proj;
    ortho_proj <<
        2.0f / (R - L), 0.0f, 0.0f, (R + L) / (L - R),
        0.0f, 2.0f / (T - B), 0.0f, (T + B) / (B - T),
        0.f, 0.f, -1.f, 0.f,
        0.f, 0.f, 0.f, 1.f;

    g_pipeline->bind();
    glProgramUniform1i(g_frag_shader->program(), g_frag_shader->sampler("tex").location, 0);
    g_vert_shader->uniform("mvp") = ortho_proj;

    GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glEnableVertexAttribArray(g_AttribLocationPosition);
    glEnableVertexAttribArray(g_AttribLocationUV);
    glEnableVertexAttribArray(g_AttribLocationColor);
    glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

    // Draw
    ImVec2 pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                     (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                     (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                ImVec4 clip_rect =
                    ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y,
                           pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y);
                if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
                    clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
                    glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w),
                              (int)(clip_rect.z - clip_rect.x),
                              (int)(clip_rect.w - clip_rect.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D,
                                  (GLuint)(intptr_t)pcmd->TextureId);
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                   sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                          : GL_UNSIGNED_INT,
                                   idx_buffer_offset);
                }
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    glDeleteVertexArrays(1, &vao_handle);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

namespace detail {

void
update_mouse_state()
{
    // Update buttons
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
        // If a mouse press event came, always pass it as "mouse held this
        // frame", so we don't miss click-release events that are shorter than 1
        // frame.
        io.MouseDown[i] =
            g_MouseJustPressed[i] || glfwGetMouseButton(g_Window, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    // Update mouse position
    const ImVec2 mouse_pos_backup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
#ifdef __EMSCRIPTEN__
    const bool focused = true;  // Emscripten
#else
    const bool focused = glfwGetWindowAttrib(g_Window, GLFW_FOCUSED) != 0;
#endif
    if (focused) {
        if (io.WantSetMousePos) {
            glfwSetCursorPos(g_Window, (double)mouse_pos_backup.x,
                             (double)mouse_pos_backup.y);
        } else {
            double mouse_x, mouse_y;
            glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
    }
}

void
update_mouse_cursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) ||
        glfwGetInputMode(g_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        // Show OS mouse cursor
        // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse
        // cursor with GLFW 3.2, but 3.3 works here.
        glfwSetCursor(g_Window, g_MouseCursors[imgui_cursor]
                                    ? g_MouseCursors[imgui_cursor]
                                    : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

const char*
get_clipboard_text(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

void
set_clipboard_text(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void
on_mouse_button_event(GLFWwindow*, int button, int action,
                                   int /*mods*/)
{
    if (action == GLFW_PRESS && button >= 0 &&
        button < IM_ARRAYSIZE(g_MouseJustPressed)) {
        g_MouseJustPressed[button] = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        if (action == GLFW_PRESS && button >= 0 &&
            button < IM_ARRAYSIZE(g_pressed)) {
            g_pressed[button] = true;
        }
        if (action == GLFW_RELEASE && button >= 0 &&
            button < IM_ARRAYSIZE(g_pressed)) {
            g_pressed[button] = false;
            if (g_dragged[button]) {
                g_events->emit<events::mouse_stop_drag>(button, g_cursor, modifier_state());
            } else {
                g_events->emit<events::mouse_click>(button, g_cursor, modifier_state());
            }
            g_dragged[button] = false;
        }
    }
}

void on_mouse_move(GLFWwindow*, double x, double y) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        if (g_cursor[0] < 0.f || g_cursor[1] < 0.f) {
            g_cursor = vec2f_t(x, y);
            return;
        }

        vec2f_t px(x, y);
        vec2f_t delta = px - g_cursor;
        g_cursor = px;

        bool any_pressed = false;
        for (int i = 0; i < 5; ++i) {
            if (g_pressed[i]) {
                any_pressed = true;
                if (g_dragged[i]) {
                    g_events->emit<events::mouse_drag>(i, delta, px, modifier_state());
                } else {
                    g_events->emit<events::mouse_start_drag>(i, px, modifier_state());
                }
                g_dragged[i] = true;
            }
        }

        if (!any_pressed) {
            g_events->emit<events::mouse_move>(delta, px, modifier_state());
        }
    }
}

void
on_mouse_enter(GLFWwindow*, int) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        // invalidate cursor on enter/leave
        g_cursor = vec2f_t(-1.f, -1.f);
    }
}

void
on_scroll(GLFWwindow*, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;

    if (!io.WantCaptureMouse) {
        g_events->emit<events::mouse_scroll>(yoffset, modifier_state());
    }
}

void
on_window_resize(GLFWwindow*, int width, int height)
{
    g_events->emit<events::window_resize>(vec4i_t(0, 0, width, height));
}

bool
create_fonts_texture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels, &width,
        &height);  // Load as RGBA 32-bits (75% of the memory is wasted, but
                   // default font is so small) because it is more likely to be
                   // compatible with user's existing shaders. If your
                   // ImTextureId represent a higher-level concept than just a
                   // GL texture id, consider calling GetTexDataAsAlpha8()
                   // instead to save on GPU memory.

    // Upload texture to graphics system
    //GLint last_texture;
    //glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

    // Restore state
    //glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

bool
create_device_objects()
{
    // Backup GL state
    //GLint last_texture, last_array_buffer, last_vertex_array;
    //glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    //glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    //glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    g_vert_shader = shader_program::load(SHADER_ROOT + "imgui.vert", GL_VERTEX_SHADER);
    g_frag_shader = shader_program::load(SHADER_ROOT + "imgui.frag", GL_FRAGMENT_SHADER);
    g_pipeline = std::shared_ptr<shader_pipeline>(new shader_pipeline(g_vert_shader, g_frag_shader));

    g_AttribLocationPosition = g_vert_shader->input("position").location;
    g_AttribLocationUV = g_vert_shader->input("uv").location;
    g_AttribLocationColor = g_vert_shader->input("color").location;

    // Create buffers
    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    detail::create_fonts_texture();

    // Restore modified GL state
    //glBindTexture(GL_TEXTURE_2D, last_texture);
    //glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    //glBindVertexArray(last_vertex_array);

    return true;
}

}  // namespace detail

}  // namespace graphene::imgui
