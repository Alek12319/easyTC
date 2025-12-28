cd G:\my_learning\easyTC\build
Remove-Item * -Recurse
cmake ..
cmake --build . --config Release --target install
cd G:\my_learning\easyTC
G:/my_learning/easyTC/install/easyExample/easyExample.exe