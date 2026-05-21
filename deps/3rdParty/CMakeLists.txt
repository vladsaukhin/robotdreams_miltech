# ##################################################################################################
# 3rdParty packages
# ##################################################################################################

find_package(GTest REQUIRED)

# ##################################################################################################
# Promote global visibility for 3rd-party libs
# ##################################################################################################
set_target_properties(GTest::gmock PROPERTIES IMPORTED_GLOBAL TRUE)
set_target_properties(GTest::gmock_main PROPERTIES IMPORTED_GLOBAL TRUE)
