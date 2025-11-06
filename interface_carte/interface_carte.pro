QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mafenetre.cpp

HEADERS += \
    mafenetre.h

FORMS += \
    mafenetre.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/../projetQtcarte_deja_totalement_fonctionnel/Librairie+sample/LIB/ -lODALID_Education

INCLUDEPATH += $$PWD/../projetQtcarte_deja_totalement_fonctionnel/Librairie+sample/LIB
DEPENDPATH += $$PWD/../projetQtcarte_deja_totalement_fonctionnel/Librairie+sample/LIB

RESOURCES += \
    ressource.qrc
