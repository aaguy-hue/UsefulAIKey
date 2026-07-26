#include "pch.h"
#include "AppItem.h"
#if __has_include("AppItem.g.cpp")
#include "AppItem.g.cpp"
#endif

// SoftwareBitmapSource is the XAML ImageSource we can feed a SoftwareBitmap into.
// WindowsApps.h gives us LoadIconBitmap, which does the shell/GDI extraction.
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

#include "WindowsApps.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::UsefulAIKey::implementation
{
    AppItem::AppItem(hstring const& name, hstring const& path)
        : m_name(name), m_path(path)
    {
    }

    hstring AppItem::Name() { return m_name; }
    hstring AppItem::Path() { return m_path; }

    Media::ImageSource AppItem::Logo() { return m_logo; }

    bool AppItem::IsSelected() { return m_isSelected; }

    void AppItem::IsSelected(bool value)
    {
        if (m_isSelected == value) return;
        m_isSelected = value;
        // Two properties change together: the flag itself and the derived
        // Visibility the XAML selection highlight binds to.
        RaisePropertyChanged(L"IsSelected");
        RaisePropertyChanged(L"SelectionIndicatorVisibility");
    }

    Visibility AppItem::SelectionIndicatorVisibility()
    {
        return m_isSelected ? Visibility::Visible : Visibility::Collapsed;
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

    fire_and_forget AppItem::LoadIconAsync()
    {
        if (m_path.empty()) co_return;

        // Keep ourselves alive for the whole coroutine (the list could be rebuilt
        // while we're mid-await) and remember the UI thread so we can come back to it.
        auto lifetime = get_strong();
        auto uiThread = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
        auto path = std::wstring{ m_path.c_str() };

        try
        {
            // background work: the shell/GDI extraction can hit disk, so keep it
            // off the UI thread
            co_await resume_background();
            auto bitmap = LoadIconBitmap(path, 48);
            if (!bitmap) co_return;

            // back on the UI thread: XAML objects must be created there
            co_await wil::resume_foreground(uiThread);

            Media::Imaging::SoftwareBitmapSource source;
            co_await source.SetBitmapAsync(bitmap);

            m_logo = source;
            RaisePropertyChanged(L"Logo");
        }
        catch (hresult_error const&)
        {
            // Some targets have no usable icon. That's fine -- the row just shows no
            // logo. Swallow so one bad icon doesn't take down the whole list load.
        }
    }
}
