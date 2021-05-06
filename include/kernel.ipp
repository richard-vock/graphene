namespace graphene::detail {

constexpr GLuint default_work_group_size_x = 32;
constexpr GLuint default_work_group_size_y = 32;
constexpr GLuint default_work_group_size_z = 1;

constexpr GLuint
ceil_div(GLuint n, GLuint d) {
    return n / d + (n % d ? 1u : 0u);
}

inline
kernel::kernel(std::string const& name) {
    program = baldr::shader_program::load_binary(GRAPHENE_SHADER_ROOT + name + ".comp", GL_COMPUTE_SHADER);
    pipeline = std::make_shared<baldr::shader_pipeline>(program);
}

inline void
kernel::sync(GLbitfield barriers) noexcept {
    glMemoryBarrier(barriers);
    // GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    // GLenum res = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 10000000000);
    // if(res == GL_TIMEOUT_EXPIRED || res == GL_WAIT_FAILED) {
    //     fmt::print("timeout\n");
    // }
}

inline void
kernel::launch(GLuint groups_x, GLuint groups_y, GLuint groups_z) noexcept {
    pipeline->bind();
    glDispatchCompute(groups_x, groups_y, groups_z);
}

inline void
kernel::launch1(GLuint width) noexcept {
    constexpr GLuint ws0 = detail::default_work_group_size_x;
    launch1(width, ws0);
}

inline void
kernel::launch2(GLuint width, GLuint height) noexcept {
    constexpr GLuint ws0 = detail::default_work_group_size_x;
    constexpr GLuint ws1 = detail::default_work_group_size_y;
    launch2(width, height, ws0, ws1);
}

inline void
kernel::launch3(GLuint width, GLuint height, GLuint depth) noexcept {
    constexpr GLuint ws0 = detail::default_work_group_size_x;
    constexpr GLuint ws1 = detail::default_work_group_size_y;
    constexpr GLuint ws2 = detail::default_work_group_size_z;
    launch3(width, height, depth, ws0, ws1, ws2);
}

inline void
kernel::launch1(GLuint width, GLuint ws0) noexcept {
    launch(ceil_div(width, ws0), 1u, 1u);
}

inline void
kernel::launch2(GLuint width, GLuint height, GLuint ws0, GLuint ws1) noexcept {
    launch(ceil_div(width, ws0), ceil_div(height, ws1), 1u);
}

inline void
kernel::launch3(GLuint width, GLuint height, GLuint depth, GLuint ws0, GLuint ws1, GLuint ws2) noexcept {
    launch(ceil_div(width, ws0), ceil_div(height, ws1), ceil_div(depth, ws2));
}

inline void
kernel::operator()(GLuint groups_x, GLuint groups_y, GLuint groups_z) noexcept {
    launch(groups_x, groups_y, groups_z);
}

inline const baldr::shader_uniform&
kernel::uniform(const std::string& name) const noexcept {
    return program->uniform(name);
}

inline const baldr::sampler_unit&
kernel::sampler(const std::string& name) const noexcept {
    return program->sampler(name);
}

inline const baldr::image_unit&
kernel::image(const std::string& name) const noexcept {
    return program->image(name);
}

inline const baldr::shader_ssbo&
kernel::ssbo(const std::string& name) const noexcept {
    return program->ssbo(name);
}

}  // namespace graphene::detail
