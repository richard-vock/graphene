#include <filesystem>
namespace fs = std::filesystem;

#include <graphene/graphene.hpp>
//#include <graphene/assimp.hpp>
#include <graphene/pcl.hpp>
using namespace graphene;

#include <CLI/CLI.hpp>

int
main(int argc, char const** argv)
{
    CLI::App app("Simple Visualizer");
    std::vector<fs::path> input_files;
    auto* opt_input = app.add_option("input", input_files, "Input Files");
    opt_input->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    //mat4f_t scale = mat4f_t::Identity();
    //scale.topLeftCorner<3,3>() *= 10.f;
    std::vector<std::shared_ptr<renderable>> objects;
    for (const auto& input_file : input_files) {
        //auto meshes = assimp::load_meshes(input_file, true);
        //objects.insert(objects.end(), meshes.begin(), meshes.end());
        auto cloud = pcl::load_cloud(input_file, vec4f_t::Ones());
        //cloud->move(scale);
        objects.push_back(cloud);
    }

    application vis(
        {.window_size = vec2i_t(1024, 768),
         .window_title = "graphene",
         .vsync = true,
         .font = {.path = "/usr/share/fonts/TTF/Inconsolata-Regular.ttf",
                  .size = 16.f}});

    // clang-format off
    shared<float> test_float = 0.4f;
    shared<float> test_float2 = 0.4f;
    shared<vec3f_t> clear_col(vec3f_t(0.45f, 0.55f, 0.60f));
    property::window vis_window(
        "Hello world",
        property::section(
            "Scalars",
            property::range("float", test_float, bounds{0.f, 1.f},
            [&](float v) { pdebug("{}", v); }),
            property::real("float2", test_float2, 2, 0.02f,
                                    [&](float v) { pdebug("{}", v); }),
            property::rgb("Background Color", clear_col)
        )
    );
    // clang-format on

    // on init add meshes
    auto init = [&]() {
        uint32_t obj_idx = 1;
        for (auto obj : objects) {
            vis.events()->emit<events::add_object>(
                fmt::format("mesh {:02}", obj_idx++), obj);
        }
    };

    vis.run(init, vis_window);
}
