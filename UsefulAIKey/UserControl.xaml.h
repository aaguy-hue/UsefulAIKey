#pragma once

#include "UserControl.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct UserControl : UserControlT<UserControl>
    {
        UserControl()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct UserControl : UserControlT<UserControl, implementation::UserControl>
    {
    };
}
