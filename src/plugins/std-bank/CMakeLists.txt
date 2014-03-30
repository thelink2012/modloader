add_library(std-bank SHARED bank.cpp CAECustomBankLoader.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-bank RUNTIME DESTINATION "./modloader/.data/plugins")

