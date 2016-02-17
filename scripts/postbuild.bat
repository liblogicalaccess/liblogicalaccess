IF "%1" == "true" (
	for /r %2 %%d in (*.dll *.exe) do (
	:while1
	fsign "%%d"
		@if %ERRORLEVEL% GEQ 1 (
			sleep 1
			goto :while1
		)
	)
)