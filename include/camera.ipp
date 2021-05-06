namespace graphene {

template <typename Model>
inline std::shared_ptr<camera>
camera::create(view_t view, std::shared_ptr<event_manager> events, vec4i_t viewport, shared<float> fov) {
    std::shared_ptr<Model> model = camera_traits<Model>::init(view);
    // use std::function closures as a type-erasure mechanism
    return std::shared_ptr<camera>(new camera(
        events,
        std::move(viewport),
        fov,
        [model] (vec3f_t trs, vec3f_t rot) { camera_traits<Model>::update(model, trs, rot); },
        [model] () { return camera_traits<Model>::view_matrix(model); },
        [model] (mat4f_t const& view_mat) { camera_traits<Model>::restore(model, view_mat); }
    ));
}

inline vec3f_t
camera::position() const
{
    return view_matrix_.block<3,3>(0,0).transpose() *
           (-view_matrix_.block<3,1>(0,3));
}

inline vec3f_t
camera::forward() const
{
    return (-view_matrix_.block<1, 3>(2, 0).transpose());
}

inline vec3f_t
camera::right() const
{
    return view_matrix_.block<1, 3>(0, 0).transpose();
}

inline vec3f_t
camera::up() const
{
    return view_matrix_.block<1, 3>(1, 0).transpose();
}

inline float
camera::fov() const
{
    return fov_;
}

inline float
camera::near() const {
    return last_proj_(2,3) / (last_proj_(2,2) - 1.f);
}

inline float
camera::far() const {
    return last_proj_(2,3) / (last_proj_(2,2) + 1.f);
}

inline const vec4i_t&
camera::viewport() const
{
    std::lock_guard<std::mutex> lock(mut_);
    return viewport_;
}

inline mat4f_t
camera::view_matrix() const
{
    std::lock_guard<std::mutex> lock(mut_);
    return view_matrix_;
}

inline mat3f_t
camera::normal_matrix() const {
    std::lock_guard<std::mutex> lock(mut_);
    return view_matrix_.topLeftCorner<3,3>();
}

}  // namespace graphene
