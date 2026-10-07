if(CONFIG_ZMK_BATTERY_NRF_VDDH_MV)
  zephyr_library()
  zephyr_library_sources(drivers/battery_nrf_vddh_mv.c)
endif()
