add_subdirectory(kcm)

set(effect_SRCS
		Model.cc
		YetAnotherMagicLampEffect.cc
		plugin.cc
)

kconfig_add_kcfg_files(effect_SRCS
		YetAnotherMagicLampConfig.kcfgc
)

add_library(kwin_effect_yetanothermagiclamp SHARED ${effect_SRCS})

target_include_directories(kwin_effect_yetanothermagiclamp PRIVATE
		${Qt6Core_INCLUDE_DIRS}
		${Qt6Gui_INCLUDE_DIRS}
		${KF6ConfigCore_INCLUDE_DIRS}
		${KF6ConfigGui_INCLUDE_DIRS}
		${KF6CoreAddons_INCLUDE_DIRS}
		${KWin_INCLUDE_DIRS}
)

target_link_libraries(kwin_effect_yetanothermagiclamp
		Qt6::Core
		Qt6::Gui
		KF6::ConfigCore
		KF6::ConfigGui
		KF6::CoreAddons
		KF6::Svg
		KWin::kwin
		Plasma::Plasma
)

install(
		TARGETS
				kwin_effect_yetanothermagiclamp

		DESTINATION
				${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins/
)
