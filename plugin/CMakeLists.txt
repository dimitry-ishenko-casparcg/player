# plugin

include(GNUInstallDirs)
install(FILES gnome/casparcg-player.py
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nautilus-python/extensions
)
install(FILES kde/casparcg-player.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kio/servicemenus
)
