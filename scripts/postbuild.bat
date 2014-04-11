IF "%1" == "true" (
	for /r %2 %%d in (*.dll *.exe) do (
	:while1
	signtool sign /v /f %ISLOGRepository%keys\islog_sign_binary.pfx /p %ISLOGSIGN% /t %ISLOGSignTimestampSrv% "%%d"
		@if %ERRORLEVEL% GEQ 1 (
			sleep 1
			goto :while1
		)
	)
)