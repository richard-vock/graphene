namespace graphene {

inline float
pack_rgba(const vec4f_t& rgba) {
    packed_color_t p;
    p.r = static_cast<uint8_t>(rgba[0] * 255.f);
    p.g = static_cast<uint8_t>(rgba[1] * 255.f);
    p.b = static_cast<uint8_t>(rgba[2] * 255.f);
    p.a = static_cast<uint8_t>(rgba[3] * 255.f);
    return p.packed;
}

inline vec4f_t
unpack_rgba(const float packed) {
    packed_color_t p;
    p.packed = packed;
    vec4f_t rgba;
    rgba[0] = static_cast<float>(p.r) / 255.f;
    rgba[1] = static_cast<float>(p.g) / 255.f;
    rgba[2] = static_cast<float>(p.b) / 255.f;
    rgba[3] = static_cast<float>(p.a) / 255.f;
    return rgba;
}

inline
renderable::renderable() : transform_(mat4f_t::Identity()) {
}

inline const mat4f_t&
renderable::transform() const {
    return transform_;
}

inline mat4f_t&
renderable::transform() {
    return transform_;
}

inline void
renderable::set_transform(const mat4f_t& t) {
    transform_ = t;
}

inline void
renderable::move(const mat4f_t& t) {
    transform_ = t * transform_;
}

} // graphene
