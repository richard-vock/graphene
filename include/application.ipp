namespace graphene {

namespace detail {

void run_main_loop(application::impl& inst, std::function<void()> init, std::function<void()> render_ui);

}  // namespace detail

template <typename Init, typename... Windows>
inline void
application::run(Init&& init, Windows&&... windows)
{
    detail::run_main_loop(*impl_, init, [=] () mutable {
        (windows.render(), ...);
    });
}

}  // namespace graphene
