find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(KF6 REQUIRED COMPONENTS I18n Config ConfigWidgets KCMUtils)

set(kcm_SRCS
    YetAnotherMagicLampEffectKCM.h
    YetAnotherMagicLampEffectKCM.cc
    plugin.cc
		metadata.json
)

kconfig_add_kcfg_files(kcm_SRCS
    ../YetAnotherMagicLampConfig.kcfgc
)

ki18n_wrap_ui(kcm_SRCS YetAnotherMagicLampEffectKCM.ui)

qt_add_dbus_interface(kcm_SRCS ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

kcoreaddons_add_plugin(kwin_yetanothermagiclamp_config
    SOURCES ${kcm_SRCS}
    INSTALL_NAMESPACE "kwin/effects/configs"
)

target_link_libraries(kwin_yetanothermagiclamp_config
		PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::DBus
    Qt6::Gui
    KF6::ConfigCore
    KF6::ConfigGui
		KF6::KCMUtils
    KF6::I18n
    KF6::CoreAddons
)
