/*
(1) in the file platform.txt change the following line:
        build.extra_flags=
to:
        build.extra_flags=-I "{build.path}" -include options.h

(2) in the Arduino\hardware\arduino\avr\cores\arduino directory
create an empty file called options.h

now in any sketch directory you can create a file called options.h
that contains the things you want to change locally to that sketch.
in my case this is:
#define SERIAL_TX_BUFFER_SIZE 64
#define SERIAL_RX_BUFFER_SIZE 1024
*/


#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_RX_BUFFER_SIZE 128

