ALTER TABLE artery_run
ADD COLUMN `penetration_rate_oem` VARCHAR(20) NULL AFTER `date`,
ADD COLUMN `penetration_rate_non_oem` VARCHAR(20) NULL AFTER `penetration_rate_oem`;

### Config: LTEOnlyBS
UPDATE artery_run 
SET penetration_rate_oem = '0.054'
WHERE config = 'LTEOnlyBS' AND run_number = 0;

UPDATE artery_run 
SET penetration_rate_oem = '0.082'
WHERE config = 'LTEOnlyBS' AND run_number = 1;

UPDATE artery_run 
SET penetration_rate_oem = '0.109'
WHERE config = 'LTEOnlyBS' AND run_number = 2;

UPDATE artery_run 
SET penetration_rate_oem = '0.136'
WHERE config = 'LTEOnlyBS' AND run_number = 3;

UPDATE artery_run 
SET penetration_rate_oem = '0.163'
WHERE config = 'LTEOnlyBS' AND run_number = 4;

UPDATE artery_run 
SET penetration_rate_oem = '0.190'
WHERE config = 'LTEOnlyBS' AND run_number = 5;

UPDATE artery_run 
SET penetration_rate_oem = '0.218'
WHERE config = 'LTEOnlyBS' AND run_number = 6;

UPDATE artery_run 
SET penetration_rate_oem = '0.245'
WHERE config = 'LTEOnlyBS' AND run_number = 7;

UPDATE artery_run 
SET penetration_rate_oem = '0.272'
WHERE config = 'LTEOnlyBS' AND run_number = 8;

UPDATE artery_run 
SET penetration_rate_oem = '0.299'
WHERE config = 'LTEOnlyBS' AND run_number = 9;

UPDATE artery_run 
SET penetration_rate_oem = '0.326'
WHERE config = 'LTEOnlyBS' AND run_number = 10;

UPDATE artery_run 
SET penetration_rate_oem = '0.353'
WHERE config = 'LTEOnlyBS' AND run_number = 11;

UPDATE artery_run 
SET penetration_rate_oem = '0.381'
WHERE config = 'LTEOnlyBS' AND run_number = 12;

UPDATE artery_run 
SET penetration_rate_oem = '0.5'
WHERE config = 'LTEOnlyBS' AND run_number = 13;

UPDATE artery_run 
SET penetration_rate_oem = '0.75'
WHERE config = 'LTEOnlyBS' AND run_number = 14;

UPDATE artery_run 
SET penetration_rate_oem = '1'
WHERE config = 'LTEOnlyBS' AND run_number = 15;


### Config: Stage2BS
UPDATE artery_run 
SET penetration_rate_oem = '0.054', 
	penetration_rate_non_oem = '0.082' 
WHERE config = 'Stage2BS' AND run_number = 0;

UPDATE artery_run 
SET penetration_rate_oem = '0.082', 
	penetration_rate_non_oem = '0.124' 
WHERE config = 'Stage2BS' AND run_number = 1;

UPDATE artery_run 
SET penetration_rate_oem = '0.109', 
	penetration_rate_non_oem = '0.165' 
WHERE config = 'Stage2BS' AND run_number = 2;

UPDATE artery_run 
SET penetration_rate_oem = '0.136', 
	penetration_rate_non_oem = '0.206' 
WHERE config = 'Stage2BS' AND run_number = 3;

UPDATE artery_run 
SET penetration_rate_oem = '0.163', 
	penetration_rate_non_oem = '0.247' 
WHERE config = 'Stage2BS' AND run_number = 4;

UPDATE artery_run 
SET penetration_rate_oem = '0.190', 
	penetration_rate_non_oem = '0.288' 
WHERE config = 'Stage2BS' AND run_number = 5;

UPDATE artery_run 
SET penetration_rate_oem = '0.218', 
	penetration_rate_non_oem = '0.330' 
WHERE config = 'Stage2BS' AND run_number = 6;

UPDATE artery_run 
SET penetration_rate_oem = '0.245', 
	penetration_rate_non_oem = '0.371' 
WHERE config = 'Stage2BS' AND run_number = 7;

UPDATE artery_run 
SET penetration_rate_oem = '0.272', 
	penetration_rate_non_oem = '0.412' 
WHERE config = 'Stage2BS' AND run_number = 8;

UPDATE artery_run 
SET penetration_rate_oem = '0.299', 
	penetration_rate_non_oem = '0.453' 
WHERE config = 'Stage2BS' AND run_number = 9;

UPDATE artery_run 
SET penetration_rate_oem = '0.326', 
	penetration_rate_non_oem = '0.495' 
WHERE config = 'Stage2BS' AND run_number = 10;

UPDATE artery_run 
SET penetration_rate_oem = '0.353', 
	penetration_rate_non_oem = '0.536' 
WHERE config = 'Stage2BS' AND run_number = 11;

UPDATE artery_run 
SET penetration_rate_oem = '0.381', 
	penetration_rate_non_oem = '0.577' 
WHERE config = 'Stage2BS' AND run_number = 12;
