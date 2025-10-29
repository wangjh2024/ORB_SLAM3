@echo off
echo Fixing all Boost serialization issues...

cd /d "E:\VS2022Project\ORB_SLAM3"

:: 创建修复头文件
echo #pragma once > fix_boost_serialization.h
echo. >> fix_boost_serialization.h
echo // 修复 Boost 序列化访问权限问题 >> fix_boost_serialization.h
echo #ifndef BOOST_SERIALIZATION_ACCESS >> fix_boost_serialization.h
echo #define BOOST_SERIALIZATION_ACCESS() ^\ >> fix_boost_serialization.h
echo     friend class boost::serialization::access; >> fix_boost_serialization.h
echo #endif >> fix_boost_serialization.h
echo. >> fix_boost_serialization.h
echo // 包含必要的 Boost 头文件 >> fix_boost_serialization.h
echo #include ^<boost/serialization/access.hpp^> >> fix_boost_serialization.h

:: 修复所有头文件
for %%f in (
    "include\Map.h"
    "include\KeyFrameDatabase.h"
    "include\KeyFrame.h"
    "include\MapPoint.h"
    "include\CameraModels\Pinhole.h"
    "include\CameraModels\KannalaBrandt8.h"
    "include\Atlas.h"
    "include\ImuTypes.h"
    "Thirdparty\DBoW2\DBoW2\BowVector.h"
    "Thirdparty\DBoW2\DBoW2\FeatureVector.h"
) do (
    echo Fixing %%f
    powershell -Command "$content = Get-Content '%%f'; if ($content -notcontains '#include \"fix_boost_serialization.h\"') { $content = '#include \"fix_boost_serialization.h\"', $content; $content | Set-Content '%%f' }"
)

echo Fix completed!
pause