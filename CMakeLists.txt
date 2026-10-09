zephyr_library()
zephyr_library_sources_ifdef(CONFIG_REBOOT src/reboot_on_fatal.c)

# zmkfirmware/zmk#3522. Remove once fixed upstream.
if(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
  zephyr_library_sources(src/split_source_guard.c)
  zephyr_link_libraries(-Wl,--wrap=zmk_split_transport_central_peripheral_event_handler)
endif()
