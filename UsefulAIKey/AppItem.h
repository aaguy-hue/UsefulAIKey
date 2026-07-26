#pragma once

#include "AppItem.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct AppItem : AppItemT<AppItem>
    {
        AppItem(hstring const& name, hstring const& path, hstring const& iconPath);

        hstring Name();
        hstring Path();

        Microsoft::UI::Xaml::Media::ImageSource Logo();

        bool IsPinned();
        void IsPinned(bool value);

        Microsoft::UI::Xaml::Visibility PinIndicatorVisibility();

        // INotifyPropertyChanged. cppwinrt lets us back the projected event with a
        // winrt::event; XAML subscribes through these and we fire it in RaisePropertyChanged.
        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    private:
        hstring m_name;
        hstring m_path;
        //hstring m_iconPath;

        Microsoft::UI::Xaml::Media::ImageSource m_logo{ nullptr };
        bool m_isPinned{ false };
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
