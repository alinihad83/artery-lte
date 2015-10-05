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

class ServerDatabase {

protected:
    sql::Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *prep_stmt_insert_run;
    sql::PreparedStatement *prep_stmt_insert_vehicle;
    sql::PreparedStatement *prep_stmt_insert_section;
    sql::PreparedStatement *prep_stmt_insert_traci;
    sql::PreparedStatement *prep_stmt_insert_report;
    sql::PreparedStatement *prep_stmt_select_section_id;
    sql::PreparedStatement *prep_stmt_select_run_id;

public:
    ServerDatabase();
    virtual ~ServerDatabase();
    virtual sql::Connection* getConnection();
    virtual int32_t insertRun(int number, std::string network, std::time_t date);
    virtual void insertVehicle(std::string id, std::string type, double length);
    virtual void insertSection(std::pair< std::string, int32_t > section, double length);
    virtual void insertTraCI(std::string vehicleId, std::pair< std::string, int32_t > section, uint64_t simtime, double speed, double position);
    virtual void insertLTEReport(LTEReport *report, uint64_t simtime_rx);

private:
    int32_t currentRunId;
    int32_t getSectionId(const std::pair<std::string, int32_t>& section);
};

#endif /* SERVERDATABASE_H_ */
