add_library(std-bank SHARED bank.cpp CAECustomBankLoader.cpp)
install(TARGETS std-bank RUNTIME DESTINATION "./modloader/.data/plugins")

