# embedded_docker

A Docker File to be used in different kind of embedded projects.

This is kind of a base project to work on different kind of technologies.

main script is "bake" and the below is its help menu

```
This script is used for setting up the environment.
Options are are:
  --verbose               : Enable verbose output
  --module <module_name>  : Specify a module to include (can be used multiple times)
                          : Available modules are:
                              - helper
                              - esp8266
                              - esp32cam
                              - cc2640r2f
                              - stm32l4a6_freertos
                              - stm32f103_libcanard
                              - bbb_yocto
                              - atmega328p
                              - libcsp_works
                              - renode
```
