ALTER TABLE reports
	ADD COLUMN from_vehicle INT UNSIGNED NULL AFTER vehicle,
	ADD COLUMN via_report INT UNSIGNED NULL AFTER from_vehicle,
	ADD CONSTRAINT `fk_reports_from_vehicle`
		FOREIGN KEY (from_vehicle) REFERENCES vehicles (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT,
	ADD CONSTRAINT `fk_reports_via_report`
		FOREIGN KEY (via_report) REFERENCES reports (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT
	;
