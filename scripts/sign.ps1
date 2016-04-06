$files = Get-ChildItem -path $args[0] -recurse -Include *.exe,*.dll

while ($true)
{
	& signtool sign /v /f $env:ISLOGRepository\keys\islog_sign_binary.pfx /p $env:ISLOGSIGN /t http://timestamp.verisign.com/scripts/timstamp.dll $files
	if ($LASTEXITCODE -eq 0) {
		break;
	}
}
while ($true)
{
	& signtool sign /as /fd SHA256 /f $env:ISLOGRepository\keys\islog_sign_binary.pfx /p $env:ISLOGSIGN /tr http://timestamp.globalsign.com/?signature=sha2 /td SHA256 $files
	if ($LASTEXITCODE -eq 0) {
		break;
	}
}



