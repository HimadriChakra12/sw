# sw

simple terminal timer/alarm/counter

[![asciicast](https://asciinema.org/a/ct0LcBgRGFwkSWWJnGRXEXaw9.svg)](https://asciinema.org/a/ct0LcBgRGFwkSWWJnGRXEXaw9)

## dependency

- dbus
- C compiler

## compile

```sh
cc -o nob nob.c
./nob
```

## supported platform

- Linux (Alpine Linux)
- NetBSD

## usage

```sh
./sw # count up
./sw 1h20s # count down 1 hour 20 second
./sw 15m # count down 15 minute
./sw 5m Tea Time # count down 5 minute with message "Tea Time"
```
