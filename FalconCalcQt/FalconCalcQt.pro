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
	..\src\calculate.h	\
	..\src\EngineErrors.h	\
	..\src\LongNumber.h	\
	..\src\SmartString.h	\
	AboutDialog.h	\
	FalconCalcQt.h	\
	FCSettings.h	\
	HelpDialog.h	\
	HistoryDialog.h	\
	HistoryOptions.h\
	LocaleDlg.h		\
	schemes.h		\
	VariablesFunctionsDialog.h
SOURCES += \
	..\src\calculate.cpp	\
	..\src\EngineErrors.cpp	\
	..\src\LongNumber.cpp	\
	..\src\SmartString.cpp	\
	FalconCalcQt.cpp         \
	FCSettings.cpp           \
	HelpDialog.cpp           \
	HistoryDialog.cpp        \
	HistoryOptions.cpp       \
	LocaleDlg.cpp            \
	main.cpp                 \
	schemes.cpp              \
	VariablesFunctionsDialog.cpp

FORMS += \
	AboutDialog.ui	\
	FalconCalcQt.ui	\
	HelpDialog.ui	\
	HistoryDialog.ui	\
	HistoryOptions.ui	\
	LocaleDlg.ui	\
	VariablesFunctionDialog.ui
	
RESOURCES += FalconCalcQt.qrc
INCLUDEPATH += .
QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wno-reorder
QT += gui widgets network
requires(qtConfig(filedialog))
TRANSLATIONS += ./translations/FalconCalcQt_en.ts \
			    ./translations/FalconCalcQt_hu.ts
