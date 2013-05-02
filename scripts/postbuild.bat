IF "%1" == "true" (
	signtool sign /v /f %ISLOGRepository%keys\islog_sign_binary.pfx /p %ISLOGSIGN% /t %ISLOGSignTimestampSrv% bin/Win32/Release/*.dll
	signtool sign /v /f %ISLOGRepository%keys\islog_sign_binary.pfx /p %ISLOGSIGN% /t %ISLOGSignTimestampSrv% bin/x64/Release/*.dll
	signtool sign /v /f %ISLOGRepository%keys\islog_sign_binary.pfx /p %ISLOGSIGN% /t %ISLOGSignTimestampSrv% bin/Win32/Release/*.exe
	signtool sign /v /f %ISLOGRepository%keys\islog_sign_binary.pfx /p %ISLOGSIGN% /t %ISLOGSignTimestampSrv% bin/x64/Release/*.exe
)