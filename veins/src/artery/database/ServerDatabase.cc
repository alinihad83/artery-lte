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

        // Connect to MariaDB
        driver = get_driver_instance();
        con = driver->connect(fullHost, user, passwd);
        con->setSchema(db);

        prepStmtInsertRun = con->prepareStatement("INSERT INTO artery_run(run_number, config, network, date, penetration_rate_oem, penetration_rate_non_oem, lte_transmission_interval) VALUES (?, ?, ?, FROM_UNIXTIME(?), ?, ?, ?)");
        prepStmtInsertVehicle = con->prepareStatement("INSERT INTO vehicles(node, type, length, runid) VALUES (?, ?, ?, ?)");
        prepStmtInsertSection = con->prepareStatement("INSERT INTO sections(road_id, lane_index, length, runid) VALUES (?, ?, ?, ?)");
        prepStmtSelectSectionId = con->prepareStatement("SELECT id FROM sections WHERE road_id = ? AND lane_index = ? AND runid = ?");
        prepStmtSelectVehicleId = con->prepareStatement("SELECT id FROM vehicles WHERE node = ? AND runid = ?");
        prepStmtSelectRunId = con->prepareStatement("SELECT id FROM artery_run WHERE run_number = ? AND network = ? AND date = FROM_UNIXTIME(?)");

        /* NOTE: INSERT DELAYED is not supported on all engines, notably InnoDB.
        It is best to create the tables with the ENGINE = MYISAM option.
        When a client uses INSERT DELAYED, it gets an okay from the server at once,
        and the row is queued to be inserted when the table is not in use by any other thread.

        Another major benefit of using INSERT DELAYED is that inserts from many clients are bundled
        together and written in one block. This is much faster than performing many separate inserts.

        Note that INSERT DELAYED is slower than a normal  INSERT if the table is not otherwise in use.
        There is also the additional overhead for the server to handle a separate thread for each table
        for which there are delayed rows. This means that you should use INSERT DELAYED only when you are
        really sure that you need it.

        cf. https://mariadb.com/kb/en/mariadb/insert-delayed/
        */
        prepStmtInsertTraci = con->prepareStatement("INSERT INTO traci(runid, vehicle, section, simtime, speed, position_lane, position_x, position_y) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        prepStmtInsertReport = con->prepareStatement("INSERT INTO reports(runid, vehicle, section, speed, position_lane, simtime_tx, simtime_rx, bytes, position_x, position_y) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

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
    delete prepStmtInsertRun;
    delete prepStmtInsertVehicle;
    delete prepStmtInsertSection;
    delete prepStmtInsertTraci;
    delete prepStmtInsertReport;
    delete prepStmtSelectSectionId;
    delete prepStmtSelectRunId;
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
    std::string configName = simulation.getActiveEnvir()->getConfigEx()->getActiveConfigName();
    std::string network = simulation.getNetworkType()->getName();

    const char* penetrationRateOEM = simulation.getActiveEnvir()->getConfigEx()->getVariable("penetrationRateOEM");
    const char* penetrationRateNonOEM = simulation.getActiveEnvir()->getConfigEx()->getVariable("penetrationRateNonOEM");
    const char* lteTransmissionInterval = simulation.getActiveEnvir()->getConfigEx()->getVariable("lteTransmissionInterval");

    std::time_t date = std::time(nullptr);

    currentRunId = this->insertRun(runNumber, configName, network, date, penetrationRateOEM, penetrationRateNonOEM, lteTransmissionInterval);
}

