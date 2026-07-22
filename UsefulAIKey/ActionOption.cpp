#include "pch.h"
#include "ActionOption.h"
#if __has_include("ActionOption.g.cpp")
#include "ActionOption.g.cpp"
#endif

namespace winrt::UsefulAIKey::implementation
{
    ActionOption::ActionOption(UsefulAIKey::ActionKind kind, hstring const& label)
        : m_kind(kind), m_label(label)
    {
    }

    UsefulAIKey::ActionKind ActionOption::Kind() { return m_kind; }

    hstring ActionOption::Label() { return m_label; }
    void ActionOption::Label(hstring const& value) { m_label = value; }
}
