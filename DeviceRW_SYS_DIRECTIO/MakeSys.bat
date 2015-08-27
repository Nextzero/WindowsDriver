echo off
REM	-------------------------------------------------------------------------
REM	Write by haozhe 2008-11-24
REM	
REM	_OBJNAME_		===>	Current Work driver letter
REM	_WORKDRV_		===>	Current Work driver letter
REM	_OBJPATH_		===>	Current Output file path
REM	_MAKEENV_		===>	Current DDK compile environment W2K, WXP, WNET, WLH
REM	-------------------------------------------------------------------------

	set _OBJNAME_=HelloDDK.sys
	set _WORKDRV_=%cd:~0,2%
	set _OBJPATH_=..\..\..\bin\debug\plugin\devicemon
	set _MAKEENV_=WXP
	set _WDKMODE_=1
	if "%1"=="checked"	set _OBJPATH_=..\..\..\bin\debug\plugin\devicemon
	if "%1"=="free"		set _OBJPATH_=..\..\..\bin\release\plugin\devicemon
	
REM	-------------------------------------------------------------------------

	if "%1"==""				goto ERROR
	if "%_WDKMODE_%"=="1"	goto WDKMODE

	if "%_MAKEENV_%"=="W2K" if "%1"=="checked"		set	_DDKPATH_=d:\MSDDK\Win2000
	if "%_MAKEENV_%"=="W2K" if "%1"=="checked"		set	_WKSTEP1_=CALL %_DDKPATH_%\bin\setenv.bat %_DDKPATH_% %1
	if "%_MAKEENV_%"=="W2K" if "%1"=="checked"		set	_WKSTEP2_=%_DDKPATH_%\bin\build -cewf -386 -M 1
	if "%_MAKEENV_%"=="W2K" if "%1"=="checked"		set	_WKSTEP3_=copy objchk\i386\%_OBJNAME_% %_OBJPATH_%
	if "%_MAKEENV_%"=="W2K" if "%1"=="checked"		goto MAKEWORK

	if "%_MAKEENV_%"=="W2K" if "%1"=="free"			set	_DDKPATH_=d:\MSDDK\Win2000
	if "%_MAKEENV_%"=="W2K" if "%1"=="free"			set	_WKSTEP1_=CALL %_DDKPATH_%\bin\setenv.bat %_DDKPATH_% %1
	if "%_MAKEENV_%"=="W2K" if "%1"=="free"			set	_WKSTEP2_=%_DDKPATH_%\bin\build -cewf -386 -M 1
	if "%_MAKEENV_%"=="W2K" if "%1"=="free"			set	_WKSTEP3_=copy objfre\i386\%_OBJNAME_% %_OBJPATH_%
	if "%_MAKEENV_%"=="W2K" if "%1"=="free"			goto MAKEWORK

	if "%_MAKEENV_%"=="WXP" if "%1"=="checked"		set	_DDKPATH_=d:\MSDDK\WinXP
	if "%_MAKEENV_%"=="WXP" if "%1"=="checked"		set	_WKSTEP1_=CALL %_DDKPATH_%\bin\setenv.bat %_DDKPATH_% %1
	if "%_MAKEENV_%"=="WXP" if "%1"=="checked"		set	_WKSTEP2_=%_DDKPATH_%\bin\x86\build -cewf -386 -M 1
	if "%_MAKEENV_%"=="WXP" if "%1"=="checked"		set	_WKSTEP3_=copy objchk\i386\%_OBJNAME_% %_OBJPATH_%
	if "%_MAKEENV_%"=="WXP" if "%1"=="checked"		goto MAKEWORK

	if "%_MAKEENV_%"=="WXP" if "%1"=="free"			set	_DDKPATH_=d:\MSDDK\WinXP
	if "%_MAKEENV_%"=="WXP" if "%1"=="free"			set	_WKSTEP1_=CALL %_DDKPATH_%\bin\setenv.bat %_DDKPATH_% %1
	if "%_MAKEENV_%"=="WXP" if "%1"=="free"			set	_WKSTEP2_=%_DDKPATH_%\bin\x86\build -cewf -386 -M 1
	if "%_MAKEENV_%"=="WXP" if "%1"=="free"			set	_WKSTEP3_=copy objfre\i386\%_OBJNAME_% %_OBJPATH_%
	if "%_MAKEENV_%"=="WXP" if "%1"=="free"			goto MAKEWORK
	
	goto ERROR

:WDKMODE
	if "%1"=="checked"	set	_MODE_=chk
	if "%1"=="free"		set	_MODE_=fre
	
	set NO_SAFESEH=1
	set	_DDKPATH_=d:\MSDDK\WDK
	set	_WKSTEP1_=call %_DDKPATH_%\bin\setenv.bat %_DDKPATH_% %_MODE_% %_MAKEENV_%
	set	_WKSTEP2_=%_DDKPATH_%\bin\x86\build -cewf -386 -M 1
	set	_WKSTEP3_=copy obj%_MODE_%_%_MAKEENV_%_x86\i386\%_OBJNAME_%  %_OBJPATH_%
	goto MAKEWORK

:MAKEWORK
	%_WKSTEP1_%
	%_WORKDRV_%
	%_WKSTEP2_%
	%_WKSTEP3_%
	del %_DDKPATH_%\*.dat
	goto EXIT

:ERROR
	echo Please use:  MakeSys [checked/free]

:EXIT
