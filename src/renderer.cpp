#include <impl/renderer.hpp>

#include <png++/png.hpp>
#include <numeric>

using namespace baldr;

namespace graphene::detail {

renderer::renderer(std::shared_ptr<event_manager> events, std::shared_ptr<camera> cam) : events_(events), cam_(cam) { init(); }

renderer::~renderer() {}

void
renderer::init()
{
    gbuffer_vs_ = shader_program::load(SHADER_ROOT + "gbuffer.vert",
                               GL_VERTEX_SHADER);
    gbuffer_fs_ = shader_program::load(SHADER_ROOT + "gbuffer.frag",
                               GL_FRAGMENT_SHADER);

    pipeline_ = std::make_shared<shader_pipeline>(gbuffer_vs_, gbuffer_fs_);

    // clear color
    clear_color_ = vec3f_t(0.2f, 0.2f, 0.2f);
    events_->connect<events::set_clear_color>([&](vec3f_t col) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        clear_color_ = col;
    });

    // add object
    events_->connect<events::add_object>([&](std::string name, std::shared_ptr<const renderable> obj) {
        std::lock_guard<std::mutex> lock(data_mutex_);

        objects_[name] = render_data();
        render_data& data = objects_[name];

        data.hidden = false;
        data.transform = obj->transform();

        switch (obj->render_mode()) {
            case render_mode_t::splats:
                data.mode = GL_POINTS;
                break;
            case render_mode_t::triangles:
                data.mode = GL_TRIANGLES;
                break;
            case render_mode_t::lines:
                data.mode = GL_LINES;
                break;
            case render_mode_t::line_strip:
                data.mode = GL_LINE_STRIP;
                break;
        }
        data.shaded = obj->shaded();

        renderable::data_matrix_t data_mat = obj->data_matrix();
        std::vector<uint32_t> indices = obj->vertex_indices();
        data.vertex_count = indices.size();
        if (data.vertex_count) {
            data.vbo = std::make_unique<data_buffer>(data_mat, GL_STATIC_DRAW);
            data.ibo = std::make_unique<data_buffer>(indices, GL_STATIC_DRAW);
            data.vao = std::make_unique<vertex_array>();
            data.vao->set_index_buffer(*data.ibo);
            gbuffer_vs_->buffer_binding(*data.vao, "pos", "nrm", "fltcol", "uv", padding(sizeof(float))) = *data.vbo;
        }

        if (auto tex = obj->texture()) {
            vec2i_t size = obj->texture_size();
            data.texture = texture::rgba32f(size[0], size[1]);
            data.texture->set(tex->data());
        }
    });

    // remove object
    events_->connect<events::remove_object>([&](std::string name) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (auto it = objects_.find(name); it != objects_.end()) {
            objects_.erase(it);
        }
    });

    // hide object
    events_->connect<events::remove_object>([&](std::string name) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (auto it = objects_.find(name); it != objects_.end()) {
            it->second.hidden = true;
        }
    });

    // show object
    events_->connect<events::remove_object>([&](std::string name) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (auto it = objects_.find(name); it != objects_.end()) {
            it->second.hidden = false;
        }
    });

    // viewport dependent initialization
    auto reshape = [&](vec4i_t ) {
        //compose_tex_ = texture::rgba32f(vp[2], vp[3]);
        //red_tex_ = texture::rgba32f(vp[2], vp[3]);
        //gbuffer_fs_->output("color") = *compose_tex_;
        //gbuffer_fs_->output("red") = *red_tex_;
    };
    reshape(cam_->viewport());
    events_->connect<events::window_resize>(reshape);
}

void
renderer::render()
{
    gbuffer_vs_->uniform("view_mat") = cam_->view_matrix();
    gbuffer_fs_->uniform("view_mat") = cam_->view_matrix();
    if (cam_->projection_matrix_changed()) {
        gbuffer_vs_->uniform("proj_mat") =
            cam_->projection_matrix();
    }
    vec4i_t vp = cam_->viewport();
    glViewport(vp[0], vp[1], vp[2], vp[3]);

    {
        std::lock_guard lock(data_mutex_);
        glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], 1.f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //backbuffer()->bind();
    pipeline_->bind();

    for (auto && [name, obj] : objects_) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (obj.hidden) {
            continue;
        }

        gbuffer_vs_->uniform("model_mat") = obj.transform;
        gbuffer_vs_->uniform("normal_mat") = cam_->normal_matrix() * obj.transform.topLeftCorner<3,3>();
        gbuffer_fs_->uniform("use_texture") = static_cast<bool>(obj.texture);
        gbuffer_fs_->uniform("shade") = static_cast<bool>(obj.shaded);
        if (obj.texture) {
            gbuffer_fs_->sampler("tex") = *obj.texture;
        }

        if (obj.vertex_count) {
            obj.vao->bind();
            glDrawElements(obj.mode, obj.vertex_count, GL_UNSIGNED_INT, 0);
        }
    }
}

}  // namespace graphene::detail
