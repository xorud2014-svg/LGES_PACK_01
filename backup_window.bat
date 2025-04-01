@echo off

echo start

REM save the path
set backup_program_dir=%cd%\backup_program

echo %backup_program_dir%

REM setting config_backup path
set config_backup=%backup_program_dir%\config_backup
set current_backup=%backup_program_dir%\current_backup

REM generate current_backup.tar.gz
tar cfz %backup_program_dir%\current_backup.tar.gz -C %current_backup% .

REM generate config_backup.tar.gz
tar cfz %backup_program_dir%\config_backup.tar.gz -C %config_backup% .

REM generate backup_program.tar.gz in the parent directory
tar cfz %backup_program_dir%\..\backup_program.tar.gz -C %backup_program_dir% current_backup.tar.gz config_backup.tar.gz

REM delete temp tar.gz
del %backup_program_dir%\current_backup.tar.gz
del %backup_program_dir%\config_backup.tar.gz

echo done