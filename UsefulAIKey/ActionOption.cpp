#include "pch.h"
#include "ActionOption.h"
#include "ActionOption.g.cpp"

namespace winrt::UsefulAIKey::implementation
{
    // Implement the constructor defined in the .idl
    winrt::UsefulAIKey::ActionOption(winrt::hstring const& id, winrt::hstring const& label)
        : m_id(id), m_label(label)
    {
    }

    // Property getters/setters (if not already implemented)
    winrt::hstring ActionOption::Id() { return m_id; }
    void ActionOption::Id(winrt::hstring const& value) { m_id = value; }

    winrt::hstring ActionOption::Label() { return m_label; }
    void ActionOption::Label(winrt::hstring const& value) { m_label = value; }
}