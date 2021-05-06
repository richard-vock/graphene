#include <renderable.hpp>
#include <kernel.hpp>

#include <random>
#include <algorithm>
#include <numeric>
#include <chrono>
namespace chr = std::chrono;

constexpr uint32_t num_resolution_samples = 20;
constexpr uint32_t grid_size = 512u;

namespace graphene {

namespace detail {

class search_grid {
public:
    search_grid(std::shared_ptr<baldr::data_buffer> vbo,
                std::shared_ptr<baldr::data_buffer> bbox,
                float search_radius,
                vec3f_t const& sector_origin)
        : radius_(search_radius)
    {
        uint32_t point_count = vbo->value_count<float>() / 10u;


        baldr::data_buffer index_buffer(sizeof(uint32_t), GL_DYNAMIC_DRAW);

        constexpr uint32_t cell_count = grid_size * grid_size * grid_size;
        baldr::data_buffer grid(cell_count * 2 * sizeof(uint32_t), GL_DYNAMIC_DRAW);
        grid.clear_to_zero();

        auto count_points = std::make_shared<detail::kernel>("count_grid_points");
        count_points->uniform("point_count") = point_count;
        count_points->uniform("origin") = sector_origin;
        count_points->uniform("radius") = search_radius;
        count_points->ssbo("vbo_buffer") = *vbo;
        count_points->ssbo("bbox_buffer") = *bbox;
        count_points->ssbo("index_buffer") = index_buffer;
        count_points->ssbo("grid_buffer") = grid; // misused as cell point counter
        count_points->launch1(point_count, 1024u);
        count_points->sync();

        uint32_t work_groups = cell_count / (2*1024);
        baldr::data_buffer sum_buffer(work_groups * sizeof(uint32_t), GL_DYNAMIC_DRAW);

        auto sum = std::make_shared<detail::kernel>("prefix_sum");
        sum->ssbo("grid_buffer") = grid; // misused as cell point counter
        sum->ssbo("sum_buffer") = sum_buffer;
        sum->launch(work_groups, 1, 1);
        sum->sync();

        auto correct = std::make_shared<detail::kernel>("prefix_sum_block_correct");
        correct->ssbo("grid_buffer") = grid;
        correct->ssbo("sum_buffer") = sum_buffer;
        correct->launch(work_groups, 1, 1);
        correct->sync();

        baldr::data_buffer sorted_indices(point_count * sizeof(uint32_t), GL_DYNAMIC_DRAW);
        auto reorder = std::make_shared<detail::kernel>("reorder_points");
        reorder->uniform("point_count") = point_count;
        reorder->uniform("origin") = sector_origin;
        reorder->uniform("radius") = search_radius;
        reorder->ssbo("vbo_buffer") = *vbo;
        reorder->ssbo("bbox_buffer") = *bbox;
        reorder->ssbo("grid_buffer") = grid;
        reorder->ssbo("sorted_indices_buffer") = sorted_indices;
        reorder->launch1(point_count, 1024u);
        reorder->sync();

        // baldr::data_buffer neigh_buffer(point_count * sizeof(uint32_t), GL_DYNAMIC_DRAW);
        // neigh_buffer.clear_to_zero();

        // auto count_neighs = std::make_shared<detail::kernel>("count_neighbors");
        // count_neighs->uniform("point_count") = point_count;
        // count_neighs->uniform("origin") = sector_origin;
        // count_neighs->uniform("radius") = search_radius;

        // count_buffer.get_data(debug_data.get());
        // fmt::print("after:\n");
        // for (uint32_t i = 0; i < 70; ++i) {
        //     if (i == 64) {fmt::print("| ");}
        //     fmt::print("{} ", debug_data.get()[i]);
        // }
        // fmt::print("\n");

        // getchar();

        // uint32_t grid_point_count = 0;
        // index_buffer.get_data(&grid_point_count);
        // fmt::print("points in grid: {}\n", grid_point_count);
    }

    bbox3f_t
    valid_bounds() const {
        return valid_bounds_;
    }

protected:
    vec3i_t
    grid_coord(vec3f_t const& pos) const {
        vec3i_t coord = ((pos - bounds_.min()) / radius_).template cast<int>();
        return coord;
    }

