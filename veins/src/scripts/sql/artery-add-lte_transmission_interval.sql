ALTER TABLE artery_run
ADD COLUMN `lte_transmission_interval` VARCHAR(20) NOT NULL DEFAULT '3.6s' AFTER `penetration_rate_non_oem`;

