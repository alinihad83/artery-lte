#
# Create tables for ArteryLTE.
#
# Note: Using CamelCase in database schemes is a bad idea, as it might lead to incompatibilities between different DBMS. Hence, the naming scheme differs from code style.

DROP TABLE IF EXISTS traci;
DROP TABLE IF EXISTS reports;
DROP TABLE IF EXISTS vehicles;
DROP TABLE IF EXISTS sections;
DROP TABLE IF EXISTS artery_run;
DROP TABLE IF EXISTS meta;

CREATE TABLE artery_run (
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     config VARCHAR(100) NOT NULL,
     run_number INT UNSIGNED NOT NULL,
     network VARCHAR(80) NOT NULL,
     date TIMESTAMP NOT NULL
);

CREATE TABLE vehicles ( 
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     node VARCHAR(80) NOT NULL,
     type VARCHAR(80),
     length DOUBLE PRECISION NOT NULL,
     runid INT UNSIGNED NOT NULL,
     UNIQUE(node,runid),
     CONSTRAINT `fk_vehicles_run`
          FOREIGN KEY (runid) REFERENCES artery_run (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT
);

CREATE TABLE sections ( 
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     road_id VARCHAR(80) NOT NULL,
     lane_index INT UNSIGNED NOT NULL,
     length DOUBLE PRECISION NOT NULL, 
     runid INT UNSIGNED NOT NULL,
     UNIQUE(road_id,lane_index,runid),
     CONSTRAINT `fk_sections_run`
          FOREIGN KEY (runid) REFERENCES artery_run (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT
);

CREATE TABLE traci (
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     runid INT UNSIGNED NOT NULL,
     simtime BIGINT UNSIGNED NOT NULL,
     vehicle INT UNSIGNED NOT NULL, 
     speed DOUBLE PRECISION NOT NULL, 
     section INT UNSIGNED NOT NULL, 
     position_lane DOUBLE PRECISION NOT NULL,
     position_x DOUBLE PRECISION NOT NULL,
     position_y DOUBLE PRECISION NOT NULL,
     CONSTRAINT `fk_traci_vehicle`
          FOREIGN KEY (vehicle) REFERENCES vehicles (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT,
     CONSTRAINT `fk_traci_section`
          FOREIGN KEY (section) REFERENCES sections (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT,
     CONSTRAINT `fk_traci_run`
          FOREIGN KEY (runid) REFERENCES artery_run (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT
); #ENGINE = MYISAM for INSERT DELAYED

CREATE TABLE reports (
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     runid INT UNSIGNED NOT NULL,
     vehicle INT UNSIGNED NOT NULL, 
     section INT UNSIGNED NOT NULL, 
     speed DOUBLE PRECISION NOT NULL, 
     position_lane DOUBLE PRECISION NOT NULL,
     position_x DOUBLE PRECISION NOT NULL,
     position_y DOUBLE PRECISION NOT NULL,
     simtime_tx BIGINT UNSIGNED NOT NULL,
     simtime_rx BIGINT UNSIGNED NOT NULL,
     bytes BIGINT UNSIGNED,
     CONSTRAINT `fk_reports_vehicle`
          FOREIGN KEY (vehicle) REFERENCES vehicles (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT,
     CONSTRAINT `fk_reports_section`
          FOREIGN KEY (section) REFERENCES sections (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT,
     CONSTRAINT `fk_reports_run`
          FOREIGN KEY (runid) REFERENCES artery_run (id)
          ON DELETE CASCADE
          ON UPDATE RESTRICT
#     via INT UNSIGNED
); #ENGINE = MYISAM for INSERT DELAYED

CREATE TABLE meta (
     id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
     name VARCHAR(200) NOT NULL,
     value VARCHAR(200) NOT NULL
);