    uint32_t
    hash_cell(vec3i_t const& cell) const {
        return cell[2] * grid_size * grid_size
             + cell[1] * grid_size
             + cell[0];
    }

protected:
    float radius_;
    bbox3f_t bounds_;
    bbox3f_t valid_bounds_;
    std::shared_ptr<baldr::data_buffer> grid_;
    // std::vector<std::pair<uint32_t, uint32_t>> grid_points_;
    // std::vector<uint32_t> cells_begin_;
    // std::vector<uint32_t> cells_end_;
    // std::shared_ptr<data_buffer> cells_;
};

} // detail


void
renderable::compute_bbox_() {
    auto vbo = vertex_buffer();

    uint32_t count = vertex_count();
    uint32_t num_iters = static_cast<uint32_t>(std::log2(count));

    uint32_t work_count = count / 2 + count % 2;

    baldr::data_buffer work_buffer(6 * work_count * sizeof(float), GL_DYNAMIC_DRAW);
    auto bbox_buffer = std::make_shared<baldr::data_buffer>(6 * sizeof(float), GL_DYNAMIC_DRAW);

    // init samples buffers
    std::default_random_engine gen;
    std::vector<uint32_t> samples(num_resolution_samples);
    std::iota(samples.begin(), samples.end(), 0u);
    std::shuffle(samples.begin(), samples.end(), gen);
    baldr::data_buffer sample_buffer(num_resolution_samples * sizeof(uint32_t), GL_DYNAMIC_DRAW, samples.data());
    baldr::data_buffer sample_dist_buffer(num_resolution_samples * sizeof(float), GL_DYNAMIC_DRAW);
    samples.clear();

    auto compute_bb = std::make_shared<detail::kernel>("compute_bounding_box");
    compute_bb->ssbo("vbo_buffer") = *vbo;
    compute_bb->ssbo("work_buffer") = work_buffer;
    compute_bb->ssbo("bbox_buffer") = *bbox_buffer;
    compute_bb->ssbo("sample_buffer") = sample_buffer;
    compute_bb->ssbo("sample_dist_buffer") = sample_dist_buffer;
    compute_bb->uniform("max_count") = count;
    compute_bb->uniform("sample_count") = num_resolution_samples;
    for (uint32_t iter=0; iter <= num_iters; ++iter) {
        uint32_t div = std::pow(2u, iter+1);
        uint32_t iter_count = count / div;
        if (count % div) {
            ++iter_count;
        }
        compute_bb->uniform("iter_count") = iter_count;
        compute_bb->uniform("iter") = iter;
        compute_bb->launch1(count);
        compute_bb->sync();
    }

    std::unique_ptr<float[]> bbox_data(new float[6]);
    bbox_buffer->get_data(bbox_data.get());
    bbox_.min()[0] = bbox_data[0];
    bbox_.min()[1] = bbox_data[1];
    bbox_.min()[2] = bbox_data[2];
    bbox_.max()[0] = bbox_data[3];
    bbox_.max()[1] = bbox_data[4];
    bbox_.max()[2] = bbox_data[5];

    // fmt::print("bbox min: {}\n", bbox_.min().transpose());
    // fmt::print("bbox max: {}\n", bbox_.max().transpose());

    std::unique_ptr<float[]> sample_dist_data(new float[num_resolution_samples]);
    sample_dist_buffer.get_data(sample_dist_data.get());
    std::sort(sample_dist_data.get(), sample_dist_data.get() + num_resolution_samples);
    res_ = sample_dist_data[num_resolution_samples / 2u];

    // fmt::print("res: {}\n", res_);
    // //
    // float search_radius = 2.f * res_;
    // float extent = search_radius * static_cast<float>(grid_size - 2);
    // vec3i_t sec_counts = (bbox_.diagonal() / extent).array().ceil().template cast<int>();
    // fmt::print("sector counts: {}\n", sec_counts.transpose());
    // auto begin = chr::system_clock::now();
    // for (int z = 0; z < sec_counts[2]; ++z) {
    //     for (int y = 0; y < sec_counts[1]; ++y) {
    //         for (int x = 0; x < sec_counts[0]; ++x) {
    //             vec3i_t coord(x, y, z);
    //             vec3f_t origin = bbox_.min() + extent * coord.template cast<float>();
    //             // fmt::print("building grid {} @ {}\n", coord.transpose(), origin.transpose());
    //             detail::search_grid grid(vbo, bbox_buffer, 2.f * res_, origin);
    //         }
    //     }
    // }
    // auto end = chr::system_clock::now();
    // uint64_t dur = chr::duration_cast<chr::milliseconds>(end - begin).count();
    // fmt::print("Building grids took {}ms\n", dur);

    computed_bbox_ = true;
}

std::shared_ptr<baldr::vertex_array>
renderable::vertex_array() {
    if (!vao_) {
        vao_ = std::make_shared<baldr::vertex_array>();
        vao_->set_index_buffer(index_buffer());
    }
    return vao_;
}

bool
renderable::pickable() const {
    return pickable_;
}

void
renderable::set_pickable(bool state) {
    pickable_ = state;
}

std::optional<uint32_t>
renderable::pick_vertices(const vec2f_t& px,
                          const mat4f_t& view_matrix,
                          const mat4f_t& projection_matrix,
                          const vec4i_t& viewport,
                          float radius,
                          uint32_t max_candidates) {
    auto vbo = vertex_buffer();
    baldr::data_buffer candidates(2 * max_candidates * sizeof(float), GL_DYNAMIC_DRAW);
    baldr::data_buffer counter(sizeof(uint32_t), GL_DYNAMIC_DRAW);
    uint32_t point_count = vertex_count();

    auto gather = std::make_shared<detail::kernel>("gather_ray_cylinder_points");
    gather->ssbo("vbo_buffer") = *vbo;
    gather->ssbo("candidate_buffer") = candidates;
    gather->ssbo("count_buffer") = counter;
    gather->uniform("point_count") = point_count;
    gather->uniform("max_candidates") = max_candidates;
    gather->uniform("radius") = radius;
    gather->uniform("px") = px;
    gather->uniform("modelview_mat") = view_matrix * transform();
    gather->uniform("proj_mat") = projection_matrix;
    gather->uniform("viewport") = viewport;
    gather->launch1(point_count);
    gather->sync();

    uint32_t candidate_count = 0;
    counter.get_data(&candidate_count);
    // fmt::print("candidate_count: {}\n", candidate_count);
    if (!candidate_count) {
        return std::nullopt;
    }

    std::unique_ptr<float[]> candidate_data(new float[2 * max_candidates]);
    candidates.get_data(candidate_data.get());

    candidate_count = std::min(candidate_count, max_candidates);
    uint32_t nearest = candidate_data[0];
    float min_depth = candidate_data[max_candidates];
    for (uint32_t i = 1; i < candidate_count; ++i) {
        float depth = candidate_data[max_candidates+i];
        if (depth < min_depth) {
            min_depth = depth;
            nearest = static_cast<uint32_t>(candidate_data[i]);
        }
    }

    return nearest;
}

bool
renderable::hidden() const {
    return hidden_;
}

void
renderable::hide() {
    hidden_ = true;
}

void
renderable::show() {
    hidden_ = false;
}

void
renderable::set_hidden(bool state) {
    hidden_ = state;
}

} // graphene
