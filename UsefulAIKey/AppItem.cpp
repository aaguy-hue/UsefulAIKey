#include "pch.h"
#include "AppItem.h"
#if __has_include("AppItem.g.cpp")
#include "AppItem.g.cpp"
#endif

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>


using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::UsefulAIKey::implementation
{
    AppItem::AppItem(hstring const& name, hstring const& path, hstring const& iconPath)
        : m_name(name), m_path(path) /*, m_iconPath(iconPath) */
    {
        using namespace Microsoft::UI::Xaml::Media::Imaging;

        BitmapImage bitmap;
        bitmap.UriSource(Windows::Foundation::Uri(iconPath));
		m_logo = bitmap;
    }

    hstring AppItem::Name() { return m_name; }
    hstring AppItem::Path() { return m_path; }

    bool AppItem::IsPinned() { return m_isPinned; }

    void AppItem::IsPinned(bool value)
    {
        if (m_isPinned == value) return;
        m_isPinned = value;
        // Two properties change together: the flag itself and the derived
        // Visibility the XAML pin marker binds to.
        RaisePropertyChanged(L"IsPinned");
        RaisePropertyChanged(L"PinIndicatorVisibility");
    }

    Visibility AppItem::PinIndicatorVisibility()
    {
        return m_isPinned ? Visibility::Visible : Visibility::Collapsed;
    }

    event_token AppItem::PropertyChanged(Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void AppItem::PropertyChanged(event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void AppItem::RaisePropertyChanged(hstring const& propertyName)
    {
        m_propertyChanged(*this, Data::PropertyChangedEventArgs(propertyName));
    }

    Microsoft::UI::Xaml::Media::ImageSource AppItem::Logo() {
		return m_logo;
    }
}
