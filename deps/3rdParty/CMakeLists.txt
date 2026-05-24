# ##################################################################################################
# 3rdParty packages
# ##################################################################################################
if(NOT CMAKE_CROSSCOMPILING)
    find_package(GTest REQUIRED)
endif()

# ##################################################################################################
# Promote global visibility for 3rd-party libs
# ##################################################################################################
if(NOT CMAKE_CROSSCOMPILING)
    set_target_properties(GTest::gmock PROPERTIES IMPORTED_GLOBAL TRUE)
    set_target_properties(GTest::gmock_main PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
