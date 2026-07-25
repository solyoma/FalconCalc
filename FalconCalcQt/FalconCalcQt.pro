# ----------------------------------------------------
# FalconCalcQt GUI application
# ------------------------------------------------------

TARGET = FalconCalcQt
DESTDIR = ../Debug
CONFIG += debug
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += ./Debug/moc
OBJECTS_DIR += ./Debug
UI_DIR += ./Debug/Ui
RCC_DIR += ./Debug/rcc
win32:RC_FILE = FalconCalcQt.rc
HEADERS += \
	../common/calculate.h	\
	../common/defines.h	\
	../common/common.h		\
	../common/EngineErrors.h	\
	../common/LongNumber.h	\
	../common/version.h	\
	AboutDialog.h		\
	FalconCalcQt.h		\
	FCSettings.h		\
	HelpDialog.h		\
	HistoryDialog.h		\
	HistoryOptions.h	\
	LocaleDlg.h		\
	resource.h		\
	schemes.h		\
	SmartStringQt.h		\
	VarFuncDefDialog.h	\
	VariablesFunctionsDialog.h
SOURCES += \
	../common/calculate.cpp	\
	../common/EngineErrors.cpp	\
	../common/LongNumber.cpp	\
	FalconCalcQt.cpp        \
	FCSettings.cpp          \
	HelpDialog.cpp          \
	HistoryDialog.cpp       \
	HistoryOptions.cpp      \
	LocaleDlg.cpp           \
	main.cpp                \
	schemes.cpp             \
	VariablesFunctionsDialog.cpp

FORMS += \
	AboutDialog.ui		\
	FalconCalcQt.ui		\
	HelpDialog.ui		\
	HistoryDialog.ui	\
	HistoryOptions.ui	\
	LocaleDlg.ui		\
	VarFuncDefDialog.ui	\
	VariablesFunctionsDialog.ui
	
RESOURCES += FalconCalcQt.qrc
INCLUDEPATH += ../common
DEFINES += QTSA_PROJECT
RC_FILE += FalconCalcQt.rc
QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wno-reorder
QT += gui widgets network
requires(qtConfig(filedialog))
TRANSLATIONS += ./translations/FalconCalcQt_en.ts \
			    ./translations/FalconCalcQt_hu.ts
