#pragma once

#include "ActionOption.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct ActionOption : ActionOptionT<ActionOption>
    {
        ActionOption(UsefulAIKey::ActionKind kind, hstring const& label);

        // Read-only in the .idl, so there is no setter here.
        UsefulAIKey::ActionKind Kind();

        hstring Label();
        void Label(hstring const& value);

    private:
        UsefulAIKey::ActionKind m_kind;
        hstring m_label;
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct ActionOption : ActionOptionT<ActionOption, implementation::ActionOption>
    {
    };
}
