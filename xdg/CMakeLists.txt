# xdg

include(GNUInstallDirs)

if(WIN32)
    set(SYSCONFDIR "C:/ProgramData")
else()
    set(SYSCONFDIR "/etc/xdg")
endif()

install(FILES player.conf DESTINATION ${SYSCONFDIR}/${NAME})
