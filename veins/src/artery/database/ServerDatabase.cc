/*
 * ServerDatabase.cpp
 *
 *  Created on: 30.09.2015
 *      Author: timpner
 */

#include <artery/database/ServerDatabase.h>

ServerDatabase::ServerDatabase() {
    try {
        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "omnetpp", "omnetpp");
        /* Connect to the MySQL test database */
        con->setSchema("artery");

        prep_stmt_insert_vehicle = con->prepareStatement("INSERT INTO vehicles(id, type, length) VALUES (?, ?, ?)");
        prep_stmt_insert_section = con->prepareStatement("INSERT INTO sections(road_id, lane_index, length) VALUES (?, ?, ?)");
        prep_stmt_select_section_id = con->prepareStatement("SELECT id FROM sections WHERE road_id = ? AND lane_index = ?");

        // NOTE: INSERT DELAYED is not supported on all engines, notably InnoDB.
        // It is best to create the tables with the ENGINE = MYISAM option.
        // TODO INSERT DELAYED necessary?
        prep_stmt_insert_traci = con->prepareStatement("INSERT INTO traci(vehicle, section, simtime, speed, position) VALUES (?, ?, ?, ?, ?)");
        prep_stmt_insert_report = con->prepareStatement("INSERT INTO reports(vehicle, section, speed, position, simtime_tx, simtime_rx, bytes) VALUES (?, ?, ?, ?, ?, ?, ?)");

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

ServerDatabase::~ServerDatabase() {
    delete prep_stmt_insert_vehicle;
    delete prep_stmt_insert_section;
    delete prep_stmt_insert_traci;
    delete prep_stmt_insert_report;
    delete prep_stmt_select_section_id;
    delete con;
}

void ServerDatabase::insertVehicle(std::string id, std::string type, double length) {
    try {

        prep_stmt_insert_vehicle->setString(1, id);
        prep_stmt_insert_vehicle->setString(2, type);
        prep_stmt_insert_vehicle->setDouble(3, length);
        prep_stmt_insert_vehicle->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

void ServerDatabase::insertSection(std::pair< std::string, int32_t > section, double length) {
    try {

        prep_stmt_insert_section->setString(1, section.first);
        prep_stmt_insert_section->setInt(2, section.second);
        prep_stmt_insert_section->setDouble(3, length);
        prep_stmt_insert_section->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

int32_t ServerDatabase::getSectionId(const std::pair<std::string, int32_t>& section) {

    prep_stmt_select_section_id->setString(1, section.first);
    prep_stmt_select_section_id->setInt(2, section.second);

    sql::ResultSet* res = prep_stmt_select_section_id->executeQuery();

    int32_t sectionId = -1;
    while (res->next()) {
        // We expect 1 result
        sectionId = res->getInt(1);
    }

    delete res;

    return sectionId;
}

void ServerDatabase::insertTraCI(std::string vehicleId, std::pair< std::string, int32_t > section, uint64_t simtime, double speed, double position) {
    try {

        int32_t sectionId = getSectionId(section);

        // (vehicle, section, simtime, speed, position)
        prep_stmt_insert_traci->setString(1, vehicleId);
        prep_stmt_insert_traci->setInt(2, sectionId);
        prep_stmt_insert_traci->setInt64(3, simtime);
        prep_stmt_insert_traci->setDouble(4, speed);
        prep_stmt_insert_traci->setDouble(5, position);

        prep_stmt_insert_traci->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

void ServerDatabase::insertLTEReport(LTEReport *report, uint64_t simtime_rx) {
    try {

        std::pair<std::string, int32_t> section(report->getRoadId(), report->getLaneIndex());
        int32_t sectionId = getSectionId(section);

        // (vehicle, section, speed, position, simtime_tx, simtime_rx, bytes)
        prep_stmt_insert_report->setString(1, report->getSrc());
        prep_stmt_insert_report->setInt(2, sectionId);
        prep_stmt_insert_report->setDouble(3, report->getSpeed());
        prep_stmt_insert_report->setDouble(4, report->getLanePosition());
        prep_stmt_insert_report->setInt64(5, report->getSendingTime().raw());
        prep_stmt_insert_report->setInt64(6, simtime_rx);
        prep_stmt_insert_report->setInt64(7, report->getByteLength());

        prep_stmt_insert_report->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__
                << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

sql::Connection* ServerDatabase::getConnection() {
    return con;
}
