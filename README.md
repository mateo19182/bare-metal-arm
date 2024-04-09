openocd
make
gdb-multiarch -ex "target extended-remote localhost:3333" main.elf

temporizador TPM: Timer PWM Module

Multipurpose Clock Generator (MCG), pag 401 en adelante de https://campusvirtual.udc.gal/pluginfile.php/1035970/mod_resource/content/1/MCUXpresso%20SDK%20API%20Reference%20Manual_MKL46Z4.pdf

cronómetro que permita fixar un número de minutos e segundos de partida (mm:ss) cos botóns, e lanzar a conta atrás, empregando o LCD para mostrar en todo momento a conta. O cronómetro ten que incorporar a posibilidade de pausar e reanudar a conta.