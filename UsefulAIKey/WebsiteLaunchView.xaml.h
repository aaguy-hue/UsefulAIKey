#pragma once

#include "WebsiteLaunchView.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct WebsiteLaunchView : WebsiteLaunchViewT<WebsiteLaunchView>
    {
        WebsiteLaunchView()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void Load(hstring const& savedWebsite);

    private:
        hstring m_savedWebsite;
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct WebsiteLaunchView : WebsiteLaunchViewT<WebsiteLaunchView, implementation::WebsiteLaunchView>
    {
    };
}
