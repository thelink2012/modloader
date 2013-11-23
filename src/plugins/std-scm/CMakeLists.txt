add_library(std-scm SHARED scm.cpp)
install(TARGETS std-scm RUNTIME DESTINATION "./modloader/.data/plugins")
