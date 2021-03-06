cmake_minimum_required(VERSION 3.5)
set(KDEPIM_VERSION_NUMBER "5.11.40")
project(akonadiconsole VERSION ${KDEPIM_VERSION_NUMBER})

set(KF5_MIN_VERSION "5.56.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(CheckFunctionExists)
include(ECMGeneratePriFile)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)


# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
    set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()


set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")

set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")

set(AKONADI_CONTACT_VERSION "5.10.90")
set(CALENDARSUPPORT_LIB_VERSION_LIB "5.10.90")
set(AKONADI_VERSION "5.10.90")
set(AKONADI_SEARCH_VERSION "5.10.90")

set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")


set(QT_REQUIRED_VERSION "5.10.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets DBus Sql Test)
set(MESSAGELIB_LIB_VERSION_LIB "5.10.90")
set(LIBKDEPIM_LIB_VERSION_LIB "5.10.90")
set(KCALENDARCORE_LIB_VERSION "5.10.90")
set(KCONTACTS_LIB_VERSION "5.10.90")
set(KMIME_LIB_VERSION "5.10.90")

# Find KF5 package
find_package(KF5Completion ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5ItemModels ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5TextWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5WidgetsAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5Completion ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} REQUIRED)

# Find KdepimLibs Package
find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)

if (NOT WIN32)
    find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5AkonadiSearch ${AKONADI_SEARCH_VERSION} CONFIG REQUIRED)

    find_package(Xapian CONFIG)
    set_package_properties(Xapian PROPERTIES
        DESCRIPTION "The Xapian search engine library"
        URL "http://xapian.org"
        TYPE REQUIRED
    )

    set(ENABLE_LIBKDEPIM TRUE)
    set(ENABLE_SEARCH TRUE)
    set(ENABLE_CONTENTVIEWS TRUE)
endif()

include_directories(${akonadiconsole_SOURCE_DIR} ${akonadiconsole_BINARY_DIR} ${XAPIAN_INCLUDE_DIR})

configure_file(config-akonadiconsole.h.cmake ${CMAKE_BINARY_DIR}/config-akonadiconsole.h)

install( FILES akonadiconsole.renamecategories akonadiconsole.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
