@echo off
echo Fixing Boost serialization issues...

REM 修改所有包含BOOST_SERIALIZATION_ACCESS的头文件
for /r %%f in (*.h) do (
    powershell -Command "(Get-Content '%%f') -replace 'BOOST_SERIALIZATION_ACCESS', 'friend class boost::serialization::access;' | Set-Content '%%f'"
)

echo Boost serialization fixes applied!
pause