int32_t ServerDatabase::insertRun(int number, std::string configName, std::string network, std::time_t date, const char* penetrationRateOEM, const char* penetrationRateNonOEM, char const *lteTransmissionInterval) {

    int32_t runId = -1;

    try {
        prepStmtInsertRun->setInt(1, number);
        prepStmtInsertRun->setString(2, configName);
        prepStmtInsertRun->setString(3, network);
        prepStmtInsertRun->setInt(4, date);
        prepStmtInsertRun->setString(5, penetrationRateOEM);
        prepStmtInsertRun->setString(6, penetrationRateNonOEM);
        prepStmtInsertRun->setString(7, lteTransmissionInterval);

        prepStmtInsertRun->executeUpdate();

        prepStmtSelectRunId->setInt(1, number);
        prepStmtSelectRunId->setString(2, network);
        prepStmtSelectRunId->setInt(3, date);

        sql::ResultSet* res = prepStmtSelectRunId->executeQuery();

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


void ServerDatabase::insertVehicle(std::string node, std::string type, double length) {
    try {
        prepStmtInsertVehicle->setString(1, node);
        prepStmtInsertVehicle->setString(2, type);
        prepStmtInsertVehicle->setDouble(3, length);
        prepStmtInsertVehicle->setInt(4, currentRunId);

        prepStmtInsertVehicle->executeUpdate();

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
        prepStmtInsertSection->setString(1, section.first);
        prepStmtInsertSection->setInt(2, section.second);
        prepStmtInsertSection->setDouble(3, length);
        prepStmtInsertSection->setInt(4, currentRunId);

        prepStmtInsertSection->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}


int32_t ServerDatabase::getSectionId(const std::pair<std::string, int32_t>& section) {

    prepStmtSelectSectionId->setString(1, section.first);
    prepStmtSelectSectionId->setInt(2, section.second);
    prepStmtSelectSectionId->setInt(3, currentRunId);

    sql::ResultSet* res = prepStmtSelectSectionId->executeQuery();

    int32_t sectionId = -1;
    while (res->next()) {
        // We expect 1 result
        sectionId = res->getInt(1);
    }

    delete res;

    return sectionId;
}

int32_t ServerDatabase::getVehicleId(const std::string vehicleNodeId) {

    prepStmtSelectVehicleId->setString(1, vehicleNodeId);
    prepStmtSelectVehicleId->setInt(2, currentRunId);

    sql::ResultSet* res = prepStmtSelectVehicleId->executeQuery();

    int32_t vehicleId = -1;
    while (res->next()) {
        // We expect 1 result
        vehicleId = res->getInt(1);
    }

    delete res;

    return vehicleId;
}

void ServerDatabase::insertTraCI(std::string vehicleNodeId, std::pair< std::string, int32_t > section, uint64_t simtime, double speed, double positionLane, double positionX, double positionY) {
    try {
        int32_t sectionId = getSectionId(section);
        int32_t vehicleId = getVehicleId(vehicleNodeId);

        // Schema: (runid, vehicle, section, simtime, speed, position_lane, position_x, position_y)
        prepStmtInsertTraci->setInt(1, currentRunId);
        prepStmtInsertTraci->setInt(2, vehicleId);
        prepStmtInsertTraci->setInt(3, sectionId);
        prepStmtInsertTraci->setInt64(4, simtime);
        prepStmtInsertTraci->setDouble(5, speed);
        prepStmtInsertTraci->setDouble(6, positionLane);
        prepStmtInsertTraci->setDouble(7, positionX);
        prepStmtInsertTraci->setDouble(8, positionY);

        prepStmtInsertTraci->executeUpdate();

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

void ServerDatabase::insertLTEReport(LTEReport *report, uint64_t simtimeRX) {
    try {
        std::pair<std::string, int32_t> section(report->getRoadId(), report->getLaneIndex());
        int32_t sectionId = getSectionId(section);
        int32_t vehicleId = getVehicleId(report->getSrc());

        // Schema: (runid, vehicle, section, speed, position_lane, simtime_tx, simtime_rx, bytes, position_x, position_y)
        prepStmtInsertReport->setInt(1, currentRunId);
        prepStmtInsertReport->setInt(2, vehicleId);
        prepStmtInsertReport->setInt(3, sectionId);
        prepStmtInsertReport->setDouble(4, report->getSpeed());
        prepStmtInsertReport->setDouble(5, report->getLanePosition());
        prepStmtInsertReport->setInt64(6, report->getSendingTime().raw());
        prepStmtInsertReport->setInt64(7, simtimeRX);
        prepStmtInsertReport->setInt64(8, report->getByteLength());
        prepStmtInsertReport->setDouble(9, report->getXPosition());
        prepStmtInsertReport->setDouble(10, report->getYPosition());

        prepStmtInsertReport->executeUpdate();

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
