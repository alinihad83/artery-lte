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
    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

ServerDatabase::~ServerDatabase() {
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

sql::Connection* ServerDatabase::getConnection() {

    delete prep_stmt_insert_vehicle;
    return con;
}
