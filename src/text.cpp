#include <text.hpp>

#include <imgui.h>

namespace graphene::property {

text::text(const char* txt) : text_(txt) {}

text::text(std::string_view txt) : text_(txt) {}

text::text(const std::string& txt) : text_(txt) {}

void
text::render()
{
    std::string t = text_;
    if (func_) {
        t = func_();
    }
    ImGui::Text("%s", t.c_str());
}

}  // namespace graphene::property
