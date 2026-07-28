#include "pch.h"
#include "WebsiteLaunchView.xaml.h"
#if __has_include("WebsiteLaunchView.g.cpp")
#include "WebsiteLaunchView.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::UsefulAIKey::implementation
{
    void WebsiteLaunchView::Load(hstring const& savedWebsite)
    {
        m_savedWebsite = savedWebsite;
        //PopulateWebsiteField();
    }

    void WebsiteLaunchView::SaveWebsite_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        // todo: validate that it's an actual valid url
		OutputDebugStringW((L"Saving website: " + WebsiteUrlTextBox().Text() + L"\n").c_str());
    }
}

