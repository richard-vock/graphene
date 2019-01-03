#include <impl/renderer.hpp>

#include <png++/png.hpp>
#include <numeric>

using namespace baldr;

namespace graphene::detail {

vec2f_t
correct_near_far(const bbox3f_t& bbox, const ray_t& view, const mat4f_t& transform, vec2f_t nf) {
    auto add = [&] (bbox3f_t::CornerType c) {
        vec3f_t pos = (transform * bbox.corner(c).homogeneous()).head(3);
        float t = view.direction().dot(pos - view.origin());
        nf[0] = std::min(nf[0], t);
        nf[1] = std::max(nf[1], t);
    };

    add(bbox3f_t::BottomLeftFloor);
    add(bbox3f_t::BottomRightFloor);
    add(bbox3f_t::TopLeftFloor);
    add(bbox3f_t::TopRightFloor);
    add(bbox3f_t::BottomLeftCeil);
    add(bbox3f_t::BottomRightCeil);
    add(bbox3f_t::TopLeftCeil);
    add(bbox3f_t::TopRightCeil);

    return nf;
}

renderer::renderer(std::shared_ptr<event_manager> events,
                   std::shared_ptr<camera> cam,
                   const parameters& params)
    : events_(events),
      cam_(cam),
      params_(params)
{
    init();
}

renderer::~renderer() {}

void
renderer::init()
{
    geometry_vs_ = shader_program::load(SHADER_ROOT + "geometry.vert",
                               GL_VERTEX_SHADER);
    geometry_fs_ = shader_program::load(SHADER_ROOT + "geometry.frag",
                               GL_FRAGMENT_SHADER);
    geometry_pass_ = std::make_shared<render_pass>(geometry_vs_, geometry_fs_);

    render_depth_shader_ = shader_program::load(
        SHADER_ROOT + "render_depth.frag", GL_FRAGMENT_SHADER);
    render_depth_pass_ =
        std::make_shared<baldr::fullscreen_pass>(render_depth_shader_);

    point_visibility_ = std::make_shared<point_visibility>(point_visibility::parameters{
        .occlusion_threshold = params_.occlusion_threshold,
        .fill = params_.fill
    });

    // clear color
    clear_color_ = vec4f_t(0.3f, 0.3f, 0.3f, 1.f);

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
            data.ibo = std::make_shared<data_buffer>(indices, GL_STATIC_DRAW);
            data.vao = std::make_unique<vertex_array>();
            data.vao->set_index_buffer(data.ibo);
            geometry_vs_->buffer_binding(*data.vao, "pos", "nrm", "fltcol", "uv", padding(sizeof(float))) = *data.vbo;
        }

        if (auto tex = obj->texture()) {
            vec2i_t size = obj->texture_size();
            data.texture = texture::rgba32f(size[0], size[1]);
            data.texture->set(tex->data());
        }

        // bbox min/max are colwise min/max over the first 3 columns
        data.bbox.min() = data_mat.block(0, 0, data_mat.rows(), 3).colwise().minCoeff().transpose();
        data.bbox.max() = data_mat.block(0, 0, data_mat.rows(), 3).colwise().maxCoeff().transpose();
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
    auto reshape = [&](vec4i_t vp) {
        // textures
        geometry_depth_ = texture::depth32f(vp[2], vp[3]);
        point_visibility_->reshape(vp, geometry_depth_);
    };
    reshape(cam_->viewport());
    events_->connect<events::window_resize>(reshape);
}

void
renderer::render()
{
    mat4f_t vmat = cam_->view_matrix();
    geometry_vs_->uniform("view_mat") = vmat;
    geometry_fs_->uniform("view_mat") = vmat;

    // estimate near/far values using object bounding boxes
    vec2f_t nf(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
    if (objects_.size()) {
        ray_t view(cam_->position(), cam_->forward());
        for (auto && [name, obj] : objects_) {
            nf = correct_near_far(obj.bbox, view, obj.transform, nf);
        }
        //nf[0] -= 0.01f;
        //nf[1] += 0.01f;
        if (nf[0] < 0.001f) {
            nf[0] = 0.001f;
        } else {
        }
        if (nf[1] <= nf[0]) {
            nf[1] = nf[0] + 0.001f;
        }
    } else {
        nf[0] = 0.01f;
        nf[1] = 200.f;
    }

    mat4f_t pmat = cam_->projection_matrix(nf);

    vec4i_t vp = cam_->viewport();
    glViewport(vp[0], vp[1], vp[2], vp[3]);

    geometry_vs_->uniform("proj_mat") = pmat;
    render_depth_shader_->uniform("proj_mat") = pmat;
    render_depth_shader_->uniform("near") = nf[0];
    render_depth_shader_->uniform("far") = nf[1];

    geometry_pass_->render(render_options{
            .depth_attachment = geometry_depth_,
            .clear_depth = 1.f
        },
        [&] (auto vs, auto fs) {
            for (auto && [name, obj] : objects_) {
                std::lock_guard<std::mutex> lock(data_mutex_);
                if (obj.hidden) {
                    continue;
                }

                vs->uniform("model_mat") = obj.transform;
                vs->uniform("normal_mat") = cam_->normal_matrix() * obj.transform.template topLeftCorner<3,3>();
                fs->uniform("use_texture") = static_cast<bool>(obj.texture);
                fs->uniform("shade") = static_cast<bool>(obj.shaded);
                if (obj.texture) {
                    geometry_fs_->sampler("tex") = *obj.texture;
                }

                if (obj.vertex_count) {
                    obj.vao->bind();
                    glDrawElements(obj.mode, obj.vertex_count, GL_UNSIGNED_INT, 0);
                }
            }
    });

    point_visibility_->render(pmat, vp, cam_->near_plane_size());

    render_depth_pass_->render(render_options{
        .input = {{"tex", point_visibility_->output()}},
        .clear_color = *clear_color_
    });
}

}  // namespace graphene::detail
