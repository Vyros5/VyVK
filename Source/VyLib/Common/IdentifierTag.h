#pragma once

#include <typeindex>

namespace Vy 
{
    template <typename Tag> 
    class IdentifierTag 
    {
        friend class ::std::hash<IdentifierTag>;

    public:
        constexpr IdentifierTag(std::type_index index) noexcept : 
            m_Index{ index } 
        {
        }

        std::strong_ordering operator<=>(const IdentifierTag&) const = default;

    private:
        std::type_index m_Index;
    };
} // namespace Vy

template <typename Tag> 
struct std::hash<Vy::IdentifierTag<Tag>> 
{
    [[nodiscard]] std::size_t
    operator()(Vy::IdentifierTag<Tag> tag) const noexcept 
    {
        return tag.m_Index.hash_code();
    }
};