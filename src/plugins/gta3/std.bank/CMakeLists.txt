add_library(std.bank SHARED main.cpp CAECustomBankLoader.hpp CAECustomBankLoader.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std.bank RUNTIME DESTINATION "./modloader/.data/plugins/gta3")

