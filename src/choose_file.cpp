#include <graphene/choose_file.hpp>

#include <imgui.h>

namespace {

constexpr float modal_width = 600.f;
constexpr float modal_height = 600.f;

}

namespace fs = std::filesystem;

namespace graphene {

namespace filter {

open::open() {}

bool
open::show(const fs::path&)
{
    return true;
}

bool
open::accept(const fs::path& p)
{
    return fs::exists(p);
}

save::save() {}

bool
save::show(const fs::path&)
{
    return true;
}

bool
save::accept(const fs::path&)
{
    return true;
}

file::file() {}

bool
file::show(const fs::path&)
{
    return true;
}

bool
file::accept(const fs::path& p)
{
    return fs::is_regular_file(p);
}

directory::directory() {}

bool
directory::show(const fs::path& p)
{
    return fs::is_directory(p);
}

bool
directory::accept(const fs::path& p)
{
    return fs::is_directory(p);
}

with_extension::with_extension(std::string ext) : ext_(ext) {}

bool
with_extension::show(const fs::path& p)
{
    return !fs::is_regular_file(p) || (p.extension() == ext_);
}

bool
with_extension::accept(const fs::path& p)
{
    return fs::is_regular_file(p) && (p.extension() == ext_);
}

} // filter

namespace property {

void
choose_file::render()
{
    if (ImGui::Button(label_.c_str())) {
        ImGui::SetNextWindowSize(ImVec2(modal_width, modal_height));
        ImGui::OpenPopup(label_.c_str());
    }
    if (ImGui::BeginPopupModal(label_.c_str(), NULL/*,
                               ImGuiWindowFlags_AlwaysAutoResize*/)) {
        //bool pathClick = false;
        uint32_t element_index = 0u;
        uint32_t pop_count = 0u;
        fs::path directory = detail::canonical_path(path_);
        for (auto path_element : directory) {
            if (ImGui::Button(path_element.string().c_str())) {
                pop_count = std::distance(directory.begin(), directory.end()) - element_index - 1;
                //pathClick = true;
                //break;
            }
            ImGui::SameLine();
            ++element_index;
        }
        ImGui::NewLine();
        if (pop_count) {
            path_ = directory;
            while (pop_count--) {
                path_ = path_.parent_path();
            }
        }

        ImVec2 max_size = ImGui::GetContentRegionMax();
        ImVec2 size(max_size.x, max_size.y - 120.f);
        ImGui::Spacing();

        ImGui::BeginChild("##FileDialog_FileList", size);

        std::vector<fs::path> entries;
        fs::directory_iterator dir_it(directory), end_it;
        for (; dir_it != end_it; ++dir_it) {
            fs::path entry(*dir_it);
            if (entry.stem().string()[0] != '.' && list_filter_(entry)) {
                entries.push_back(entry);
            }
        }
        std::sort(entries.begin(), entries.end(), [&] (const fs::path& p0, const fs::path& p1) {
            // directories before files
            if (fs::is_directory(p0) && !fs::is_directory(p1)) {
                return true;
            }
            if (!fs::is_directory(p0) && fs::is_directory(p1)) {
                return false;
            }
            // default to lexicographic comparison
            return p0.string() < p1.string();
        });

        for (const auto& entry : entries)
        {
            std::string str;
            if (fs::is_directory(entry)) {
                str = "[Dir] " + entry.stem().string();
            }
            if (fs::is_symlink(entry)) {
                str = "[Link] " + entry.filename().string();
            }
            if (fs::is_regular_file(entry)) {
                str = "[File] " + entry.filename().string();
            }

            if (list_filter_(entry))
            {
                if (ImGui::Selectable(str.c_str(), fs::equivalent(entry, path_), ImGuiSelectableFlags_DontClosePopups))
                {
                    path_ = directory / (fs::is_directory(entry) ? entry.stem() : entry.filename());
                    //break;
                }
            }
        }

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool choosable = choose_filter_(path_);
        ImVec4 disabled_color(0.3f, 0.3f, 0.3f, 1.f);
        if (!choosable) {
            ImGui::PushStyleColor(ImGuiCol_Button, disabled_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, disabled_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, disabled_color);
        }
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (choosable) {
                chosen_ = true;
                func_(path_);
                ImGui::CloseCurrentPopup();
            }
        }
        if (!choosable) {
            ImGui::PopStyleColor(3);
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    std::string path_str;
    if (chosen_) {
        if (fs::is_directory(path_)) {
            path_str = path_.stem().string();
        } else {
            path_str = path_.filename().string();
        }
    }
    ImGui::SameLine();
    ImGui::Text(path_str.c_str());
}

} // property

}  // namespace graphene
