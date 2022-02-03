#include "pch.h"
#include "OrientationWindows.h"

using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::Devices::Sensors;

namespace winrt
{
    /// <summary>
    /// Moves execution onto a ReactDispatcher's thread
    /// </summary>
    /// <param name="dispatcher">The dispatcher to resume execution on</param>
    /// <param name="tryImmediate">
    /// When true, if we're already execution on the dispatcher's thread it will continue execution immediately instead
    /// of scheduling onto the queue. When false, execution is always scheduled onto the back of the queue.
    /// </param>
    /// <returns></returns>
    [[nodiscard]] inline auto resume_foreground(
        winrt::Microsoft::ReactNative::ReactDispatcher const& dispatcher,
        bool tryImmediate)
    {
        struct awaitable
        {
            awaitable(winrt::Microsoft::ReactNative::ReactDispatcher const& dispatcher, bool tryImmediate) noexcept
                : m_dispatcher(dispatcher),
                m_tryImmediate(tryImmediate)
            {
            }

            bool await_ready() const noexcept
            {
                return m_tryImmediate ? m_dispatcher.HasThreadAccess() : false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::experimental::coroutine_handle<> handle) const
            {
                m_dispatcher.Post(
                    [handle]
                {
                    handle();
                });
            }

        private:
            winrt::Microsoft::ReactNative::ReactDispatcher m_dispatcher;
            bool m_tryImmediate;
        };

        return awaitable {dispatcher, tryImmediate};
    }
} // namespace winrt

namespace OrientationWindows {

    constexpr std::string_view OrientationToString(DisplayOrientations orientation) noexcept {
        switch (orientation) {
        case DisplayOrientations::Landscape:
            return "LANDSCAPE";
        case DisplayOrientations::Portrait:
            return "PORTRAIT";
        case DisplayOrientations::LandscapeFlipped:
            return "LANDSCAPE-RIGHT";
        case DisplayOrientations::PortraitFlipped:
            return "PORTRAIT-UPSIDEDOWN";
        }
        return "";
    }

    constexpr std::string_view DeviceOrientationToString(SimpleOrientation orientation) noexcept {
        switch (orientation) {
        case SimpleOrientation::Facedown:
            return "FACE-DOWN";
        case SimpleOrientation::Faceup:
            return "FACE-UP";
        case SimpleOrientation::NotRotated:
            return "LANDSCAPE";
        case SimpleOrientation::Rotated90DegreesCounterclockwise:
            return "PORTRAIT-UPSIDEDOWN";
        case SimpleOrientation::Rotated180DegreesCounterclockwise:
            return "LANDSCAPE-RIGHT";
        case SimpleOrientation::Rotated270DegreesCounterclockwise:
            return "PORTRAIT";
        }
        return "";
    }

    void OrientationLockerModule::Initialize(React::ReactContext const& reactContext) noexcept {
        m_reactContext = reactContext;

        // We need to be on the UI thread to access Sensor data
        m_initializer = InitOnUI();
    }

    winrt::IAsyncAction OrientationLockerModule::InitOnUI() {
        // Keep 'this' alive through entirety of async operation
        auto strong_this = shared_from_this();

        co_await winrt::resume_foreground(m_reactContext.UIDispatcher(), true);

        m_deviceOrientationSensor = SimpleOrientationSensor::GetDefault();
        if (m_deviceOrientationSensor != nullptr)
        {
            m_deviceOrientationChangedRevoker = m_deviceOrientationSensor.OrientationChanged(winrt::auto_revoke, [fireEvent = DeviceOrientationDidChange](SimpleOrientationSensor const& sensor, SimpleOrientationSensorOrientationChangedEventArgs const&) {
                fireEvent(DeviceOrientationToString(sensor.GetCurrentOrientation()));
            });
        }

        m_displayInfo = DisplayInformation::GetForCurrentView();
        m_initialOrientation = OrientationToString(m_displayInfo.CurrentOrientation());
        m_orientationChangedRevoker = m_displayInfo.OrientationChanged(winrt::auto_revoke, [fireEvent = OrientationDidChange](DisplayInformation const& displayInfo, winrt::Windows::Foundation::IInspectable const&) {
            fireEvent(OrientationToString(displayInfo.CurrentOrientation()));
        });
    }

    void OrientationLockerModule::GetConstants(React::ReactConstantProvider& provider) noexcept {
        // Block until initializer completes
        if (m_initializer) {
            m_initializer.get();
            m_initializer = nullptr;
        }

        provider.Add(L"initialOrientation", m_initialOrientation);
    }

    winrt::fire_and_forget OrientationLockerModule::GetOrientation(winrt::Microsoft::ReactNative::ReactPromise<std::string_view> promise) noexcept {
        co_await winrt::resume_foreground(m_reactContext.UIDispatcher(), true);

        auto displayInfo = DisplayInformation::GetForCurrentView();

        promise.Resolve(OrientationToString(displayInfo.CurrentOrientation()));
    }

    winrt::fire_and_forget OrientationLockerModule::GetDeviceOrientation(winrt::Microsoft::ReactNative::ReactPromise<std::string_view> promise) noexcept {
        auto sensor = m_deviceOrientationSensor;
        if (sensor)
        {
            co_await winrt::resume_foreground(m_reactContext.UIDispatcher(), true);

            promise.Resolve(DeviceOrientationToString(sensor.GetCurrentOrientation()));
        }
        else {
            // No Orientation Sensor found on device
            promise.Resolve("UNKNOWN");
        }
    }

    winrt::fire_and_forget OrientationLockerModule::LockToMode(DisplayOrientations targetOrientation, std::string_view eventName) {

        auto fireEvent = LockDidChange;

        co_await winrt::resume_foreground(m_reactContext.UIDispatcher(), true);

        auto viewSettings = UIViewSettings::GetForCurrentView();
        auto displayInfo = DisplayInformation::GetForCurrentView();

        const auto mode = viewSettings.UserInteractionMode();
        if (mode == UserInteractionMode::Touch) {
            if (displayInfo.AutoRotationPreferences() != targetOrientation) {
                DisplayInformation::AutoRotationPreferences(targetOrientation);
                fireEvent(eventName.size() > 0 ? eventName : OrientationToString(targetOrientation));
            }
        }
    }

    void OrientationLockerModule::LockToPortrait() noexcept {
        LockToMode(DisplayOrientations::Portrait);
    }

    void OrientationLockerModule::LockToPortraitUpsideDown() noexcept {
        LockToMode(DisplayOrientations::PortraitFlipped);
    }

    void OrientationLockerModule::LockToLandscape() noexcept {
        LockToMode(DisplayOrientations::Landscape);
    }

    void OrientationLockerModule::LockToLandscapeRight() noexcept {
        LockToMode(DisplayOrientations::LandscapeFlipped);
    }

    void OrientationLockerModule::LockToLandscapeLeft() noexcept {
        LockToMode(DisplayOrientations::Landscape, "LANDSCAPE-LEFT");
    }

    void OrientationLockerModule::UnlockAllOrientations() noexcept {
        LockToMode(DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped | DisplayOrientations::Portrait | DisplayOrientations::PortraitFlipped, "UNLOCK");
    }
}