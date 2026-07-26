#include "pch.h"
#include "AppItem.h"
#if __has_include("AppItem.g.cpp")
#include "AppItem.g.cpp"
#endif

// These headers give us the WinRT (not Win32/GDI) way to read a file's icon and
// turn it into something XAML can display:
//   StorageFile            - a WinRT handle to a file on disk
//   ThumbnailMode          - asks the shell for the "icon" thumbnail of that file
//   BitmapImage            - an ImageSource that can be fed from a stream
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

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

    fire_and_forget AppItem::LoadIconAsync()
    {
        if (m_path.empty()) co_return;

        // Keep ourselves alive for the whole coroutine (the list could be rebuilt
        // while we're mid-await) and remember the UI thread so we can come back to it.
        auto lifetime = get_strong();
        auto uiThread = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
        auto path = m_path;

        try
        {
            // --- background work: touch the disk / shell off the UI thread ---
            auto file = co_await Windows::Storage::StorageFile::GetFileFromPathAsync(path);

            // ThumbnailMode::SingleItem returns the file's icon (48px is a good size
            // for a 24px Image with room for high-DPI). This is the app's logo.
            auto thumbnail = co_await file.GetThumbnailAsync(
                Windows::Storage::FileProperties::ThumbnailMode::SingleItem, 48);

            if (!thumbnail || thumbnail.Size() == 0) co_return;

            // --- back on the UI thread: XAML objects must be created there ---
            co_await wil::resume_foreground(uiThread);

            Media::Imaging::BitmapImage bitmap;
            co_await bitmap.SetSourceAsync(thumbnail);

            m_logo = bitmap;
            RaisePropertyChanged(L"Logo");
        }
        catch (hresult_error const&)
        {
            // Some targets have no readable icon (or the path isn't accessible).
            // That's fine -- the row just shows no logo. Swallow so one bad icon
            // doesn't take down the whole list load.
        }
    }
}
