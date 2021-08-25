# C Threads simple example : concurrent reading of two serial ports

Thread programming can be useful in small and simple programs.
In my application I needed to record information coming from two devices, a GPS and a depth sensor.
Reading sequentially two serial ports in a single thread could be tricky. With a blocking behavior, you will miss some sentences form one port while waiting for the other. With a non blocking configuration, you will get some incomplete sentences. The faster the communication, the worst it will be.

Programming with threads will solve the problem. Here are the important parts of the code :

### Functions

```C
void *read_and_print(void* nport){}
```

Infinite loop which wait for the given port to send data, and read it line after line, with a blocking behavior. Each line is printed onscreen and in the output file `serial.log`. Each sentence is recorded to this file with an Unix time tag and an hour tag (hhmmss), comma separated.

### Main

```C
pthread_create(&t1,NULL,read_and_print,(void *)&COM1);
pthread_create(&t2,NULL,read_and_print,(void *)&COM2);
```

Create two threads which run the function `read_and_print` for each port.

```C
pthread_join(t1,NULL);
```

suspend `main` while first thread is running (it could have been second one, doesn't matter). This line prevent the program to terminate after threads creation.

### COM ports initialization

As this program is used to read data transfer using [NMEA 0183 protocol](https://en.wikipedia.org/wiki/NMEA_0183), the c_cflag member of terminal control structure is configured with 4800 baud rate (B4800), 8 data bits (CS8), no parity, 1 stop bit and no handshake (default).
