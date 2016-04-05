IF "%1" == "true" (
	Powershell.exe -executionpolicy remotesigned -File .\scripts\sign.ps1 "%2"
)