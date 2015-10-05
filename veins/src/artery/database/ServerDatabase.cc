/*
 * ServerDatabase.cpp
 *
 *  Created on: 30.09.2015
 *      Author: timpner
 */

#include <sstream>
#include <artery/database/ServerDatabase.h>

Register_PerObjectConfigOption(CFGID_MARIADB_HOST,       "mysql-host",         KIND_NONE, CFG_STRING,   "\"\"",  "Hostname of mysql server");
Register_PerObjectConfigOption(CFGID_MARIADB_USER,       "mysql-user",         KIND_NONE, CFG_STRING,   "\"\"",  "User name for mysql server");
Register_PerObjectConfigOption(CFGID_MARIADB_PASSWD,     "mysql-password",     KIND_NONE, CFG_STRING,   "\"\"",  "Password for mysql server");
Register_PerObjectConfigOption(CFGID_MARIADB_DB,         "mysql-database",     KIND_NONE, CFG_STRING,   "\"\"",  "Database name");
Register_PerObjectConfigOption(CFGID_MARIADB_PORT,       "mysql-port",         KIND_NONE, CFG_INT,      "0",     "Port of mysql server");


ServerDatabase::ServerDatabase() {
    try {

        readConfig();

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect(fullHost, user, passwd);
        /* Connect to the MySQL test database */
        con->setSchema(db);

        prep_stmt_insert_run = con->prepareStatement("INSERT INTO artery_run(run_number, network, date) VALUES (?, ?, FROM_UNIXTIME(?))");
        prep_stmt_insert_vehicle = con->prepareStatement("INSERT INTO vehicles(id, type, length) VALUES (?, ?, ?)");
        prep_stmt_insert_section = con->prepareStatement("INSERT INTO sections(road_id, lane_index, length) VALUES (?, ?, ?)");
        prep_stmt_select_section_id = con->prepareStatement("SELECT id FROM sections WHERE road_id = ? AND lane_index = ?");
        prep_stmt_select_run_id = con->prepareStatement("SELECT id FROM artery_run WHERE run_number = ? AND network = ? AND date = FROM_UNIXTIME(?)");

        // NOTE: INSERT DELAYED is not supported on all engines, notably InnoDB.
        // It is best to create the tables with the ENGINE = MYISAM option.
        prep_stmt_insert_traci = con->prepareStatement("INSERT DELAYED INTO traci(vehicle, section, simtime, speed, position) VALUES (?, ?, ?, ?, ?)");
        prep_stmt_insert_report = con->prepareStatement("INSERT DELAYED INTO reports(vehicle, section, speed, position, simtime_tx, simtime_rx, bytes) VALUES (?, ?, ?, ?, ?, ?, ?)");

        storeRunId();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

ServerDatabase::~ServerDatabase() {
    delete prep_stmt_insert_run;
    delete prep_stmt_insert_vehicle;
    delete prep_stmt_insert_section;
    delete prep_stmt_insert_traci;
    delete prep_stmt_insert_report;
    delete prep_stmt_select_section_id;
    delete prep_stmt_select_run_id;
    delete con;
}


/**
 * Read config from omnetpp.ini
 */
void ServerDatabase::readConfig() {
    std::string cfgobj = "mysql";
    cConfiguration *cfg = ev.getConfig();

    std::string host = cfg->getAsString(cfgobj.c_str(), CFGID_MARIADB_HOST, NULL);
    user = cfg->getAsString(cfgobj.c_str(), CFGID_MARIADB_USER, NULL);
    passwd = cfg->getAsString(cfgobj.c_str(), CFGID_MARIADB_PASSWD, NULL);
    db = cfg->getAsString(cfgobj.c_str(), CFGID_MARIADB_DB, NULL);
    unsigned int port = (unsigned int) (cfg->getAsInt(cfgobj.c_str(), CFGID_MARIADB_PORT, 0));

    std::stringstream ss;
    ss << "tcp://" << host << ":" << port;
    fullHost = ss.str();
}

/**
 * Store run information in database and 'currentRunId' member.
 */
void ServerDatabase::storeRunId() {
    int runNumber = simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber();
    std::string network = simulation.getNetworkType()->getName();
    std::time_t date = std::time(nullptr);
    currentRunId = this->insertRun(runNumber, network, date);
}

int32_t ServerDatabase::insertRun(int number, std::string network, std::time_t date) {

    int32_t runId = -1;

    try {
        prep_stmt_insert_run->setInt(1, number);
        prep_stmt_insert_run->setString(2, network);
        prep_stmt_insert_run->setInt(3, date);

        prep_stmt_insert_run->executeUpdate();

        prep_stmt_select_run_id->setInt(1, number);
        prep_stmt_select_run_id->setString(2, network);
        prep_stmt_select_run_id->setInt(3, date);

        sql::ResultSet* res = prep_stmt_select_run_id->executeQuery();

        while (res->next()) {
            // We expect 1 result
            runId = res->getInt(1);
        }

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    return runId;
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
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

sql::Connection* ServerDatabase::getConnection() {
    return con;
}
