/*
 * ServerDatabase.h
 *
 *  Created on: 30.09.2015
 *      Author: timpner
 */

#ifndef SERVERDATABASE_H_
#define SERVERDATABASE_H_

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>
#include <ctime>
#include <artery/messages/LTEReport_m.h>

/**
 * @brief
 * A database utility that provides storage and retrieval of telemetry information received from vehicles, as well as ground truth data from TraCI.
 *
 * @author Julian Timpner
 */
class ServerDatabase {

protected:
    sql::Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *prepStmtInsertRun;
    sql::PreparedStatement *prepStmtInsertVehicle;
    sql::PreparedStatement *prepStmtInsertSection;
    sql::PreparedStatement *prepStmtInsertTraci;
    sql::PreparedStatement *prepStmtInsertReport;
    sql::PreparedStatement *prepStmtSelectSectionId;
    sql::PreparedStatement *prepStmtSelectVehicleId;
    sql::PreparedStatement *prepStmtSelectRunId;

public:
    ServerDatabase();
    virtual ~ServerDatabase();
    virtual sql::Connection* getConnection();
    virtual int32_t insertRun(int number, std::string network, std::time_t date);
    virtual void insertVehicle(std::string id, std::string type, double length);
    virtual void insertSection(std::pair< std::string, int32_t > section, double length);
    virtual void insertTraCI(std::string vehicleNodeId, std::pair< std::string, int32_t > section, uint64_t simtime, double speed, double positionLane, double positionX, double positionY);
    virtual void insertLTEReport(LTEReport *report, uint64_t simtimeRX);

private:
    std::string fullHost;
    std::string user;
    std::string passwd;
    std::string db;
    int32_t currentRunId;
    int32_t getSectionId(const std::pair<std::string, int32_t>& section);
    int32_t getVehicleId(const std::string vehicleNodeId);
    void readConfig();
    void storeRunId();
};

#endif /* SERVERDATABASE_H_ */
