# PostToAxis
Fusion 360 Addin to send data directly to LinuxCNC


To use it:
Copy sourcecode to %APPDATA%\Autodesk\Autodesk Fusion 360\API\AddIns
Build the code

Copy Server/server2.py to your linux computer and run it with:
python server2.py

An icon is created, press it, select the setup you want to send.
Select the operation you want to send or do not select anything to send the whole setup.

Hopefully it will load into linuxcnc and be ready to run!


Note:
This is very alpha, some functionality like the md5 check on server side is not yet implemented fully!
Worksheet sending is not implemented yet either.

Hope it works and please report bugs! =)


Added PostToAxis.dll for testing if it works like that.

Folder structure in addin should be:
AddIns\PostToAxis
AddIns\PostToAxis.dll
AddIns\resources
AddIns\resources\*

Hope that helps in testing it out