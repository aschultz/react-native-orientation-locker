#pragma once
#include "pch.h"

#include <functional>
#include <memory>
#include <string>
#include <NativeModules.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Devices.Sensors.h>

namespace OrientationWindows {

    REACT_MODULE(OrientationLockerModule, L"OrientationLocker");
    struct OrientationLockerModule : public std::enable_shared_from_this<OrientationLockerModule>
    {
        REACT_INIT(Initialize)
        void Initialize(React::ReactContext const& reactContext) noexcept;
        winrt::Windows::Foundation::IAsyncAction InitOnUI();

        REACT_CONSTANT_PROVIDER(GetConstants)
        void GetConstants(React::ReactConstantProvider& provider) noexcept;

        REACT_METHOD(GetOrientation, L"getOrientation");
        winrt::fire_and_forget GetOrientation(winrt::Microsoft::ReactNative::ReactPromise<std::string_view> promise) noexcept;

        REACT_METHOD(GetDeviceOrientation, L"getDeviceOrientation");
        winrt::fire_and_forget GetDeviceOrientation(winrt::Microsoft::ReactNative::ReactPromise<std::string_view> promise) noexcept;

        REACT_METHOD(LockToPortrait, L"lockToPortrait");
        void LockToPortrait() noexcept;

        REACT_METHOD(LockToPortraitUpsideDown, L"lockToPortraitUpsideDown");
        void LockToPortraitUpsideDown() noexcept;

        REACT_METHOD(LockToLandscape, L"lockToLandscape");
        void LockToLandscape() noexcept;

        REACT_METHOD(LockToLandscapeRight, L"lockToLandscapeRight");
        void LockToLandscapeRight() noexcept;

        REACT_METHOD(LockToLandscapeLeft, L"lockToLandscapeLeft");
        void LockToLandscapeLeft() noexcept;

        REACT_METHOD(UnlockAllOrientations, L"unlockAllOrientations");
        void UnlockAllOrientations() noexcept;

        REACT_EVENT(OrientationDidChange, L"orientationDidChange");
        std::function<void(std::string_view)> OrientationDidChange;

        REACT_EVENT(DeviceOrientationDidChange, L"deviceOrientationDidChange");
        std::function<void(std::string_view)> DeviceOrientationDidChange;

        REACT_EVENT(LockDidChange, L"lockDidChange");
        std::function<void(std::string_view)> LockDidChange;

    private:
        winrt::fire_and_forget LockToMode(winrt::Windows::Graphics::Display::DisplayOrientations targetOrientation, std::string_view eventName = "");

        winrt::Microsoft::ReactNative::ReactContext m_reactContext;
                
        winrt::Windows::Devices::Sensors::SimpleOrientationSensor m_deviceOrientationSensor{ nullptr };
        winrt::Windows::Graphics::Display::DisplayInformation m_displayInfo{ nullptr };
        winrt::Windows::UI::ViewManagement::UIViewSettings m_viewSettings{ nullptr };

        std::string m_initialOrientation{ "UNKNOWN" };

        winrt::Windows::Devices::Sensors::SimpleOrientationSensor::OrientationChanged_revoker m_deviceOrientationChangedRevoker{};
        winrt::Windows::Graphics::Display::DisplayInformation::OrientationChanged_revoker m_orientationChangedRevoker{};
    };
}
