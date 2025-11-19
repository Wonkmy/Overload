@echo off

call "%~dp0\Scripts\Windows\RunPremake5.bat" %*

exit /b %errorlevel%
