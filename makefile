all:
	gcc -Wall -pthread -g SystemManager.c structs.h -o system_manager
	gcc -Wall -pthread UserConsole.c structs.h -o console
	gcc -Wall -pthread Sensor.c structs.h -o sensor

console:
	gcc -Wall -pthread UserConsole.c structs.h -o console

system_manager:
	gcc -Wall -pthread SystemManager.c structs.h -o system_manager

sensor:
	gcc -Wall -pthread Sensor.c structs.h -o sensor

rmconsole:
	rm console

rmsystem_manager:
	rm system_manager

rm_sensor:
	rm sensor

clean:
	rm system_manager
	rm console
	rm sensor
