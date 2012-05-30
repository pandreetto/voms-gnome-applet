project(voms-gnome-applet)


add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-base.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-base.svg --export-png=voms-ac-base-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-expired.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-expired.svg --export-png=voms-ac-expired-22x22.png -w22 -h22)


add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-gray.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-gray.svg --export-png=voms-ac-gray-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-25.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-25.svg --export-png=voms-ac-25-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-50.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-50.svg --export-png=voms-ac-50-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-75.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-75.svg --export-png=voms-ac-75-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-22x22.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-100.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-100.svg --export-png=voms-ac-100-22x22.png -w22 -h22)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-base.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-base.svg --export-png=voms-ac-base-48x48.png -w48 -h48)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-expired.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-expired.svg --export-png=voms-ac-expired-48x48.png -w48 -h48)


add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-gray.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-gray.svg --export-png=voms-ac-gray-48x48.png -w48 -h48)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-25.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-25.svg --export-png=voms-ac-25-48x48.png -w48 -h48)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-50.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-50.svg --export-png=voms-ac-50-48x48.png -w48 -h48)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-75.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-75.svg --export-png=voms-ac-75-48x48.png -w48 -h48)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-48x48.png
                   DEPENDS ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-100.svg
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-100.svg --export-png=voms-ac-100-48x48.png -w48 -h48
                   COMMAND inkscape ${CMAKE_SOURCE_DIR}/icons/scalable/voms-ac-100.svg --export-png=voms-gnome-applet.png -w32 -h32)


add_custom_target(bitmaps ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-22x22.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-48x48.png
                                      ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-48x48.png)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-22x22.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-22x22.png
        DESTINATION ${ICON22_SUBDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-base-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-expired-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-gray-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-25-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-50-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-75-48x48.png
              ${CMAKE_CURRENT_BINARY_DIR}/voms-ac-100-48x48.png
        DESTINATION ${ICON48_SUBDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/voms-gnome-applet.png
        DESTINATION ${PIXMAP_DIR})




