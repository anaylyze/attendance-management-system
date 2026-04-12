#include "manager/StudentRegistry.h"
#include "manager/AttendanceManager.h"
#include "service/FileManager.h"
#include "service/Logger.h"
#include "service/ReportGenerator.h"
#include "ui/MenuController.h"

#include <iostream>
#include <sys/stat.h>
#include <exception>

#ifdef _WIN32
#  include <direct.h>
#  define AMS_MKDIR(p) _mkdir(p)
#else
#  include <sys/types.h>
#  define AMS_MKDIR(p) mkdir((p), 0755)
#endif

int main() {
    const std::string dataDir    = "data";
    const std::string reportsDir = "reports";
    const std::string logPath    = dataDir + "/system.log";

    AMS_MKDIR(dataDir.c_str());
    AMS_MKDIR(reportsDir.c_str());

    ams::Logger::instance().init(logPath);
    ams::Logger::instance().info("=== Application starting ===");

    try {
        ams::StudentRegistry   registry;
        ams::AttendanceManager attendance;
        ams::FileManager       fileManager(dataDir);
        ams::ReportGenerator   reportGen(reportsDir);

        ams::MenuController controller(registry, attendance, fileManager, reportGen);
        controller.run();

    } catch (const std::exception& e) {
        ams::Logger::instance().error(
            std::string("Unhandled exception: ") + e.what());
        std::cerr << "\n[FATAL] Unhandled exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        ams::Logger::instance().error("Unknown unhandled exception");
        std::cerr << "\n[FATAL] Unknown exception.\n";
        return 2;
    }

    ams::Logger::instance().info("=== Application exited cleanly ===");
    return 0;
}
