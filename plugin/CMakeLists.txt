# plugin

configure_file(gnome/player.py.in gnome/${NAME}.py @ONLY)
configure_file(kde/player.desktop.in kde/${NAME}.desktop @ONLY)

include(GNUInstallDirs)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gnome/${NAME}.py
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nautilus-python/extensions
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kde/${NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kio/servicemenus
)
