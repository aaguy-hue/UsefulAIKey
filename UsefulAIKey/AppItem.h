#pragma once

#include "AppItem.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct AppItem : AppItemT<AppItem>
    {
        AppItem(hstring const& name, hstring const& path);

        hstring Name();
        hstring Path();

        Microsoft::UI::Xaml::Media::ImageSource Logo();

        bool IsSelected();
        void IsSelected(bool value);

        Microsoft::UI::Xaml::Visibility SelectionIndicatorVisibility();

        // INotifyPropertyChanged. cppwinrt lets us back the projected event with a
        // winrt::event; XAML subscribes through these and we fire it in RaisePropertyChanged.
        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        // Fire-and-forget coroutine that fetches the target's icon and assigns Logo.
        // MUST be called on the UI thread -- it captures that thread's DispatcherQueue
        // so it can hop back to build the XAML BitmapImage after the async I/O.
        winrt::fire_and_forget LoadIconAsync();

    private:
        hstring m_name;
        hstring m_path;

        Microsoft::UI::Xaml::Media::ImageSource m_logo{ nullptr };
        bool m_isSelected{ false };
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        void RaisePropertyChanged(hstring const& propertyName);
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct AppItem : AppItemT<AppItem, implementation::AppItem>
    {
    };
}
