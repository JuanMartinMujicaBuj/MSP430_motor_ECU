% %% LEER UN MSJ QUE TERMINA CON \n
% clc; clear; close all
% instrreset
% %oldPorts = instrfind; delete(oldPorts);
% s     = serial('COM4','BaudRate',9600);
% set(s,'Terminator',10); %10: line feed, 13: carriage return
% fopen(s)
% while(1)
%     fgets(s) %print lo que recibo
% end
%-------------------------------------------------------------------------%

% %% LEER Y ESCRIBIR EN PUERTO SERIE: +-60 CADA 4 SEG
% %Lee un msj que termina con \n, envía un msj tipo '+60', '-60'
% %No estoy seguro si tambien envia un '\n' al final. Anyways, el msp toma
% %solo los primeros 3 chars. Si hay un '\n', no le da bola.
% clc; clear; close all
% oldPorts = instrfind; delete(oldPorts);
% s     = serial('COM4','BaudRate',9600);
% set(s,'Terminator',10); %10: line feed (\n), 13: carriage return
% fopen(s)
% 
% i=1;
% while(1)
%     fgetl(s) %print lo que recibo: lectura joystick
%     i=i+1;
%     if mod(i,2)==0
%         fprintf(s,'+60'); %escribir una rta
%     else
%         fprintf(s,'-60');
%     end
%     fgetl(s) %print lo que vuelvo a recibir: echo del angulo que le mande
% end
%-------------------------------------------------------------------------%

% %% LEER Y ESCRIBIR EN PUERTO SERIE CADA 50MS
% %Lee un msj que termina con \n: el valor del joystick (de '-10\n' a '+10\n').
% %Suma ese valor a la variable angulo (variable dentro de matlab).
% %Envía el valor de angulo en un msj tipo '+37', '-06'.
% clc; clear; close all
% oldPorts = instrfind; delete(oldPorts);
% s     = serial('COM4','BaudRate',9600);
% set(s,'Terminator',10); %10: line feed (\n), 13: carriage return
% 
% joystickVal=0;
% angulo=0;
% signo=1;
% 
% fopen(s)
% while(1)
%     aux1=fgetl(s);
%     joystickVal=str2num(aux1); %display lo que recibo: lectura joystick
%     angulo=angulo+joystickVal;
%     if angulo<-90
%         angulo=-90;
%     elseif angulo>90
%         angulo=90;
%     end
%     fprintf(s,'%+0.2d',angulo); %send angulo en formato: '+02', '-77', etc.
%     aux2=fgetl(s); %display lo que vuelvo a recibir: echo del angulo que le mande
%     disp([aux1 ' ' aux2])
% end
%-------------------------------------------------------------------------%

% %% LEER PUERTO SERIE CADA 50MS
% %Lee un msj que contiene 3 bytes con info de: joystickVal, rpm, angulo
% %y termina con un '\n'.
% %Imprime esos valores.
% clc; clear; close all
% oldPorts = instrfind; delete(oldPorts);
% s     = serial('COM4','BaudRate',9600);
% set(s,'Terminator',10); %10: line feed (\n), 13: carriage return
% 
% %joystick=0;
% %rpm=0;
% %angulo=0;
% 
% fopen(s)
% while(1)
%     aux=fgetl(s);
%     %joystick=str2num(aux(1:3));
%     %rpm=str2num(aux(4:6));
%     %angulo=str2num(aux(7:9));
%     disp([aux(1:3) ' ' aux(4:6) ' ' aux(7:9)])
% end
%-------------------------------------------------------------------------%

%% LEER PUERTO SERIE CADA 50MS
%Lee un msj que contiene 6 bytes con info de: joystickVal, rpmMedidas, y
%termina con un '\n'.
%Imprime esos valores.
%Escribe un msj de 10 bytes con los valores de phi1, phi2, modoRemoto y joystickVal.
clc; clear; close all
oldPorts = instrfind; delete(oldPorts);
s     = serial('COM4','BaudRate',9600);
set(s,'Terminator',10); %10: line feed (\n), 13: carriage return

phi1=120;
phi2=130;
modoRemoto='N';
joystickVal=1;
i=0;
disp('joystick rpm phi1 phi2 modoRemoto')
fopen(s);
while(1)
    i=i+1;
    aux=fgetl(s);
    disp([aux(1:3) ' ' aux(4:6) ' ' modoRemoto]);
    fprintf(s,'%0.3d',phi1);
    fprintf(s,'%0.3d',phi2);
    fprintf(s,'%c',modoRemoto);
    fprintf(s,'%+0.2d',joystickVal);
    if i==500
        phi2=200;
    elseif i==1000
        phi2=300;
        modoRemoto='Y';
    elseif i==1100
        joystickVal=-5;
    elseif i==1120
        joystickVal=3;
    elseif i==1200
        joystickVal=-1;
    elseif i==1300
        joystickVal=0;
    elseif i==1500
        phi1=295;
        modoRemoto='N';
    elseif i==2000
        phi1=5;
    elseif i==2500
        phi1=120;
        phi2=130;
        i=0;
    end
    pause(0.1);
end
%-------------------------------------------------------------------------%


%-------------------------------------------------------------------------%
%TO DO: una gui para escribir un phi1, phi2, modoRemoto, joystickVal en
%tiempo real y con input del usuario, en vez de dejarlo programado en el
%matlab como hice arriba